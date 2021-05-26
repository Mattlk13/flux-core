/************************************************************\
 * Copyright 2019 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

/* builtin PMI-1 plugin for jobs
 *
 * Provide PMI-1 service so that an MPI or Flux job can bootstrap.
 * Much of the work is done by the PMI-1 wire protocol engine in
 * libpmi/simple_server.c and libsubprocess socketpair channels.
 *
 * At startup this module is registered as a builtin shell plugin under
 * the name "pmi" via an entry in builtins.c builtins array.
 *
 * At shell "init", the plugin intiailizes a PMI object including the
 * pmi simple server and empty local kvs cache.
 *
 * During each task's "task init" callback, the pmi plugin sets up the
 * subprocess channel, sets the PMI_FD, PMI_RANK, and PMI_SIZE environment
 * variables, and subscribes to the newly created PMI_FD channel in order
 * to read PMI requests.
 *
 * The output callback pmi_fd_read_cb() reads the request from the PMI_FD
 * channel and pushes it into the PMI-1 protocol engine.  If the request
 * can be immediately answered, the shell_pmi_response_send() callback
 * registered with the engine is invoked, which writes the response to
 * the subprocess channel.
 *
 * Other requests have callbacks from the engine to provide data,
 * which is fed back to the engine, which then calls shell_pmi_response_send().
 * These are kvs_get, kvs_put, and barrier.  Although the task
 * is effectively blocked while these callbacks are handled, they are
 * implemented with asynchronous continuation callbacks so that other tasks
 * and the shell's reactor remain live while the task awaits an answer.
 *
 * If shell->verbose is true (shell --verbose flag was provided), the
 * protocol engine emits client and server telemetry to stderr, and
 * shell_pmi_task_ready() logs read errors, EOF, and finalization to stderr
 * in a compatible format.
 *
 * Caveats:
 * - PMI kvsname parameter is ignored
 * - 64-bit Flux job id's are assigned to integer-typed PMI appnum
 * - PMI publish, unpublish, lookup, spawn are not implemented
 * - Teardown of the subprocess channel is deferred until task completion,
 *   although client closes its end after PMI_Finalize().
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <flux/core.h>
#include <jansson.h>

#include "src/common/libczmqcontainers/czmq_containers.h"
#include "src/common/libpmi/simple_server.h"
#include "src/common/libpmi/clique.h"
#include "src/common/libutil/errno_safe.h"

#include "builtins.h"
#include "internal.h"
#include "task.h"
#include "pmi_exchange.h"

struct shell_pmi {
    flux_shell_t *shell;
    struct pmi_simple_server *server;
    json_t *global; // already exchanged
    json_t *pending;// pending to be exchanged
    json_t *locals;  // never exchanged
    struct pmi_exchange *exchange;
};

/* pmi_simple_ops->abort() signature */
static void shell_pmi_abort (void *arg,
                             void *client,
                             int exit_code,
                             const char *msg)
{
    /* Generate job exception (exit_code ignored for now) */
    shell_die (exit_code,
               "MPI_Abort%s%s",
               msg ? ": " : "",
               msg ? msg : "");
}

static int put_dict (json_t *dict, const char *key, const char *val)
{
    json_t *o;

    if (!(o = json_string (val)))
        goto nomem;
    if (json_object_set_new (dict, key, o) < 0) {
        json_decref (o);
        goto nomem;
    }
    return 0;
nomem:
    errno = ENOMEM;
    return -1;
}

/**
 ** ops for using native Flux KVS for PMI KVS
 ** This is used if pmi.kvs=native option is provided.
 **/

static void native_lookup_continuation (flux_future_t *f, void *arg)
{
    struct shell_pmi *pmi = arg;
    void *cli = flux_future_aux_get (f, "pmi_cli");
    const char *val = NULL;

    (void)flux_kvs_lookup_get (f, &val); // leave val=NULL on failure
    pmi_simple_server_kvs_get_complete (pmi->server, cli, val);
    flux_future_destroy (f);
}

static int native_lookup (struct shell_pmi *pmi, const char *key, void *cli)
{
    char *nkey;
    flux_future_t *f;

    if (asprintf (&nkey, "pmi.%s", key) < 0)
        return -1;
    if (!(f = flux_kvs_lookup (pmi->shell->h, NULL, 0, nkey)))
        return -1;
    if (flux_future_aux_set (f, "pmi_cli", cli, NULL) < 0)
        goto error;
    if (flux_future_then (f, -1, native_lookup_continuation, pmi) < 0)
        goto error;
    free (nkey);
    return 0;
error:
    ERRNO_SAFE_WRAP (free, nkey);
    flux_future_destroy (f);
    return -1;
}

static void native_fence_continuation (flux_future_t *f, void *arg)
{
    struct shell_pmi *pmi = arg;
    int rc = flux_future_get (f, NULL);
    pmi_simple_server_barrier_complete (pmi->server, rc);

    flux_future_destroy (f);
    json_object_clear (pmi->pending);
}

static int native_fence (struct shell_pmi *pmi)
{
    flux_kvs_txn_t *txn;
    const char *key;
    json_t *val;
    char *nkey;
    int rc;
    char name[64];
    static int seq = 0;
    uintmax_t id = (uintmax_t)pmi->shell->jobid;
    int size = pmi->shell->info->shell_size;
    flux_future_t *f = NULL;

    if (!(txn = flux_kvs_txn_create ()))
        return -1;
    json_object_foreach (pmi->pending, key, val) {
        if (asprintf (&nkey, "pmi.%s", key) < 0)
            goto error;
        rc = flux_kvs_txn_put (txn, 0, nkey, json_string_value (val));
        ERRNO_SAFE_WRAP (free, nkey);
        if (rc < 0)
            goto error;
    }
    (void)snprintf (name, sizeof (name), "%juPMI%d", id, seq++);
    if (!(f = flux_kvs_fence (pmi->shell->h, NULL, 0, name, size, txn)))
        goto error;
    if (flux_future_then (f, -1, native_fence_continuation, pmi) < 0)
        goto error;
    flux_kvs_txn_destroy (txn);
    return 0;
error:
    flux_future_destroy (f);
    flux_kvs_txn_destroy (txn);
    return -1;
}

/* pmi_simple_ops->kvs_put() signature */
static int native_kvs_put (void *arg,
                           const char *kvsname,
                           const char *key,
                           const char *val)
{
    struct shell_pmi *pmi = arg;

    return put_dict (pmi->pending, key, val);
}

/* pmi_simple_ops->barrier_enter() signature */
static int native_barrier_enter (void *arg)
{
    struct shell_pmi *pmi = arg;

    if (pmi->shell->info->shell_size == 1) {
        pmi_simple_server_barrier_complete (pmi->server, 0);
        return 0;
    }
    if (native_fence (pmi) < 0)
        return -1; // PMI_FAIL
    return 0;
}

/* pmi_simple_ops->kvs_get() signature */
static int native_kvs_get (void *arg,
                           void *cli,
                           const char *kvsname,
                           const char *key)
{
    struct shell_pmi *pmi = arg;
    json_t *o;
    const char *val = NULL;

    if ((o = json_object_get (pmi->locals, key))
            || (o = json_object_get (pmi->pending, key))) {
        val = json_string_value (o);
        pmi_simple_server_kvs_get_complete (pmi->server, cli, val);
        return 0;
    }
    if (pmi->shell->info->shell_size > 1) {
        if (native_lookup (pmi, key, cli) == 0)
            return 0; // response deferred
    }
    return -1; // PMI_ERR_INVALID_KEY
}

/**
 ** ops for using purpose-built dict exchange for PMI KVS
 ** This is used if pmi.kvs=exchange option is provided.
 **/

static void exchange_cb (struct pmi_exchange *pex, void *arg)
{
    struct shell_pmi *pmi = arg;
    int rc = -1;

    if (pmi_exchange_has_error (pex)) {
        shell_warn ("exchange failed");
        goto done;
    }
    if (json_object_update (pmi->global, pmi_exchange_get_dict (pex)) < 0) {
        shell_warn ("failed to update dict after successful exchange");
        goto done;
    }
    json_object_clear (pmi->pending);
    rc = 0;
done:
    pmi_simple_server_barrier_complete (pmi->server, rc);
}

/* pmi_simple_ops->kvs_get() signature */
static int exchange_kvs_get (void *arg,
                              void *cli,
                              const char *kvsname,
                              const char *key)
{
    struct shell_pmi *pmi = arg;
    json_t *o;
    const char *val = NULL;

    if ((o = json_object_get (pmi->locals, key))
            || (o = json_object_get (pmi->pending, key))
            || (o = json_object_get (pmi->global, key))) {
        val = json_string_value (o);
        pmi_simple_server_kvs_get_complete (pmi->server, cli, val);
        return 0;
    }
    return -1; // PMI_ERR_INVALID_KEY
}

/* pmi_simple_ops->barrier_enter() signature */
static int exchange_barrier_enter (void *arg)
{
    struct shell_pmi *pmi = arg;

    if (pmi->shell->info->shell_size == 1) {
        pmi_simple_server_barrier_complete (pmi->server, 0);
        return 0;
    }
    if (pmi_exchange (pmi->exchange,
                      pmi->pending,
                      exchange_cb,
                      pmi) < 0) {
        shell_warn ("pmi_exchange %s", flux_strerror (errno));
        return -1; // PMI_FAIL
    }
    return 0;
}

/* pmi_simple_ops->kvs_put() signature */
static int exchange_kvs_put (void *arg,
                              const char *kvsname,
                              const char *key,
                              const char *val)
{
    struct shell_pmi *pmi = arg;

    return put_dict (pmi->pending, key, val);
}

/**
 ** end of KVS implementations
 **/

/* pmi_simple_ops->response_send() signature */
static int shell_pmi_response_send (void *client, const char *buf)
{
    struct shell_task *task = client;

    return flux_subprocess_write (task->proc, "PMI_FD", buf, strlen (buf));
}

/* pmi_simple_ops->debug_trace() signature */
static void shell_pmi_debug_trace (void *client, const char *line)
{
    struct shell_task *task = client;

    shell_trace ("%d: %s", task->rank, line);
}

static void pmi_fd_cb (flux_shell_task_t *task,
                       const char *stream,
                       void *arg)
{
    struct shell_pmi *pmi = arg;
    int len;
    const char *line;
    int rc;

    line = flux_subprocess_read_line (task->proc, "PMI_FD", &len);
    if (len < 0) {
        shell_trace ("%d: C: pmi read error: %s",
                     task->rank, flux_strerror (errno));
        return;
    }
    if (len == 0) {
        shell_trace ("%d: C: pmi EOF", task->rank);
        return;
    }
    rc = pmi_simple_server_request (pmi->server, line, task, task->rank);
    if (rc < 0) {
        shell_trace ("%d: S: pmi request error", task->rank);
        shell_die (1, "PMI-1 wire protocol error");

    }
    if (rc == 1) {
        shell_trace ("%d: S: pmi finalized", task->rank);
    }
}

/* Query broker to see if instance mapping is known, then use that information
 * to select whether process mapping should be "none", "single", or "pershell".
 * If the instance mapping is unknown, use "pershell".  The choice of default
 * was a process of trial and error:
 *
 * PMI_process_mapping originated with MPICH, which uses it to determine
 * whether it can short circult the comms path between local ranks with shmem.
 * MPICH allows the key to be missing or its value to be empty, and in those
 * cases just skips the optimization.  Based on this, one might assume that
 * either "none" or "pershell" would be valid defaults when the mapping is
 * unknown.  However, note the following:
 *
 * - MVAPICH2 fails with an "Invalid tag" error in MPI_Init() if the key
 *   does not exist (flux-framework/flux-core#3592) and an even more obscure
 *   error if it exists but is empty
 *
 * - OpenMPI might select conflicting shmem names if the mapping indicates
 *   that ranks are not co-located when they really are
 *   (flux-framework/flux-core#3551)
 *
 * Least worse choice seems to be "pershell", and then use the openmpi shell
 * plugin to cast the proper runes to avoid OpenMPI name collisions by
 * redirecting shmem paths to FLUX_JOB_TMPDIR.  FLUX_JOB_TMPDIR includes
 * the jobid and the shell rank in its path, so works as a unique path
 * prefix even when there are multiple brokers/shells per node.
 */
static const char *guess_clique_option (struct shell_pmi *pmi)
{
    const char *val;
    struct pmi_map_block *blocks = NULL;
    int nblocks;
    const char *opt = "pershell";

    if (pmi->shell->standalone)
        goto done;
    if (!(val = flux_attr_get (pmi->shell->h, "broker.mapping")))
        goto done;
    if (pmi_process_mapping_parse (val, &blocks, &nblocks) < 0)
        goto done;
    if (nblocks == 1 && blocks[0].nodes == 1)       // one node
        opt = "single";
    else if (nblocks == 1 && blocks[0].procs == 1)  // one broker per node
        opt = "pershell";
done:
    free (blocks);
    return opt;
}

/* Generate 'PMI_process_mapping' key (see RFC 13) for MPI clique computation.
 */
static int init_clique (struct shell_pmi *pmi, const char *opt)
{
    struct pmi_map_block *blocks = NULL;
    int nblocks;
    int i;
    char val[SIMPLE_KVS_VAL_MAX];

    if (!opt)
        opt = guess_clique_option (pmi);

     /* pmi.clique=pershell (default): one clique per shell.
      * Create an array of pmi_map_block structures, sized for worst case
      * mapping (no compression possible).  Walk through the rcalc info for
      * each shell rank.  If shell's mapping looks identical to previous one,
      * increment block->nodes; otherwise consume another array slot.
      */
    if (!strcmp (opt, "pershell")) {
        if (!(blocks = calloc (pmi->shell->info->shell_size, sizeof (*blocks))))
            return -1;
        nblocks = 0;

        for (i = 0; i < pmi->shell->info->shell_size; i++) {
            struct rcalc_rankinfo ri;

            if (rcalc_get_nth (pmi->shell->info->rcalc, i, &ri) < 0)
                goto error;
            if (nblocks == 0 || blocks[nblocks - 1].procs != ri.ntasks) {
                blocks[nblocks].nodeid = i;
                blocks[nblocks].procs = ri.ntasks;
                blocks[nblocks].nodes = 1;
                nblocks++;
            }
            else
                blocks[nblocks - 1].nodes++;
        }
    }
    /* pmi.clique=single: all procs are on the same node.
     */
    else if (!strcmp (opt, "single")) {
        if (!(blocks = calloc (1, sizeof (*blocks))))
            return -1;
        nblocks = 1;
        blocks[0].nodeid = 0;
        blocks[0].procs = pmi->shell->info->total_ntasks;
        blocks[0].nodes = 1;
    }
    /* pmi.clique=none: disable PMI_process_mapping generation.
     */
    else if (!strcmp (opt, "none")) {
        goto out;
    }
    else {
        shell_log_error ("pmi.clique=%s is invalid", opt);
        goto error;
    }

    /* Encode to string, and store to local KVS hash.
     */

    /* If value exceeds SIMPLE_KVS_VAL_MAX, skip setting the key
     * without generating an error.  The client side will not treat
     * a missing key as an error.  It should be unusual though so log it.
     */
    if (pmi_process_mapping_encode (blocks, nblocks, val, sizeof (val)) < 0) {
        shell_log_errno ("pmi_process_mapping_encode");
        goto out;
    }
    put_dict (pmi->locals, "PMI_process_mapping", val);
out:
    free (blocks);
    return 0;
error:
    free (blocks);
    errno = EINVAL;
    return -1;
}

static int set_flux_instance_level (struct shell_pmi *pmi)
{
    char *p;
    long l;
    int n;
    int rc = -1;
    char val [SIMPLE_KVS_VAL_MAX];
    const char *level = flux_attr_get (pmi->shell->h, "instance-level");

    if (!level)
        return 0;

    errno = 0;
    l = strtol (level, &p, 10);
    if (errno != 0 || *p != '\0' || l < 0) {
        shell_log_error ("set_flux_instance_level level=%s invalid", level);
        goto out;
    }
    n = snprintf (val, sizeof (val), "%lu", l+1);
    if (n >= sizeof (val)) {
        shell_log_errno ("set_flux_instance_level: snprintf");
        goto out;
    }
    put_dict (pmi->locals, "flux.instance-level", val);
    rc = 0;
out:
    return rc;
}

static void pmi_destroy (struct shell_pmi *pmi)
{
    if (pmi) {
        int saved_errno = errno;
        pmi_simple_server_destroy (pmi->server);
        pmi_exchange_destroy (pmi->exchange);
        json_decref (pmi->global);
        json_decref (pmi->pending);
        json_decref (pmi->locals);
        free (pmi);
        errno = saved_errno;
    }
}

static struct pmi_simple_ops shell_pmi_ops = {
    .response_send  = shell_pmi_response_send,
    .debug_trace    = shell_pmi_debug_trace,
    .abort          = shell_pmi_abort,
};

static int parse_args (flux_shell_t *shell,
                       int *exchange_k,
                       const char **kvs,
                       const char **clique)
{
    if (flux_shell_getopt_unpack (shell,
                                  "pmi",
                                  "{s?s s?{s?i} s?s}",
                                  "kvs",
                                  kvs,
                                  "exchange",
                                    "k", exchange_k,
                                  "clique",
                                  clique) < 0)
        return -1;
    return 0;
}

static struct shell_pmi *pmi_create (flux_shell_t *shell)
{
    struct shell_pmi *pmi;
    struct shell_info *info = shell->info;
    int flags = shell->verbose ? PMI_SIMPLE_SERVER_TRACE : 0;
    char kvsname[32];
    const char *kvs = "exchange";
    int exchange_k = 0; // 0=use default tree fanout
    const char *clique = NULL;

    if (!(pmi = calloc (1, sizeof (*pmi))))
        return NULL;
    pmi->shell = shell;

    if (parse_args (shell, &exchange_k, &kvs, &clique) < 0)
        goto error;
    if (!strcmp (kvs, "native")) {
        shell_pmi_ops.kvs_put = native_kvs_put;
        shell_pmi_ops.kvs_get = native_kvs_get;
        shell_pmi_ops.barrier_enter = native_barrier_enter;
        if (shell->info->shell_rank == 0)
            shell_warn ("using native Flux kvs implementation");
    }
    else if (!strcmp (kvs, "exchange")) {
        shell_pmi_ops.kvs_put = exchange_kvs_put;
        shell_pmi_ops.kvs_get = exchange_kvs_get;
        shell_pmi_ops.barrier_enter = exchange_barrier_enter;
        if (!(pmi->exchange = pmi_exchange_create (shell, exchange_k)))
            goto error;
    }
    else {
        shell_log_error ("Unknown kvs implementation %s", kvs);
        errno = EINVAL;
        goto error;
    }

    /* Use F58 representation of jobid for "kvsname", since the broker
     * will pull the kvsname and use it as the broker 'jobid' attribute.
     * This allows the broker attribute to be in the "common" user-facing
     * jobid representation.
     */
    if (flux_job_id_encode (shell->jobid,
                            "f58",
                            kvsname,
                            sizeof (kvsname)) < 0)
        goto error;
    if (!(pmi->server = pmi_simple_server_create (shell_pmi_ops,
                                                  0, // appnum
                                                  info->total_ntasks,
                                                  info->rankinfo.ntasks,
                                                  kvsname,
                                                  flags,
                                                  pmi)))
        goto error;
    if (!(pmi->global = json_object ())
        || !(pmi->pending = json_object ())
        || !(pmi->locals = json_object ())) {
        errno = ENOMEM;
        goto error;
    }
    if (init_clique (pmi, clique) < 0)
        goto error;
    if (!shell->standalone) {
        if (set_flux_instance_level (pmi) < 0)
            goto error;
    }
    return pmi;
error:
    pmi_destroy (pmi);
    return NULL;
}

static int shell_pmi_init (flux_plugin_t *p,
                           const char *topic,
                           flux_plugin_arg_t *arg,
                           void *data)
{
    flux_shell_t *shell = flux_plugin_get_shell (p);
    struct shell_pmi *pmi;
    if (!shell || !(pmi = pmi_create (shell)))
        return -1;
    if (flux_plugin_aux_set (p, "pmi", pmi, (flux_free_f) pmi_destroy) < 0) {
        pmi_destroy (pmi);
        return -1;
    }
    return 0;
}

static int shell_pmi_task_init (flux_plugin_t *p,
                                const char *topic,
                                flux_plugin_arg_t *args,
                                void *arg)
{
    flux_shell_t *shell;
    struct shell_pmi *pmi;
    flux_shell_task_t *task;
    flux_cmd_t *cmd;

    if (!(shell = flux_plugin_get_shell (p))
        || !(pmi = flux_plugin_aux_get (p, "pmi"))
        || !(task = flux_shell_current_task (shell))
        || !(cmd = flux_shell_task_cmd (task)))
        return -1;

    if (flux_cmd_add_channel (cmd, "PMI_FD") < 0)
        return -1;
    if (flux_cmd_setenvf (cmd, 1, "PMI_RANK", "%d", task->rank) < 0)
        return -1;
    if (flux_cmd_setenvf (cmd, 1, "PMI_SIZE", "%d", task->size) < 0)
        return -1;
    if (flux_shell_task_channel_subscribe (task, "PMI_FD", pmi_fd_cb, pmi) < 0)
        return -1;
    return 0;
}

struct shell_builtin builtin_pmi = {
    .name = "pmi",
    .init = shell_pmi_init,
    .task_init = shell_pmi_task_init,
};

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
