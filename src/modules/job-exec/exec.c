/************************************************************\
 * Copyright 2019 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

/* Flux subprocess-based exec implementation
 *
 * DESCRIPTION
 *
 * Launch configured job shell, one per rank.
 *
 * TEST CONFIGURATION
 *
 * Test and other configuration may be presented in the jobspec
 * attributes.system.exec.bulkexec object. Supported keys include
 *
 * {
 *    "mock_exception":s       - Generate a mock execption in phase:
 *                               "init", or "starting"
 * }
 *
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <unistd.h>

#include "job-exec.h"
#include "bulk-exec.h"
#include "rset.h"

extern char **environ;
static const char *default_cwd = "/tmp";
static const char *default_job_shell = NULL;
static const char *flux_imp_path = NULL;

/* Configuration for "bulk" execution implementation. Used only for testing
 *  for now.
 */
struct exec_conf {
    const char *        mock_exception;   /* fake exception */
};

static void exec_conf_destroy (struct exec_conf *tc)
{
    free (tc);
}

static struct exec_conf *exec_conf_create (json_t *jobspec)
{
    struct exec_conf *conf = calloc (1, sizeof (*conf));
    if (conf == NULL)
        return NULL;
    (void) json_unpack (jobspec, "{s:{s:{s:{s:{s:s}}}}}",
                                 "attributes", "system", "exec",
                                     "bulkexec",
                                         "mock_exception",
                                         &conf->mock_exception);
    return conf;
}

static const char * exec_mock_exception (struct bulk_exec *exec)
{
    struct exec_conf *conf = bulk_exec_aux_get (exec, "conf");
    if (!conf || !conf->mock_exception)
        return "none";
    return conf->mock_exception;
}

static const char *jobspec_get_job_shell (json_t *jobspec)
{
    const char *path = NULL;
    (void) json_unpack (jobspec, "{s:{s:{s:{s:s}}}}",
                                 "attributes", "system", "exec",
                                 "job_shell", &path);
    return path;
}

static const char *job_shell_path (struct jobinfo *job)
{
    const char *path = jobspec_get_job_shell (job->jobspec);
    return path ? path : default_job_shell;
}

static const char *jobspec_get_cwd (json_t *jobspec)
{
    const char *cwd = NULL;
    (void) json_unpack (jobspec, "{s:{s:{s:s}}}",
                                 "attributes", "system",
                                 "cwd", &cwd);
    return cwd;
}

static const char *job_get_cwd (struct jobinfo *job)
{
    const char *cwd;
    if (job->multiuser)
        cwd = "/";
    else if (!(cwd = jobspec_get_cwd (job->jobspec)))
        cwd = default_cwd;
    return (cwd);
}

static void start_cb (struct bulk_exec *exec, void *arg)
{
    struct jobinfo *job = arg;
    jobinfo_started (job, NULL);
    /*  This is going to be really slow. However, it should at least
     *   work for now. We wait for all imp's to start, then send input
     */
    if (job->multiuser) {
        char *input = NULL;
        json_t *o = json_pack ("{s:s}", "J", job->J);
        if (!o || !(input = json_dumps (o, JSON_COMPACT))) {
            jobinfo_fatal_error (job, errno, "Failed to get input to IMP");
            goto out;
        }
        if (bulk_exec_write (exec, "stdin", input, strlen (input)) < 0)
            jobinfo_fatal_error (job,
                                 errno,
                                 "Failed to write %ld bytes input to IMP",
                                 strlen (input));
        (void) bulk_exec_close (exec, "stdin");
out:
        json_decref (o);
        free (input);
    }

}

static void complete_cb (struct bulk_exec *exec, void *arg)
{
    struct jobinfo *job = arg;
    jobinfo_tasks_complete (job,
                            resource_set_ranks (job->R),
                            bulk_exec_rc (exec));
}

static void output_cb (struct bulk_exec *exec, flux_subprocess_t *p,
                       const char *stream,
                       const char *data,
                       int len,
                       void *arg)
{
    struct jobinfo *job = arg;
    const char *cmd = flux_cmd_arg (flux_subprocess_get_cmd (p), 0);
    jobinfo_log_output (job,
                        flux_subprocess_rank (p),
                        basename (cmd),
                        stream,
                        data,
                        len);
    flux_log (job->h, LOG_INFO, "%ju: %d: %s: %s",
                      (uintmax_t) job->id,
                      flux_subprocess_rank (p),
                      stream, data);
}

static void error_cb (struct bulk_exec *exec, flux_subprocess_t *p, void *arg)
{
    struct jobinfo *job = arg;
    const char *arg0 = flux_cmd_arg (flux_subprocess_get_cmd (p), 0);
    jobinfo_fatal_error (job, flux_subprocess_fail_errno (p),
                              "cmd=%s: rank=%d failed",
                              arg0, flux_subprocess_rank (p));
}

static struct bulk_exec_ops exec_ops = {
    .on_start =     start_cb,
    .on_exit =      NULL,
    .on_complete =  complete_cb,
    .on_output =    output_cb,
    .on_error =     error_cb
};

static int exec_init (struct jobinfo *job)
{
    flux_cmd_t *cmd = NULL;
    struct exec_conf *conf = NULL;
    struct bulk_exec *exec = NULL;
    const struct idset *ranks = NULL;

    if (job->multiuser && !flux_imp_path) {
        flux_log (job->h,
                  LOG_ERR,
                  "unable run multiuser job with no IMP configured!");
        goto err;
    }

    if (!(ranks = resource_set_ranks (job->R))) {
        flux_log_error (job->h, "exec_init: resource_set_ranks");
        goto err;
    }
    if (!(exec = bulk_exec_create (&exec_ops, job))) {
        flux_log_error (job->h, "exec_init: bulk_exec_create");
        goto err;
    }
    if (!(conf = exec_conf_create (job->jobspec))) {
        flux_log_error (job->h, "exec_init: exec_conf_create");
        goto err;
    }
    if (bulk_exec_aux_set (exec, "conf", conf,
                          (flux_free_f) exec_conf_destroy) < 0) {
        flux_log_error (job->h, "exec_init: bulk_exec_aux_set");
        goto err;
    }
    if (!(cmd = flux_cmd_create (0, NULL, environ))) {
        flux_log_error (job->h, "exec_init: flux_cmd_create");
        goto err;
    }
    if (flux_cmd_setenvf (cmd, 1, "FLUX_KVS_NAMESPACE", "%s", job->ns) < 0) {
        flux_log_error (job->h, "exec_init: flux_cmd_setenvf");
        goto err;
    }
    if (job->multiuser) {
        if (flux_cmd_argv_append (cmd, flux_imp_path) < 0
            || flux_cmd_argv_append (cmd, "exec") < 0) {
            flux_log_error (job->h, "exec_init: flux_cmd_argv_append");
            goto err;
        }
    }
    if (flux_cmd_argv_append (cmd, job_shell_path (job)) < 0
        || flux_cmd_argv_appendf (cmd, "%ju", (uintmax_t) job->id) < 0) {
        flux_log_error (job->h, "exec_init: flux_cmd_argv_append");
        goto err;
    }
    if (flux_cmd_setcwd (cmd, job_get_cwd (job)) < 0) {
        flux_log_error (job->h, "exec_init: flux_cmd_setcwd");
        goto err;
    }
    if (bulk_exec_push_cmd (exec, ranks, cmd, 0) < 0) {
        flux_log_error (job->h, "exec_init: bulk_exec_push_cmd");
        goto err;
    }
    flux_cmd_destroy (cmd);
    job->data = exec;
    return 1;
err:
    flux_cmd_destroy (cmd);
    bulk_exec_destroy (exec);
    return -1;
}

static void exec_check_cb (flux_reactor_t *r, flux_watcher_t *w,
                           int revents, void *arg)
{
    struct jobinfo *job = arg;
    struct bulk_exec *exec = job->data;
    if (bulk_exec_current (exec) >= 1) {
        jobinfo_fatal_error (job, 0, "mock starting exception generated");
        flux_log (job->h,
                  LOG_DEBUG,
                  "mock exception for starting job total=%d, current=%d",
                  bulk_exec_total (exec),
                  bulk_exec_current (exec));
        flux_watcher_destroy (w);
    }
}

static int exec_start (struct jobinfo *job)
{
    struct bulk_exec *exec = job->data;

    if (strcmp (exec_mock_exception (exec), "init") == 0) {
        /* If creating an "init" mock exception, generate it and
         *  then return to simulate an exception that came in before
         *  we could actually start the job
         */
        jobinfo_fatal_error (job, 0, "mock init exception generated");
        return 0;
    }
    else if (strcmp (exec_mock_exception (exec), "starting") == 0) {
        /*  If we're going to mock an exception in "starting" phase, then
         *   set up a check watcher to cancel the job when some shells have
         *   started but (potentially) not all.
         */
        flux_reactor_t *r = flux_get_reactor (job->h);
        flux_watcher_t *w = flux_check_watcher_create (r, exec_check_cb, job);
        if (w)
            flux_watcher_start (w);
    }

    return bulk_exec_start (job->h, exec);
}

static void exec_kill_cb (flux_future_t *f, void *arg)
{
    struct jobinfo *job = arg;
    if (flux_future_get (f, NULL) < 0 && errno != ENOENT)
        flux_log_error (job->h, "%ju: exec_kill", (uintmax_t) job->id);
    jobinfo_decref (job);
    flux_future_destroy (f);
}

static int exec_kill (struct jobinfo *job, int signum)
{
    struct  bulk_exec *exec = job->data;
    flux_future_t *f;

    if (job->multiuser)
        f = bulk_exec_imp_kill (exec, flux_imp_path, signum);
    else
        f = bulk_exec_kill (exec, signum);
    if (!f) {
        if (errno != ENOENT)
            flux_log_error (job->h, "%ju: bulk_exec_kill", job->id);
        return 0;
    }

    flux_log (job->h, LOG_DEBUG,
              "exec_kill: %ju: signal %d",
              (uintmax_t) job->id,
              signum);

    jobinfo_incref (job);
    if (flux_future_then (f, 3., exec_kill_cb, job) < 0) {
        flux_log_error (job->h, "%ju: exec_kill: flux_future_then", job->id);
        flux_future_destroy (f);
        return -1;
    }
    return 0;
}

static int exec_cancel (struct jobinfo *job)
{
    struct bulk_exec *exec = job->data;
    return bulk_exec_cancel (exec);
}

static void exec_exit (struct jobinfo *job)
{
    struct bulk_exec *exec = job->data;
    bulk_exec_destroy (exec);
    job->data = NULL;
}

/*  Configure the exec module.
 *  Read the default job shell path from config. Allow override on cmdline
 */
static int exec_config (flux_t *h, int argc, char **argv)
{
    flux_conf_error_t err;

    /*  Set default job shell path from builtin configuration,
     *   allow override via configuration, then cmdline.
     */
    default_job_shell = flux_conf_builtin_get ("shell_path", FLUX_CONF_AUTO);


    /*  Check configuration for exec.job-shell */
    if (flux_conf_unpack (flux_get_conf (h),
                          &err,
                          "{s?:{s?s}}",
                          "exec",
                            "job-shell", &default_job_shell) < 0) {
        flux_log (h, LOG_ERR,
                  "error reading config value exec.job-shell: %s",
                  err.errbuf);
        return -1;
    }

    /*  Check configuration for exec.imp */
    if (flux_conf_unpack (flux_get_conf (h),
                          &err,
                          "{s?:{s?s}}",
                          "exec",
                            "imp", &flux_imp_path) < 0) {
        flux_log (h, LOG_ERR,
                  "error reading config value exec.imp: %s",
                  err.errbuf);
        return -1;
    }

    /* Finally, override values on cmdline */
    for (int i = 0; i < argc; i++) {
        if (strncmp (argv[i], "job-shell=", 10) == 0)
            default_job_shell = argv[i]+10;
        else if (strncmp (argv[i], "imp=", 4) == 0)
            flux_imp_path = argv[i]+4;
    }
    flux_log (h, LOG_DEBUG, "using default shell path %s", default_job_shell);
    if (flux_imp_path)
        flux_log (h, LOG_DEBUG, "using imp path %s", flux_imp_path);
    return 0;
}

struct exec_implementation bulkexec = {
    .name =     "bulk-exec",
    .config =   exec_config,
    .init =     exec_init,
    .exit =     exec_exit,
    .start =    exec_start,
    .kill =     exec_kill,
    .cancel =   exec_cancel,
};

/* vi: ts=4 sw=4 expandtab
 */
