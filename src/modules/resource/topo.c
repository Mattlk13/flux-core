/************************************************************\
 * Copyright 2020 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

/* topo.c - load and verify the local rank's hwloc topology
 *
 * If resources are known at module load time, verify the topology against
 * this rank's portion of the resource object (unless noverify is set).
 *
 * Reduce r_local + xml from each rank, leaving the result in
 * topo->reduce->rl and topo->reduce->xml on rank 0.  If resource are not
 * known, then this R is set in inventory.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <jansson.h>
#include <flux/core.h>
#include <hwloc.h>

#include "src/common/libidset/idset.h"
#include "src/common/libutil/errno_safe.h"
#include "src/common/librlist/rhwloc.h"
#include "src/common/librlist/rlist.h"

#include "resource.h"
#include "inventory.h"
#include "reslog.h"
#include "drain.h"
#include "rutil.h"
#include "topo.h"

struct reduction {
    int count;          // number of ranks represented
    int descendants;    // number of TBON descendants
    struct rlist *rl;   // resources: self + descendants
    json_t *xml;        // xml object: self + descendants
};                      //   keys are single ranks, no dedup possible

struct topo {
    struct resource_ctx *ctx;
    flux_msg_handler_t **handlers;
    char *xml;
    struct rlist *r_local;

    struct reduction reduce;
};

static int drain_self (struct topo *topo, const char *reason)
{
    flux_log (topo->ctx->h, LOG_ERR, "draining: %s", reason);

    if (topo->ctx->rank == 0) {
        if (drain_rank (topo->ctx->drain, topo->ctx->rank, reason) < 0)
            return -1;
    }
    else {
        char rankstr[16];
        flux_future_t *f;

        snprintf (rankstr, sizeof (rankstr), "%ju", (uintmax_t)topo->ctx->rank);
        if (!(f = flux_rpc_pack (topo->ctx->h,
                                 "resource.drain",
                                 0,
                                 0,
                                 "{s:s s:s}",
                                 "idset",
                                 rankstr,
                                 "reason",
                                 reason)))
            return -1;
        if (flux_rpc_get (f, NULL) < 0) {
            flux_future_destroy (f);
            return -1;
        }
        flux_future_destroy (f);
    }
    return 0;
}

static int topo_verify (struct topo *topo, json_t *R, bool nodrain)
{
    json_error_t e;
    struct rlist *rl;
    rlist_error_t error;
    int rc;

    if (!(rl = rlist_from_json (R, &e))) {
        flux_log (topo->ctx->h, LOG_ERR, "R: %s", e.text);
        errno = EINVAL;
        return -1;
    }
    rc = rlist_verify (&error, rl, topo->r_local);
    if (rc < 0 && !nodrain) {
        if (drain_self (topo, error.text) < 0) {
            rlist_destroy (rl);
            return -1;
        }
    }
    else if (rc != 0)
        flux_log (topo->ctx->h, LOG_ERR, "verify: %s", error.text);
    rlist_destroy (rl);
    return 0;
}

/* Call this on any rank when there are no more descendants reporting.
 * On rank 0, this finalizes the reduction.
 * On other ranks, the reduction is sent upstream.
 */
static int topo_reduce_finalize (struct topo *topo)
{
    json_t *resobj = NULL;

    if (!(resobj = rlist_to_R (topo->reduce.rl))) {
        flux_log (topo->ctx->h, LOG_ERR, "error converting reduced rlist");
        errno = EINVAL;
        return -1;
    }
    if (topo->ctx->rank == 0) {
        if (!inventory_get (topo->ctx->inventory)) {
            if (inventory_put (topo->ctx->inventory,
                               resobj,
                               "dynamic-discovery") < 0) {
                flux_log_error (topo->ctx->h,
                                "error setting reduced resource object");
                goto error;
            }
        }
        if (!inventory_get_xml (topo->ctx->inventory)) {
            if (inventory_put_xml (topo->ctx->inventory, topo->reduce.xml) < 0){
                flux_log_error (topo->ctx->h,
                                "error setting reduced XML object");
                goto error;
            }
        }
    }
    else {
        flux_future_t *f;

        if (!(f = flux_rpc_pack (topo->ctx->h,
                                 "resource.topo-reduce",
                                 FLUX_NODEID_UPSTREAM,
                                 FLUX_RPC_NORESPONSE,
                                 "{s:i s:O s:O}",
                                 "count",
                                 topo->reduce.count,
                                 "resource",
                                 resobj,
                                 "xml",
                                 topo->reduce.xml))) {
            flux_log_error (topo->ctx->h,
                            "resource.topo-reduce: error sending request");
            goto error;
        }
        flux_future_destroy (f);
    }
    json_decref (resobj);
    return 0;
error:
    ERRNO_SAFE_WRAP (json_decref, resobj);
    return -1;
}

/* Accept reduction input from downstream ranks.
 * Ignore it if reduction has already completed (node may have restarted).
 */
static void topo_reduce_cb (flux_t *h,
                            flux_msg_handler_t *mh,
                            const flux_msg_t *msg,
                            void *arg)
{
    struct topo *topo = arg;
    json_t *resobj;
    struct rlist *rl = NULL;
    json_t *xml;
    int count;

    if (flux_request_unpack (msg,
                             NULL,
                             "{s:i s:o s:o}",
                             "count",
                             &count,
                             "resource",
                             &resobj,
                             "xml",
                             &xml) < 0)
        goto error;
    if (topo->reduce.count + count <= topo->reduce.descendants + 1) {
        json_error_t e;

        if (!(rl = rlist_from_json (resobj, &e))) {
            flux_log (h, LOG_ERR, "error reducing resource object: %s", e.text);
            errno = EINVAL;
            goto error;
        }
        if (rlist_append (topo->reduce.rl, rl) < 0) {
            errno = ENOMEM;
            goto error;
        }
        if (json_object_update (topo->reduce.xml, xml) < 0)
            goto nomem;

        topo->reduce.count += count;

        if (topo->reduce.count == topo->reduce.descendants + 1) {
            if (topo_reduce_finalize (topo) < 0)
                goto error;
        }
    }
    rlist_destroy (rl);
    return;
nomem:
    errno = ENOMEM;
error:
    flux_log_error (h, "resource.topo-reduce");
    flux_reactor_stop_error (flux_get_reactor (h));
    rlist_destroy (rl);
}

/* Set up for reduction of distributed topo->r_local to inventory.
 * Ranks with descendants wait for all of them to report in, then roll
 * up their own and their descendants' contributions into one object and
 * report that.  N.B. This is not a "timed batch" style reduction since the
 * final result cannot be obtained without the participation of all ranks.
 */
static int topo_reduce (struct topo *topo)
{
    const char *val;
    char rankstr[32];

    if (!(val = flux_attr_get (topo->ctx->h, "tbon.descendants")))
        return -1;
    errno = 0;
    topo->reduce.descendants = strtoul (val, NULL, 10);
    if (errno > 0)
        return -1;

    topo->reduce.count = 1;
    snprintf (rankstr, sizeof (rankstr), "%d", (int)topo->ctx->rank);
    if (!(topo->reduce.xml = json_pack ("{s:s}", rankstr, topo->xml)))
        goto nomem;
    if (!(topo->reduce.rl = rlist_copy_empty (topo->r_local)))
        goto nomem;

    if (topo->reduce.descendants == 0) {
        if (topo_reduce_finalize (topo) < 0)
            return -1;
    }
    return 0;
nomem:
    errno = ENOMEM;
    return -1;
}

static void topo_get_cb (flux_t *h,
                         flux_msg_handler_t *mh,
                         const flux_msg_t *msg,
                         void *arg)
{
    struct topo *topo = arg;

    if (flux_request_decode (msg, NULL, NULL) < 0)
        goto error;
    if (flux_respond (h, msg, topo->xml) < 0)
        flux_log_error (h, "error responding to topo-get request");
    return;
error:
    if (flux_respond_error (h, msg, errno, NULL) < 0)
        flux_log_error (h, "error responding to topo-get request");
}

static const struct flux_msg_handler_spec htab[] = {
    { FLUX_MSGTYPE_REQUEST, "resource.topo-reduce",  topo_reduce_cb, 0 },
    { FLUX_MSGTYPE_REQUEST, "resource.topo-get", topo_get_cb, 0 },
    FLUX_MSGHANDLER_TABLE_END,
};


void topo_destroy (struct topo *topo)
{
    if (topo) {
        int saved_errno = errno;
        flux_msg_handler_delvec (topo->handlers);
        free (topo->xml);
        rlist_destroy (topo->reduce.rl);
        json_decref (topo->reduce.xml);
        rlist_destroy (topo->r_local);
        free (topo);
        errno = saved_errno;
    }
}

struct topo *topo_create (struct resource_ctx *ctx, bool no_verify)
{
    struct topo *topo;
    json_t *R;

    if (!(topo = calloc (1, sizeof (*topo))))
        return NULL;
    topo->ctx = ctx;
    if (!(topo->xml = rhwloc_local_topology_xml ())) {
        flux_log_error (ctx->h, "error loading hwloc topology");
        goto error;
    }
    if (!(topo->r_local = rlist_from_hwloc (ctx->rank, topo->xml))) {
        flux_log_error (ctx->h, "error creating local resource object");
        goto error;
    }
    /* If global resource object is known now, use it to verify topo.
     */
    if ((R = inventory_get (ctx->inventory))) {
        const char *method = inventory_get_method (ctx->inventory);
        bool nodrain = false;

        if (method && !strcmp (method, "job-info"))
            nodrain = true;
        if (!no_verify && topo_verify (topo, R, nodrain) < 0)
            goto error;
    }
    /* Reduce topo to rank 0 unconditionally in case it is needed.
     */
    if (topo_reduce (topo) < 0) {
        flux_log_error (ctx->h, "error setting up topo reduction");
        goto error;
    }
    if (flux_msg_handler_addvec (ctx->h, htab, topo, &topo->handlers) < 0)
        goto error;
    return topo;
error:
    topo_destroy (topo);
    return NULL;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
