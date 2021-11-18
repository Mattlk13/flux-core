/************************************************************\
 * Copyright 2014 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdarg.h>
#include <czmq.h>
#include <zmq.h>
#include <flux/core.h>
#include <inttypes.h>
#include <jansson.h>
#include <uuid.h>

#include "src/common/libzmqutil/msg_zsock.h"
#include "src/common/libzmqutil/reactor.h"
#include "src/common/libzmqutil/zap.h"
#include "src/common/libzmqutil/monitor.h"
#include "src/common/libczmqcontainers/czmq_containers.h"
#include "src/common/libutil/log.h"
#include "src/common/libutil/kary.h"
#include "src/common/libutil/cleanup.h"
#include "src/common/libutil/fsd.h"
#include "src/common/libutil/errno_safe.h"
#include "src/common/libutil/monotime.h"
#include "src/common/librouter/rpc_track.h"

#include "overlay.h"
#include "attr.h"

#ifndef UUID_STR_LEN
#define UUID_STR_LEN 37     // defined in later libuuid headers
#endif

#define FLUX_ZAP_DOMAIN "flux"

#define DEFAULT_FANOUT 2

/* Overlay keepalives set PROTO aux1 to the keepalive type.
 * The meaning of PROTO aux2 depends on the type.
 */
enum keepalive_type {
    KEEPALIVE_HEARTBEAT = 0, // child sends when connection is idle
    KEEPALIVE_STATUS = 1,    // child tells parent of subtree status change
    KEEPALIVE_DISCONNECT = 2,// parent tells child to immediately disconnect
};

/* Numerical values for "subtree health" so we can send them in keepalive
 * messages.  Textual values below will be used for communication with front
 * end diagnostic tool.
 */
enum subtree_status {
    SUBTREE_STATUS_UNKNOWN = 0,
    SUBTREE_STATUS_FULL = 1,
    SUBTREE_STATUS_PARTIAL = 2,
    SUBTREE_STATUS_DEGRADED = 3,
    SUBTREE_STATUS_LOST = 4,
    SUBTREE_STATUS_OFFLINE = 5,
    SUBTREE_STATUS_MAXIMUM = 5,
};

/* Names array is indexed with subtree_status enum.
 * Convert with subtree_status_str()
 */
static const char *subtree_status_names[] = {
    "unknown",
    "full",
    "partial",
    "degraded",
    "lost",
    "offline",
    NULL,
};

struct child {
    double lastseen;
    uint32_t rank;
    char uuid[UUID_STR_LEN];
    enum subtree_status status;
    struct timespec status_timestamp;
    bool idle;
    struct rpc_track *tracker;
};

struct parent {
    zsock_t *zsock;         // NULL on rank 0
    char *uri;
    flux_watcher_t *w;
    int lastsent;
    char *pubkey;
    uint32_t rank;
    char uuid[UUID_STR_LEN];
    bool hello_error;
    bool hello_responded;
    bool offline;           // set upon receipt of KEEPALIVE_DISCONNECT
    struct rpc_track *tracker;
    struct zmqutil_monitor *monitor;
};

/* Wake up periodically (between 'sync_min' and 'sync_max' seconds) and:
 * 1) send keepalive to parent if nothing was sent in 'idle_min' seconds
 * 2) find children that have not been heard from in 'idle_max' seconds
 */
static const double sync_min = 1.0;
static const double sync_max = 5.0;

static const double idle_min = 5.0;
static const double idle_max = 30.0;

struct overlay {
    zcert_t *cert;
    struct zmqutil_zap *zap;
    int enable_ipv6;

    flux_t *h;
    attr_t *attrs;
    flux_reactor_t *reactor;
    flux_msg_handler_t **handlers;
    flux_future_t *f_sync;

    uint32_t size;
    uint32_t rank;
    int fanout;
    char uuid[UUID_STR_LEN];
    int version;
    int zmqdebug;

    struct parent parent;

    zsock_t *bind_zsock;        // NULL if no downstream peers
    char *bind_uri;
    flux_watcher_t *bind_w;
    struct child *children;
    int child_count;
    zhashx_t *child_hash;
    enum subtree_status status;
    struct timespec status_timestamp;
    struct zmqutil_monitor *bind_monitor;

    overlay_monitor_f child_monitor_cb;
    void *child_monitor_arg;

    overlay_loss_f loss_cb;
    void *loss_arg;

    overlay_recv_f recv_cb;
    void *recv_arg;

    struct flux_msglist *health_requests;
};

static void overlay_mcast_child (struct overlay *ov, const flux_msg_t *msg);
static int overlay_sendmsg_child (struct overlay *ov, const flux_msg_t *msg);
static int overlay_sendmsg_parent (struct overlay *ov, const flux_msg_t *msg);
static void hello_response_handler (struct overlay *ov, const flux_msg_t *msg);
static void hello_request_handler (struct overlay *ov, const flux_msg_t *msg);
static int overlay_keepalive_parent (struct overlay *ov,
                                     enum keepalive_type ktype,
                                     int arg);
static void overlay_health_respond_all (struct overlay *ov);
static json_t *get_subtree_topo (struct overlay *ov, int rank);

/* Convenience iterator for ov->children
 */
#define foreach_overlay_child(ov, child) \
    for ((child) = &(ov)->children[0]; \
            (child) - &(ov)->children[0] < (ov)->child_count; \
            (child)++)

static const char *subtree_status_str (enum subtree_status status)
{
    if (status > SUBTREE_STATUS_MAXIMUM)
        return "unknown";
    return subtree_status_names[status];
}

static bool subtree_is_online (enum subtree_status status)
{
    switch (status) {
        case SUBTREE_STATUS_FULL:
        case SUBTREE_STATUS_PARTIAL:
        case SUBTREE_STATUS_DEGRADED:
            return true;
        default:
            return false;
    }
}

/* Call this function after a child->status changes.
 * It calculates a new subtree status based on the state of children,
 * then if the status has changed, the parent is informed with a
 * keepalive message and any waiting health requests are processed.
 */
static void subtree_status_update (struct overlay *ov)
{
    enum subtree_status status = SUBTREE_STATUS_FULL;
    struct child *child;

    foreach_overlay_child (ov, child) {
        switch (child->status) {
            case SUBTREE_STATUS_FULL:
                break;
            case SUBTREE_STATUS_PARTIAL:
            case SUBTREE_STATUS_OFFLINE:
                if (status == SUBTREE_STATUS_FULL)
                    status = SUBTREE_STATUS_PARTIAL;
                break;
            case SUBTREE_STATUS_DEGRADED:
            case SUBTREE_STATUS_LOST:
                if (status != SUBTREE_STATUS_DEGRADED)
                    status = SUBTREE_STATUS_DEGRADED;
                break;
            case SUBTREE_STATUS_UNKNOWN:
                break;
        }
    }
    if (ov->status != status) {
        ov->status = status;
        monotime (&ov->status_timestamp);
        overlay_keepalive_parent (ov, KEEPALIVE_STATUS, ov->status);
        overlay_health_respond_all (ov);
    }
}

static void overlay_monitor_notify (struct overlay *ov)
{
    if (ov->child_monitor_cb)
        ov->child_monitor_cb (ov, ov->child_monitor_arg);
}

static int child_count (uint32_t rank, uint32_t size, int k)
{
    int count;
    for (count = 0; kary_childof (k, size, rank, count) != KARY_NONE; count++)
        ;
    return count;
}

static void overlay_loss_notify (struct overlay *ov,
                                 struct child *child,
                                 int status)
{
    if (ov->loss_cb) {
        json_t *topo;
        if (!(topo = get_subtree_topo (ov, child->rank))) {
            flux_log_error (ov->h, "overlay_loss_notify failed");
            return;
        }
        ov->loss_cb (ov,
                     child->rank,
                     subtree_status_str (status),
                     topo,
                     ov->loss_arg);
        json_decref (topo);
    }
}

int overlay_set_geometry (struct overlay *ov, uint32_t size, uint32_t rank)
{
    ov->size = size;
    ov->rank = rank;
    ov->child_count = child_count (rank, size, ov->fanout);
    if (ov->child_count > 0) {
        int i;

        if (!(ov->children = calloc (ov->child_count, sizeof (struct child))))
            return -1;
        if (!(ov->child_hash = zhashx_new ()))
            return -1;
        zhashx_set_key_duplicator (ov->child_hash, NULL);
        zhashx_set_key_destructor (ov->child_hash, NULL);
        for (i = 0; i < ov->child_count; i++) {
            struct child *child = &ov->children[i];
            child->rank = kary_childof (ov->fanout, size, rank, i);
            child->status = SUBTREE_STATUS_OFFLINE;
            monotime (&child->status_timestamp);
            child->tracker = rpc_track_create (MSG_HASH_TYPE_UUID_MATCHTAG);
            if (!child->tracker)
                return -1;
        }
        ov->status = SUBTREE_STATUS_PARTIAL;
    }
    else
        ov->status = SUBTREE_STATUS_FULL;
    monotime (&ov->status_timestamp);
    if (rank > 0) {
        ov->parent.rank = kary_parentof (ov->fanout, rank);
        ov->parent.tracker = rpc_track_create (MSG_HASH_TYPE_UUID_MATCHTAG);
    }

    return 0;
}

int overlay_get_fanout (struct overlay *ov)
{
    return ov->fanout;
}

uint32_t overlay_get_rank (struct overlay *ov)
{
    return ov->rank;
}

void overlay_set_rank (struct overlay *ov, uint32_t rank)
{
    ov->rank = rank;
}

uint32_t overlay_get_size (struct overlay *ov)
{
    return ov->size;
}

const char *overlay_get_uuid (struct overlay *ov)
{
    return ov->uuid;
}

bool overlay_parent_error (struct overlay *ov)
{
    return ((ov->parent.hello_responded && ov->parent.hello_error)
            || ov->parent.offline);
}

bool overlay_parent_success (struct overlay *ov)
{
    return (ov->parent.hello_responded
            && !ov->parent.hello_error);
}

void overlay_set_version (struct overlay *ov, int version)
{
    ov->version = version;
}

int overlay_get_child_peer_count (struct overlay *ov)
{
    struct child *child;
    int count = 0;

    foreach_overlay_child (ov, child) {
        if (subtree_is_online (child->status))
            count++;
    }
    return count;
}

void overlay_set_ipv6 (struct overlay *ov, int enable)
{
    ov->enable_ipv6 = enable;
}

void overlay_log_idle_children (struct overlay *ov)
{
    struct child *child;
    double now = flux_reactor_now (ov->reactor);
    char fsd[64];
    double idle;

    if (idle_max > 0) {
        foreach_overlay_child (ov, child) {
            if (subtree_is_online (child->status) && child->lastseen > 0) {
                idle = now - child->lastseen;

                if (idle >= idle_max) {
                    (void)fsd_format_duration (fsd, sizeof (fsd), idle);
                    if (!child->idle) {
                        flux_log (ov->h,
                                  LOG_ERR,
                        "broker on %s (rank %lu) has been unresponsive for %s",
                                  flux_get_hostbyrank (ov->h, child->rank),
                                  (unsigned long)child->rank,
                                  fsd);
                        child->idle = true;
                    }
                }
                else {
                    if (child->idle) {
                        flux_log (ov->h,
                                  LOG_ERR,
                                  "broker on %s (rank %lu) is responsive now",
                                  flux_get_hostbyrank (ov->h, child->rank),
                                  (unsigned long)child->rank);
                        child->idle = false;
                    }
                }
            }
        }
    }
}

/* N.B. overlay_child_status_update() ensures child_lookup() only
 * succeeds for online peers.
 */
static struct child *child_lookup_online (struct overlay *ov, const char *id)
{
    return ov->child_hash ?  zhashx_lookup (ov->child_hash, id) : NULL;
}

/* Slow path to look up child regardless of its status.
 */
static struct child *child_lookup (struct overlay *ov, const char *id)
{
    struct child *child;

    foreach_overlay_child (ov, child) {
        if (streq (child->uuid, id))
            return child;
    }
    return NULL;
}

/* Given a rank, find a (direct) child peer.
 * Since child ranks are numerically contiguous, perform a range check
 * and index into the child array directly.
 * Returns NULL on lookup failure.
 */
static struct child *child_lookup_byrank (struct overlay *ov, uint32_t rank)
{
    uint32_t first;
    int i;

    if ((first = kary_childof (ov->fanout, ov->size, ov->rank, 0)) == KARY_NONE
        || (i = rank - first) < 0
        || i >= ov->child_count)
        return NULL;
    return &ov->children[i];
}

/* Look up child that provides route to 'rank' (NULL if none).
 */
static struct child *child_lookup_route (struct overlay *ov, uint32_t rank)
{
    uint32_t child_rank;

    child_rank = kary_child_route (ov->fanout, ov->size, ov->rank, rank);
    if (child_rank == KARY_NONE)
        return NULL;
    return child_lookup_byrank (ov, child_rank);
}

bool overlay_uuid_is_child (struct overlay *ov, const char *uuid)
{
    if (child_lookup_online (ov, uuid) != NULL)
        return true;
    return false;
}

bool overlay_uuid_is_parent (struct overlay *ov, const char *uuid)
{
    if (ov->rank > 0 && streq (uuid, ov->parent.uuid))
        return true;
    return false;
}

int overlay_set_parent_pubkey (struct overlay *ov, const char *pubkey)
{
    if (!(ov->parent.pubkey = strdup (pubkey)))
        return -1;
    return 0;
}

int overlay_set_parent_uri (struct overlay *ov, const char *uri)
{
    free (ov->parent.uri);
    if (!(ov->parent.uri = strdup (uri)))
        return -1;
    return 0;
}

const char *overlay_get_parent_uri (struct overlay *ov)
{
    return ov->parent.uri;
}

static int overlay_sendmsg_parent (struct overlay *ov, const flux_msg_t *msg)
{
    int rc = -1;

    if (!ov->parent.zsock || ov->parent.offline) {
        errno = EHOSTUNREACH;
        goto done;
    }
    rc = zmqutil_msg_send (ov->parent.zsock, msg);
    if (rc == 0)
        ov->parent.lastsent = flux_reactor_now (ov->reactor);
done:
    return rc;
}

static int overlay_keepalive_parent (struct overlay *ov,
                                     enum keepalive_type ktype,
                                     int arg)
{
    flux_msg_t *msg = NULL;

    if (ov->parent.zsock) {
        if (!(msg = flux_keepalive_encode (ktype, arg)))
            return -1;
        flux_msg_route_enable (msg);
        if (overlay_sendmsg_parent (ov, msg) < 0)
            goto error;
        flux_msg_destroy (msg);
    }
    return 0;
error:
    flux_msg_destroy (msg);
    return -1;
}

static int overlay_keepalive_child (struct overlay *ov,
                                    const char *uuid,
                                    enum keepalive_type ktype,
                                    int arg)
{
    flux_msg_t *msg;

    if (!(msg = flux_keepalive_encode (ktype, arg)))
        return -1;
    flux_msg_route_enable (msg);
    if (flux_msg_route_push (msg, uuid) < 0)
        goto error;
    if (overlay_sendmsg_child (ov, msg) < 0)
        goto error;
    flux_msg_destroy (msg);
    return 0;
error:
    flux_msg_destroy (msg);
    return -1;
}

int overlay_sendmsg (struct overlay *ov,
                     const flux_msg_t *msg,
                     overlay_where_t where)
{
    int type;
    uint8_t flags;
    flux_msg_t *cpy = NULL;
    const char *uuid;
    uint32_t nodeid;
    struct child *child = NULL;
    int rc;

    if (flux_msg_get_type (msg, &type) < 0
        || flux_msg_get_flags (msg, &flags) < 0)
        return -1;
    switch (type) {
        case FLUX_MSGTYPE_REQUEST:
            /* If message is being routed downstream to reach 'nodeid',
             * push the local uuid, then the next hop onto the messages's
             * route stack so that the ROUTER socket can pop off next hop to
             * select the peer, and our uuid remains as part of the source addr.
             */
            if (where == OVERLAY_ANY) {
                if (flux_msg_get_nodeid (msg, &nodeid) < 0)
                    goto error;
                if ((flags & FLUX_MSGFLAG_UPSTREAM) && nodeid == ov->rank)
                    where = OVERLAY_UPSTREAM;
                else {
                    if ((child = child_lookup_route (ov, nodeid))) {
                        if (!subtree_is_online (child->status)) {
                            errno = EHOSTUNREACH;
                            goto error;
                        }
                        if (!(cpy = flux_msg_copy (msg, true)))
                            goto error;
                        if (flux_msg_route_push (cpy, ov->uuid) < 0)
                            goto error;
                        if (flux_msg_route_push (cpy, child->uuid) < 0)
                            goto error;
                        msg = cpy;
                        where = OVERLAY_DOWNSTREAM;
                    }
                    else
                        where = OVERLAY_UPSTREAM;
                }
            }
            if (where == OVERLAY_UPSTREAM) {
                rc = overlay_sendmsg_parent (ov, msg);
                if (rc == 0)
                    rpc_track_update (ov->parent.tracker, msg);
            }
            else {
                rc = overlay_sendmsg_child (ov, msg);
                if (rc == 0) {
                    if (!child) {
                        if ((uuid = flux_msg_route_last (msg)))
                            child = child_lookup_online (ov, ov->uuid);
                    }
                    if (child)
                        rpc_track_update (child->tracker, msg);
                }
            }
            if (rc < 0)
                goto error;
            break;
        case FLUX_MSGTYPE_RESPONSE:
            /* Assume if next route matches parent, the message goes upstream;
             * otherwise downstream.  The send downstream will fail with
             * EHOSTUNREACH if uuid doesn't match an immediate peer.
             */
            if (where == OVERLAY_ANY) {
                if (ov->rank > 0
                    && (uuid = flux_msg_route_last (msg)) != NULL
                    && streq (uuid, ov->parent.uuid))
                    where = OVERLAY_UPSTREAM;
                else
                    where = OVERLAY_DOWNSTREAM;
            }
            if (where == OVERLAY_UPSTREAM)
                rc = overlay_sendmsg_parent (ov, msg);
            else
                rc = overlay_sendmsg_child (ov, msg);
            if (rc < 0)
                goto error;
            break;
        case FLUX_MSGTYPE_EVENT:
            if (where == OVERLAY_DOWNSTREAM || where == OVERLAY_ANY)
                overlay_mcast_child (ov, msg);
            else {
                /* N.B. add route delimiter if needed to pass unpublished
                 * event message upstream through router socket.
                 */
                if (!(flags & FLUX_MSGFLAG_ROUTE)) {
                    if (!(cpy = flux_msg_copy (msg, true)))
                        goto error;
                    flux_msg_route_enable (cpy);
                    msg = cpy;
                }
                if (overlay_sendmsg_parent (ov, msg) < 0)
                    goto error;
            }
            break;
        default:
            goto inval;
    }
    flux_msg_decref (cpy);
    return 0;
inval:
    errno = EINVAL;
error:
    flux_msg_decref (cpy);
    return -1;
}

static void sync_cb (flux_future_t *f, void *arg)
{
    struct overlay *ov = arg;
    double now = flux_reactor_now (ov->reactor);

    if (now - ov->parent.lastsent > idle_min)
        overlay_keepalive_parent (ov, KEEPALIVE_HEARTBEAT, 0);
    overlay_log_idle_children (ov);

    flux_future_reset (f);
}

const char *overlay_get_bind_uri (struct overlay *ov)
{
    return ov->bind_uri;
}

/* Log a failure to send tracker EHOSTUNREACH response.  Suppress logging
 * ENOSYS failures, which happen if sending module unloads before completing
 * all RPCs.
 */
static void log_tracker_error (flux_t *h, const flux_msg_t *msg, int errnum)
{
    if (errnum != ENOSYS) {
        const char *topic = "unknown";
        (void)flux_msg_get_topic (msg, &topic);
        flux_log_error (h,
                        "tracker: error sending %s EHOSTUNREACH response",
                        topic);
    }
}

/* Child endpoint disconnected, so any pending RPCs going that way
 * get EHOSTUNREACH responses so they can fail fast.
 */
static void fail_child_rpcs (const flux_msg_t *msg, void *arg)
{
    struct overlay *ov = arg;
    flux_msg_t *rep;

    if (!(rep = flux_response_derive (msg, EHOSTUNREACH))
        || flux_msg_route_delete_last (rep) < 0
        || flux_msg_route_delete_last (rep) < 0
        || flux_send (ov->h, rep, 0) < 0)
        log_tracker_error (ov->h, rep, errno);
    flux_msg_destroy (rep);
}

static void overlay_child_status_update (struct overlay *ov,
                                         struct child *child,
                                         int status)
{
    if (child->status != status) {
        if (subtree_is_online (child->status)
            && !subtree_is_online (status)) {
            zhashx_delete (ov->child_hash, child->uuid);
            rpc_track_purge (child->tracker, fail_child_rpcs, ov);
            overlay_loss_notify (ov, child, status);
        }
        else if (!subtree_is_online (child->status)
            && subtree_is_online (status)) {
            zhashx_insert (ov->child_hash, child->uuid, child);
        }

        child->status = status;
        monotime (&child->status_timestamp);

        subtree_status_update (ov);
        overlay_monitor_notify (ov);
        overlay_health_respond_all (ov);
    }
}

static int overlay_sendmsg_child (struct overlay *ov, const flux_msg_t *msg)
{
    int rc = -1;

    if (!ov->bind_zsock) {
        errno = EHOSTUNREACH;
        goto done;
    }
    rc = zmqutil_msg_send_ex (ov->bind_zsock, msg, true);
    /* Since ROUTER socket has ZMQ_ROUTER_MANDATORY set, EHOSTUNREACH on a
     * connected peer signifies a disconnect.  See zmq_setsockopt(3).
     */
    if (rc < 0 && errno == EHOSTUNREACH) {
        int saved_errno = errno;
        const char *uuid;
        struct child *child;

        if ((uuid = flux_msg_route_last (msg))
            && (child = child_lookup_online (ov, uuid))) {
            overlay_child_status_update (ov, child, SUBTREE_STATUS_LOST);
        }
        errno = saved_errno;
    }
done:
    return rc;
}

static int overlay_mcast_child_one (struct overlay *ov,
                                    const flux_msg_t *msg,
                                    struct child *child)
{
    flux_msg_t *cpy;
    int rc = -1;

    if (!(cpy = flux_msg_copy (msg, true)))
        return -1;
    flux_msg_route_enable (cpy);
    if (flux_msg_route_push (cpy, child->uuid) < 0)
        goto done;
    if (overlay_sendmsg_child (ov, cpy) < 0)
        goto done;
    rc = 0;
done:
    flux_msg_destroy (cpy);
    return rc;
}

static void overlay_mcast_child (struct overlay *ov, const flux_msg_t *msg)
{
    struct child *child;

    foreach_overlay_child (ov, child) {
        if (subtree_is_online (child->status)) {
            if (overlay_mcast_child_one (ov, msg, child) < 0) {
                if (errno != EHOSTUNREACH) {
                    flux_log_error (ov->h,
                                    "mcast error to child rank %lu",
                                    (unsigned long)child->rank);
                }
            }
        }
    }
}

static void logdrop (struct overlay *ov,
                     overlay_where_t where,
                     const flux_msg_t *msg,
                     const char *fmt, ...)
{
    char reason[128];
    va_list ap;
    const char *topic = NULL;
    int type = -1;
    const char *child_uuid = NULL;

    (void)flux_msg_get_type (msg, &type);
    (void)flux_msg_get_topic (msg, &topic);
    if (where == OVERLAY_DOWNSTREAM)
        child_uuid = flux_msg_route_last (msg);

    va_start (ap, fmt);
    (void)vsnprintf (reason, sizeof (reason), fmt, ap);
    va_end (ap);

    flux_log (ov->h,
              LOG_ERR,
              "DROP %s %s topic %s %s%s: %s",
              where == OVERLAY_UPSTREAM ? "upstream" : "downstream",
              type != -1 ? flux_msg_typestr (type) : "message",
              topic ? topic : "-",
              child_uuid ? "from " : "",
              child_uuid ? child_uuid : "",
              reason);
}

/* Handle a message received from TBON child (downstream).
 */
static void child_cb (flux_reactor_t *r, flux_watcher_t *w,
                      int revents, void *arg)
{
    struct overlay *ov = arg;
    flux_msg_t *msg;
    int type = -1;
    const char *topic = NULL;
    const char *uuid = NULL;
    struct child *child;

    if (!(msg = zmqutil_msg_recv (ov->bind_zsock)))
        return;
    if (flux_msg_get_type (msg, &type) < 0
        || !(uuid = flux_msg_route_last (msg))) {
        logdrop (ov, OVERLAY_DOWNSTREAM, msg, "malformed message");
        goto done;
    }
    if (!(child = child_lookup_online (ov, uuid))) {
        /* If child is not online but we know this uuid, message is from a
         * child that transitioned to offline/lost.  Don't log the drop if
         * LOST, since message was probably sent before child received
         * KEEPALIVE_DISCONNECT and that is not interesting.
         */
        if ((child = child_lookup (ov, uuid))) {
            if (child->status != SUBTREE_STATUS_LOST) {
                logdrop (ov, OVERLAY_DOWNSTREAM, msg,
                         "message from %s rank %lu",
                         subtree_status_str (child->status),
                         (unsigned long)child->rank);
            }
        }
        /* Hello new peer!
         * hello_request_handler() completes the handshake.
         */
        else if (type == FLUX_MSGTYPE_REQUEST
            && flux_msg_get_topic (msg, &topic) == 0
            && streq (topic, "overlay.hello")) {
            hello_request_handler (ov, msg);
        }
        /* New peer is trying to communicate without first saying hello.
         * This likely means that *this broker* restarted without getting a
         * message through to the child, and the child still thinks it is
         * communicating with the same broker (since 0MQ hides reconnects).
         * Send KEEPALIVE_DISCONNECT to force subtree panic.
         */
        else {
            logdrop (ov, OVERLAY_DOWNSTREAM, msg,
                     "didn't say hello, sending disconnect");
            if (overlay_keepalive_child (ov, uuid, KEEPALIVE_DISCONNECT, 0) < 0)
                flux_log_error (ov->h, "failed to send KEEPALIVE_DISCONNECT");
        }
        goto done;
    }
    assert (subtree_is_online (child->status));

    child->lastseen = flux_reactor_now (ov->reactor);
    switch (type) {
        case FLUX_MSGTYPE_KEEPALIVE: {
            int ktype, status;
            if (flux_keepalive_decode (msg, &ktype, &status) == 0
                && ktype == KEEPALIVE_STATUS)
                overlay_child_status_update (ov, child, status);
            goto done;
        }
        case FLUX_MSGTYPE_REQUEST:
            break;
        case FLUX_MSGTYPE_RESPONSE:
            /* Response message traveling upstream requires special handling:
             * ROUTER socket will have pushed peer id onto message as if it
             * were a request, but the effect we want for responses is to have
             * a route popped off at each router hop.
             */
            (void)flux_msg_route_delete_last (msg); // child id from ROUTER
            (void)flux_msg_route_delete_last (msg); // my id
            rpc_track_update (child->tracker, msg);
            break;
        case FLUX_MSGTYPE_EVENT:
            break;
    }
    ov->recv_cb (msg, OVERLAY_DOWNSTREAM, ov->recv_arg);
done:
    flux_msg_decref (msg);
}

/* Parent endpoint disconnected, so any pending RPCs going that way
 * get EHOSTUNREACH responses so they can fail fast.
 */
static void fail_parent_rpc (const flux_msg_t *msg, void *arg)
{
    struct overlay *ov = arg;

    if (flux_respond_error (ov->h,
                            msg,
                            EHOSTUNREACH,
                            "overlay disconnect") < 0)
        log_tracker_error (ov->h, msg, errno);
}

static void parent_cb (flux_reactor_t *r, flux_watcher_t *w,
                       int revents, void *arg)
{
    struct overlay *ov = arg;
    flux_msg_t *msg;
    int type;
    const char *topic = NULL;

    if (!(msg = zmqutil_msg_recv (ov->parent.zsock)))
        return;
    if (flux_msg_get_type (msg, &type) < 0) {
        logdrop (ov, OVERLAY_UPSTREAM, msg, "malformed message");
        goto done;
    }
    if (!ov->parent.hello_responded) {
        /* process hello response */
        if (type == FLUX_MSGTYPE_RESPONSE
            && flux_msg_get_topic (msg, &topic) == 0
            && streq (topic, "overlay.hello")) {
            hello_response_handler (ov, msg);
        }
        else {
            logdrop (ov, OVERLAY_UPSTREAM, msg,
                     "message received before hello handshake completed");
        }
        goto done;
    }
    switch (type) {
        case FLUX_MSGTYPE_RESPONSE:
            rpc_track_update (ov->parent.tracker, msg);
            break;
        case FLUX_MSGTYPE_EVENT:
            /* Upstream broker enables routing and pushes our uuid, then
             * router socket pops it off, but leaves routing enabled.
             * An event type message should not have routing enabled
             * under normal circumstances, so turn it off here.
             */
            flux_msg_route_disable (msg);
            break;
        case FLUX_MSGTYPE_KEEPALIVE: {
            int ktype, reason;
            if (flux_keepalive_decode (msg, &ktype, &reason) < 0) {
                logdrop (ov, OVERLAY_UPSTREAM, msg, "malformed keepalive");
            }
            else if (ktype == KEEPALIVE_DISCONNECT) {
                flux_log (ov->h, LOG_ERR, "parent disconnect");
                (void)zsock_disconnect (ov->parent.zsock, "%s", ov->parent.uri);
                ov->parent.offline = true;
                rpc_track_purge (ov->parent.tracker, fail_parent_rpc, ov);
                overlay_monitor_notify (ov);
            }
            else
                logdrop (ov, OVERLAY_UPSTREAM, msg, "unknown keepalive type");
            goto done;
        }
        default:
            break;
    }
    ov->recv_cb (msg, OVERLAY_UPSTREAM, ov->recv_arg);
done:
    flux_msg_destroy (msg);
}

/* Check child flux-core version 'v1' against this broker's version 'v2'.
 * For now we require an exact match of (major,minor,patch) and
 * ignore any commit id appended to the version string.
 * Return 0 on error, or -1 on failure with message for child in 'errubuf'.
 */
static bool version_check (int v1, int v2, char *errbuf, int errbufsz)
{
    if (v1 != v2) {
        snprintf (errbuf, errbufsz,
                  "flux-core v%u.%u.%u mismatched with parent v%u.%u.%u",
                  (v1 >> 16) & 0xff,
                  (v1 >> 8) & 0xff,
                  v1 & 0xff,
                  (v2 >> 16) & 0xff,
                  (v2 >> 8) & 0xff,
                  v2 & 0xff);
        return false;
    }
    return true;
}

/* Handle overlay.hello request from downstream (child) TBON peer.
 * The peer may be rejected here if it is improperly configured.
 * If successful the child's status is updated to reflect its online
 * state and the state machine is notified.
 * N.B. respond using overlay_sendmsg_child() to avoid the main message path
 * during initialization.
 */
static void hello_request_handler (struct overlay *ov, const flux_msg_t *msg)
{
    struct child *child;
    json_int_t rank;
    int version;
    const char *errmsg = NULL;
    char errbuf[128];
    flux_msg_t *response;
    const char *uuid;
    int status;
    int hello_log_level = LOG_INFO;

    if (flux_request_unpack (msg,
                             NULL,
                             "{s:I s:i s:s s:i}",
                             "rank", &rank,
                             "version", &version,
                             "uuid", &uuid,
                             "status", &status) < 0
        || flux_msg_authorize (msg, FLUX_USERID_UNKNOWN) < 0)
        goto error; // EPROTO or EPERM (unlikely)

    if (!(child = child_lookup_byrank (ov, rank))) {
        snprintf (errbuf, sizeof (errbuf),
                  "rank %lu is not a peer of parent %lu: mismatched config?",
                  (unsigned long)rank,
                  (unsigned long)ov->parent.rank);
        errmsg = errbuf;
        errno = EINVAL;
        goto error_log;
    }
    /* Oops, child was previously online, but is now saying hello.
     * Update the (old) child's subtree status to LOST.  If the hello
     * request is successful, another update will immediately follow.
     */
    if (subtree_is_online (child->status)) { // crash
        flux_log (ov->h, LOG_ERR,
                  "rank %lu uuid %s lost, to be superceded by new hello",
                  (unsigned long)child->rank,
                  child->uuid);
        overlay_child_status_update (ov, child, SUBTREE_STATUS_LOST);
        hello_log_level = LOG_ERR; // want hello log to stand out in this case
    }

    if (!version_check (version, ov->version, errbuf, sizeof (errbuf))) {
        errmsg = errbuf;
        errno = EINVAL;
        goto error_log;
    }

    snprintf (child->uuid, sizeof (child->uuid), "%s", uuid);
    overlay_child_status_update (ov, child, status);

    flux_log (ov->h,
              hello_log_level,
              "hello from rank %lu uuid %s status %s",
              (unsigned long)child->rank,
              child->uuid,
              subtree_status_str (child->status));

    if (!(response = flux_response_derive (msg, 0))
        || flux_msg_pack (response, "{s:s}", "uuid", ov->uuid) < 0
        || overlay_sendmsg_child (ov, response) < 0)
        flux_log_error (ov->h, "error responding to overlay.hello request");
    flux_msg_destroy (response);
    return;
error_log:
    flux_log (ov->h, LOG_ERR, "overlay hello child %lu %s rejected",
              (unsigned long)rank,
              uuid);
error:
    if (!(response = flux_response_derive (msg, errno))
        || (errmsg && flux_msg_set_string (response, errmsg) < 0)
        || overlay_sendmsg_child (ov, response) < 0)
        flux_log_error (ov->h, "error responding to overlay.hello request");
    flux_msg_destroy (response);
}

/* Process overlay.hello response.
 * If the response indicates an error, set in motion a clean broker exit by
 * printing the error message to stderr and notifying the state machine
 * that it should check overlay_parent_error() / overlay_parent_success().
 */
static void hello_response_handler (struct overlay *ov, const flux_msg_t *msg)
{
    const char *errstr = NULL;
    const char *uuid;

    if (flux_response_decode (msg, NULL, NULL) < 0
        || flux_msg_unpack (msg, "{s:s}", "uuid", &uuid) < 0) {
        int saved_errno = errno;
        (void)flux_msg_get_string (msg, &errstr);
        errno = saved_errno;
        goto error;
    }
    flux_log (ov->h, LOG_DEBUG, "hello parent %lu %s",
              (unsigned long)ov->parent.rank, uuid);
    snprintf (ov->parent.uuid, sizeof (ov->parent.uuid), "%s", uuid);
    ov->parent.hello_responded = true;
    ov->parent.hello_error = false;
    overlay_monitor_notify (ov);
    return;
error:
    log_msg ("overlay.hello: %s", errstr ? errstr : flux_strerror (errno));
    ov->parent.hello_responded = true;
    ov->parent.hello_error = true;
    overlay_monitor_notify (ov);
}

/* Send overlay.hello message to TBON parent.
 */
static int hello_request_send (struct overlay *ov,
                               json_int_t rank,
                               int version)
{
    flux_msg_t *msg;

    if (!(msg = flux_request_encode ("overlay.hello", NULL))
        || flux_msg_pack (msg,
                          "{s:I s:i s:s s:i}",
                          "rank", rank,
                          "version", ov->version,
                          "uuid", ov->uuid,
                          "status", ov->status) < 0
        || flux_msg_set_rolemask (msg, FLUX_ROLE_OWNER) < 0
        || overlay_sendmsg_parent (ov, msg) < 0) {
        flux_msg_decref (msg);
        return -1;
    }
    flux_msg_decref (msg);
    return 0;
}

static void bind_monitor_cb (struct zmqutil_monitor *mon, void *arg)
{
    struct overlay *ov = arg;
    struct monitor_event event;

    if (zmqutil_monitor_get (mon, &event) == 0) {
        flux_log (ov->h,
                  zmqutil_monitor_iserror (&event) ? LOG_ERR : LOG_DEBUG,
                  "child sockevent %s %s%s%s",
                  event.endpoint,
                  event.event_str,
                  *event.value_str ? ": " : "",
                  event.value_str);
    }
}
static void parent_monitor_cb (struct zmqutil_monitor *mon, void *arg)
{
    struct overlay *ov = arg;
    struct monitor_event event;

    if (zmqutil_monitor_get (mon, &event) == 0) {
        flux_log (ov->h,
                  zmqutil_monitor_iserror (&event) ? LOG_ERR : LOG_DEBUG,
                  "parent sockevent %s %s%s%s",
                  event.endpoint,
                  event.event_str,
                  *event.value_str ? ": " : "",
                  event.value_str);
    }
}

int overlay_connect (struct overlay *ov)
{
    if (ov->rank > 0) {
        if (!ov->h || ov->rank == FLUX_NODEID_ANY || !ov->parent.uri) {
            errno = EINVAL;
            return -1;
        }
        if (!(ov->parent.zsock = zsock_new_dealer (NULL)))
            goto nomem;
        /* The socket monitor is only used for logging.
         * Setup may fail if libzmq is too old.
         */
        if (ov->zmqdebug) {
            ov->parent.monitor = zmqutil_monitor_create (ov->parent.zsock,
                                                         ov->reactor,
                                                         parent_monitor_cb,
                                                         ov);
        }
        zsock_set_unbounded (ov->parent.zsock);
        zsock_set_linger (ov->parent.zsock, 5);
        zsock_set_ipv6 (ov->parent.zsock, ov->enable_ipv6);
        zsock_set_zap_domain (ov->parent.zsock, FLUX_ZAP_DOMAIN);
        zcert_apply (ov->cert, ov->parent.zsock);
        zsock_set_curve_serverkey (ov->parent.zsock, ov->parent.pubkey);
        zsock_set_identity (ov->parent.zsock, ov->uuid);
        if (zsock_connect (ov->parent.zsock, "%s", ov->parent.uri) < 0)
            goto nomem;
        if (!(ov->parent.w = zmqutil_watcher_create (ov->reactor,
                                                     ov->parent.zsock,
                                                     FLUX_POLLIN,
                                                     parent_cb,
                                                     ov)))
            return -1;
        flux_watcher_start (ov->parent.w);
        if (hello_request_send (ov, ov->rank, FLUX_CORE_VERSION_HEX) < 0)
            return -1;
    }
    return 0;
nomem:
    errno = ENOMEM;
    return -1;
}

static void zaplogger (int severity, const char *message, void *arg)
{
    struct overlay *ov = arg;

    flux_log (ov->h, severity, "%s", message);
}

int overlay_bind (struct overlay *ov, const char *uri)
{
    if (!ov->h || ov->rank == FLUX_NODEID_ANY || ov->bind_zsock) {
        errno = EINVAL;
        log_err ("overlay_bind: invalid arguments");
        return -1;
    }

    assert (ov->zap == NULL);
    if (!(ov->zap = zmqutil_zap_create (ov->reactor))) {
        log_err ("error creating ZAP server");
        return -1;
    }
    zmqutil_zap_set_logger (ov->zap, zaplogger, ov);

    if (!(ov->bind_zsock = zsock_new_router (NULL))) {
        log_err ("error creating zmq ROUTER socket");
        return -1;
    }
    /* The socket monitor is only used for logging.
     * Setup may fail if libzmq is too old.
     */
    if (attr_get (ov->attrs, "tbon.zmqdebug", NULL, NULL) == 0) {
        ov->bind_monitor = zmqutil_monitor_create (ov->bind_zsock,
                                                   ov->reactor,
                                                   bind_monitor_cb,
                                                   ov);
    }
    zsock_set_unbounded (ov->bind_zsock);
    zsock_set_linger (ov->bind_zsock, 5);
    zsock_set_router_mandatory (ov->bind_zsock, 1);
    zsock_set_ipv6 (ov->bind_zsock, ov->enable_ipv6);

    zsock_set_zap_domain (ov->bind_zsock, FLUX_ZAP_DOMAIN);
    zcert_apply (ov->cert, ov->bind_zsock);
    zsock_set_curve_server (ov->bind_zsock, 1);

    if (zsock_bind (ov->bind_zsock, "%s", uri) < 0) {
        log_err ("error binding to %s", uri);
        return -1;
    }
    /* Capture URI after zsock_bind() processing, so it reflects expanded
     * wildcards and normalized addresses.
     */
    if (!(ov->bind_uri = zsock_last_endpoint (ov->bind_zsock))) {
        log_err ("error obtaining concretized bind URI");
        return -1;
    }
    if (!(ov->bind_w = zmqutil_watcher_create (ov->reactor,
                                               ov->bind_zsock,
                                               FLUX_POLLIN,
                                               child_cb,
                                               ov))) {
        log_err ("error creating watcher for bind socket");
        return -1;
    }
    flux_watcher_start (ov->bind_w);
    /* Ensure that ipc files are removed when the broker exits.
     */
    char *ipc_path = strstr (ov->bind_uri, "ipc://");
    if (ipc_path)
        cleanup_push_string (cleanup_file, ipc_path + 6);
    return 0;
}

/* Call after overlay bootstrap (bind/connect),
 * to get concretized 0MQ endpoints.
 */
int overlay_register_attrs (struct overlay *overlay)
{
    if (attr_add (overlay->attrs,
                  "tbon.parent-endpoint",
                  overlay->parent.uri,
                  FLUX_ATTRFLAG_IMMUTABLE) < 0)
        return -1;
    if (attr_add_uint32 (overlay->attrs,
                         "rank",
                         overlay->rank,
                         FLUX_ATTRFLAG_IMMUTABLE) < 0)
        return -1;
    if (attr_add_uint32 (overlay->attrs,
                         "size", overlay->size,
                         FLUX_ATTRFLAG_IMMUTABLE) < 0)
        return -1;
    if (attr_add_int (overlay->attrs,
                      "tbon.level",
                      kary_levelof (overlay->fanout, overlay->rank),
                      FLUX_ATTRFLAG_IMMUTABLE) < 0)
        return -1;
    if (attr_add_int (overlay->attrs,
                      "tbon.maxlevel",
                      kary_levelof (overlay->fanout, overlay->size - 1),
                      FLUX_ATTRFLAG_IMMUTABLE) < 0)
        return -1;
    if (attr_add_int (overlay->attrs,
                      "tbon.descendants",
                      kary_sum_descendants (overlay->fanout,
                                            overlay->size,
                                            overlay->rank),
                      FLUX_ATTRFLAG_IMMUTABLE) < 0)
        return -1;

    return 0;
}

void overlay_set_monitor_cb (struct overlay *ov,
                             overlay_monitor_f cb,
                             void *arg)
{
    ov->child_monitor_cb = cb;
    ov->child_monitor_arg = arg;
}

void overlay_set_loss_cb (struct overlay *ov,
                          overlay_loss_f cb,
                          void *arg)
{
    ov->loss_cb = cb;
    ov->loss_arg = arg;
}

static int child_rpc_track_count (struct overlay *ov)
{
    int count = 0;
    int i;
    for (i = 0; i < ov->child_count; i++)
        count += rpc_track_count (ov->children[i].tracker);
    return count;
}

static void overlay_stats_get_cb (flux_t *h,
                                  flux_msg_handler_t *mh,
                                  const flux_msg_t *msg,
                                  void *arg)
{
    struct overlay *ov = arg;

    if (flux_request_decode (msg, NULL, NULL) < 0)
        goto error;
    if (flux_respond_pack (h,
                           msg,
                           "{s:i s:i s:i s:i s:i}",
                           "child-count", ov->child_count,
                           "child-connected", overlay_get_child_peer_count (ov),
                           "parent-count", ov->rank > 0 ? 1 : 0,
                           "parent-rpc", rpc_track_count (ov->parent.tracker),
                           "child-rpc", child_rpc_track_count (ov)) < 0)
        flux_log_error (h, "error responding to overlay.stats.get");
    return;
error:
    if (flux_respond_error (h, msg, errno, NULL) < 0)
        flux_log_error (h, "error responding to overlay.stats.get");
}

static int overlay_health_respond (struct overlay *ov, const flux_msg_t *msg)
{
    struct child *child;
    json_t *array = NULL;
    json_t *entry;
    double duration;

    if (!(array = json_array ()))
        goto nomem;
    foreach_overlay_child (ov, child) {
        duration = monotime_since (child->status_timestamp) / 1000.0;
        if (!(entry = json_pack ("{s:i s:s s:f}",
                                 "rank", child->rank,
                                 "status", subtree_status_str (child->status),
                                 "duration", duration)))
            goto nomem;
        if (json_array_append_new (array, entry) < 0) {
            json_decref (entry);
            goto nomem;
        }
    }
    duration = monotime_since (ov->status_timestamp) / 1000.0;
    if (flux_respond_pack (ov->h,
                           msg,
                           "{s:i s:s s:f s:O}",
                           "rank", ov->rank,
                           "status", subtree_status_str (ov->status),
                           "duration", duration,
                           "children", array) < 0)
        flux_log_error (ov->h, "error responding to overlay.health");
    json_decref (array);
    return 0;
nomem:
    errno = ENOMEM;
    return -1;
}

static void overlay_health_respond_all (struct overlay *ov)
{
    const flux_msg_t *msg;

    msg = flux_msglist_first (ov->health_requests);
    while (msg) {
        if (overlay_health_respond (ov, msg) < 0)
            flux_log_error (ov->h, "error responding to overlay.health");
        msg = flux_msglist_next (ov->health_requests);
    }
}

static void overlay_health_cb (flux_t *h,
                               flux_msg_handler_t *mh,
                               const flux_msg_t *msg,
                               void *arg)
{
    struct overlay *ov = arg;

    if (flux_request_decode (msg, NULL, NULL) < 0)
        goto error;
    if (flux_msg_is_streaming (msg)) {
        if (flux_msglist_append (ov->health_requests, msg) < 0)
            goto error;
    }
    if (overlay_health_respond (ov, msg) < 0)
        goto error;
    return;
error:
    if (flux_respond_error (h, msg, errno, NULL) < 0)
        flux_log_error (h, "error responding to overlay.health");
}

/* If a disconnect is received for waiting health request,
 * drop the request.
 */
static void disconnect_cb (flux_t *h,
                           flux_msg_handler_t *mh,
                           const flux_msg_t *msg,
                           void *arg)
{
    struct overlay *ov = arg;
    int count;

    if ((count = flux_msglist_disconnect (ov->health_requests, msg)) < 0)
        flux_log_error (h, "error handling overlay.disconnect");
    if (count > 0)
        flux_log (h, LOG_DEBUG, "overlay: goodbye to %d health clients", count);
}

/* Recursive function to build subtree topology object.
 * Right now the tree is regular.  In the future support the configuration
 * of irregular tree topologies.
 */
static json_t *get_subtree_topo (struct overlay *ov, int rank)
{
    json_t *o;
    json_t *children;
    json_t *child;
    int i, r;
    int size;

    if (!(children = json_array()))
        goto nomem;
    for (i = 0; i < ov->fanout; i++) {
        r = kary_childof (ov->fanout, ov->size, rank, i);
        if (r == KARY_NONE)
            break;
        if (!(child = get_subtree_topo (ov, r)))
            goto error;
        if (json_array_append_new (children, child) < 0) {
            json_decref (child);
            goto nomem;
        }
    }
    size = kary_sum_descendants (ov->fanout, ov->size, rank) + 1;
    if (!(o = json_pack ("{s:i s:i s:O}",
                         "rank", rank,
                         "size", size,
                         "children", children)))
        goto nomem;
    json_decref (children);
    return o;
nomem:
    errno = ENOMEM;
error:
    ERRNO_SAFE_WRAP (json_decref, children);
    return NULL;
}

/* Get the topology of the subtree rooted here.
 */
static void overlay_topology_cb (flux_t *h,
                                 flux_msg_handler_t *mh,
                                 const flux_msg_t *msg,
                                 void *arg)
{
    struct overlay *ov = arg;
    json_t *topo = NULL;
    int rank;
    const char *errstr = NULL;

    if (flux_request_unpack (msg, NULL, "{s:i}", "rank", &rank) < 0)
        goto error;
    if (rank != ov->rank && !child_lookup_byrank (ov, rank)) {
        errstr = "requested rank is not this broker or its direct child";
        errno = ENOENT;
        goto error;
    }
    if (!(topo = get_subtree_topo (ov, rank)))
        goto error;
    if (flux_respond_pack (h, msg, "O", topo) < 0)
        flux_log_error (h, "error responding to overlay.topology");
    json_decref (topo);
    return;
error:
    if (flux_respond_error (h, msg, errno, errstr) < 0)
        flux_log_error (h, "error responding to overlay.topology");
    json_decref (topo);
}

/* Administratively force a disconnect of subtree rooted at 'rank'.
 */
static void overlay_disconnect_subtree_cb (flux_t *h,
                                           flux_msg_handler_t *mh,
                                           const flux_msg_t *msg,
                                           void *arg)
{
    struct overlay *ov = arg;
    const char *errstr = NULL;
    char errbuf[128];
    int rank;
    struct child *child;

    if (flux_request_unpack (msg, NULL, "{s:i}", "rank", &rank) < 0)
        goto error;
    if (!(child = child_lookup_byrank (ov, rank))) {
        errstr = "requested rank is not this broker's direct child";
        errno = ENOENT;
        goto error;
    }
    if (!subtree_is_online (child->status)) {
        snprintf (errbuf, sizeof (errbuf), "rank %d is already %s", rank,
                  subtree_status_str (child->status));
        errstr = errbuf;
        errno = EINVAL;
        goto error;
    }
    if (overlay_keepalive_child (ov,
                                 child->uuid,
                                 KEEPALIVE_DISCONNECT, 0) < 0) {
        errstr = "failed to send KEEPALIVE_DISCONNECT message";
        goto error;
    }
    overlay_child_status_update (ov, child, SUBTREE_STATUS_LOST);
    if (flux_respond (h, msg, NULL) < 0)
        flux_log_error (h, "error responding to overlay.disconnect-subtree");
    return;
error:
    if (flux_respond_error (h, msg, errno, errstr) < 0)
        flux_log_error (h, "error responding to overlay.disconnect-subtree");
}

int overlay_cert_load (struct overlay *ov, const char *path)
{
    struct stat sb;
    zcert_t *cert;

    if (stat (path, &sb) < 0) {
        log_err ("%s", path);
        return -1;
    }
    if ((sb.st_mode & S_IROTH) | (sb.st_mode & S_IRGRP)) {
        log_msg ("%s: readable by group/other", path);
        errno = EPERM;
        return -1;
    }
    if (!(cert = zcert_load (path))) {
        log_msg ("%s: invalid CURVE certificate", path);
        errno = EINVAL;
        return -1;
    }
    zcert_destroy (&ov->cert);
    ov->cert = cert;
    return 0;
}

const char *overlay_cert_pubkey (struct overlay *ov)
{
    return zcert_public_txt (ov->cert);
}

const char *overlay_cert_name (struct overlay *ov)
{
    return zcert_meta (ov->cert, "name");
}

int overlay_authorize (struct overlay *ov,
                       const char *name,
                       const char *pubkey)
{
    if (!ov->zap) {
        errno = EINVAL;
        return -1;
    }
    return zmqutil_zap_authorize (ov->zap, name, pubkey);
}

static int overlay_configure_attr_int (attr_t *attrs,
                                       const char *name,
                                       int default_value,
                                       int *valuep)
{
    int value = default_value;
    const char *val;
    char *endptr;

    if (attr_get (attrs, name, &val, NULL) == 0) {
        errno = 0;
        value = strtol (val, &endptr, 10);
        if (errno != 0 || *endptr != '\0') {
            log_msg ("%s value must be an integer", name);
            errno = EINVAL;
            return -1;
        }
        if (attr_delete (attrs, name, true) < 0)
            return -1;
    }
    if (attr_add_int (attrs, name, value, FLUX_ATTRFLAG_IMMUTABLE) < 0)
        return -1;
    if (valuep)
        *valuep = value;
    return 0;
}

void overlay_destroy (struct overlay *ov)
{
    if (ov) {
        int saved_errno = errno;

        flux_msglist_destroy (ov->health_requests);

        zcert_destroy (&ov->cert);
        zmqutil_zap_destroy (ov->zap);

        flux_future_destroy (ov->f_sync);
        flux_msg_handler_delvec (ov->handlers);
        ov->status = SUBTREE_STATUS_OFFLINE;
        overlay_keepalive_parent (ov, KEEPALIVE_STATUS, ov->status);

        zsock_destroy (&ov->parent.zsock);
        free (ov->parent.uri);
        flux_watcher_destroy (ov->parent.w);
        free (ov->parent.pubkey);
        zmqutil_monitor_destroy (ov->parent.monitor);

        zsock_destroy (&ov->bind_zsock);
        free (ov->bind_uri);
        flux_watcher_destroy (ov->bind_w);
        zmqutil_monitor_destroy (ov->bind_monitor);

        zhashx_destroy (&ov->child_hash);
        if (ov->children) {
            int i;
            for (i = 0; i < ov->child_count; i++)
                rpc_track_destroy (ov->children[i].tracker);
            free (ov->children);
        }
        rpc_track_destroy (ov->parent.tracker);
        free (ov);
        errno = saved_errno;
    }
}

static const struct flux_msg_handler_spec htab[] = {
    {
        FLUX_MSGTYPE_REQUEST,
        "overlay.stats.get",
        overlay_stats_get_cb,
        0
    },
    {
        FLUX_MSGTYPE_REQUEST,
        "overlay.health",
        overlay_health_cb,
        0
    },
    {
        FLUX_MSGTYPE_REQUEST,
        "overlay.disconnect",
        disconnect_cb,
        0
    },
    {
        FLUX_MSGTYPE_REQUEST,
        "overlay.topology",
        overlay_topology_cb,
        0
    },
    {
        FLUX_MSGTYPE_REQUEST,
        "overlay.disconnect-subtree",
        overlay_disconnect_subtree_cb,
        0
    },
    FLUX_MSGHANDLER_TABLE_END,
};

struct overlay *overlay_create (flux_t *h,
                                attr_t *attrs,
                                overlay_recv_f cb,
                                void *arg)
{
    struct overlay *ov;
    uuid_t uuid;

    if (!(ov = calloc (1, sizeof (*ov))))
        return NULL;
    ov->attrs = attrs;
    ov->rank = FLUX_NODEID_ANY;
    ov->parent.lastsent = -1;
    ov->h = h;
    ov->reactor = flux_get_reactor (h);
    ov->recv_cb = cb;
    ov->recv_arg = arg;
    ov->version = FLUX_CORE_VERSION_HEX;
    uuid_generate (uuid);
    uuid_unparse (uuid, ov->uuid);
    if (overlay_configure_attr_int (ov->attrs,
                                    "tbon.fanout",
                                    DEFAULT_FANOUT,
                                    &ov->fanout) < 0)
        goto error;
    if (ov->fanout < 1) {
        log_msg ("tbon.fanout must be >= 1");
        errno = EINVAL;
        goto error;
    }
    if (overlay_configure_attr_int (ov->attrs,
                                    "tbon.zmqdebug",
                                    0,
                                    &ov->zmqdebug) < 0)
        goto error;
    if (overlay_configure_attr_int (ov->attrs, "tbon.prefertcp", 0, NULL) < 0)
        goto error;
    if (flux_msg_handler_addvec (h, htab, ov, &ov->handlers) < 0)
        goto error;
    if (!(ov->f_sync = flux_sync_create (h, sync_min))
        || flux_future_then (ov->f_sync, sync_max, sync_cb, ov) < 0)
        goto error;
    if (!(ov->cert = zcert_new ()))
        goto nomem;
    if (!(ov->health_requests = flux_msglist_create ()))
        goto error;
    return ov;
nomem:
    errno = ENOMEM;
error:
    overlay_destroy (ov);
    return NULL;
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
