/************************************************************\
 * Copyright 2023 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

/* rexec.c - shell subprocess server
 */

#define FLUX_SHELL_PLUGIN_NAME "rexec"

#if HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <flux/core.h>
#include <jansson.h>

#include "src/common/libsubprocess/server.h"
#include "src/common/libutil/errprintf.h"

#include "builtins.h"
#include "internal.h"
#include "svc.h"
#include "log.h"

struct shell_rexec {
    flux_shell_t *shell;
    subprocess_server_t *server;
    char *name;
    bool parent_is_trusted;
};

static void rexec_destroy (struct shell_rexec *rexec)
{
    if (rexec) {
        int saved_errno = errno;
        subprocess_server_destroy (rexec->server);
        free (rexec->name);
        free (rexec);
        errno = saved_errno;
    }
}

/* The embedded subprocess server restricts access based on FLUX_ROLE_OWNER,
 * but this shell cannot trust message credentials if they are passing through
 * a Flux instance running as a different user (e.g. the "flux" user in a
 * system instance).  If that user were compromised, they could run arbitrary
 * commands as any user that currently has a job running.  Therefore, this
 * additional check ensures that we only trust an instance running as the same
 * user.
 *
 * For good measure, check that the shell userid matches the credential
 * userid. After the above check, this could only fail in test where the
 * owner can be mocked.
 */
static int rexec_auth_cb (const flux_msg_t *msg,
                          void *arg,
                          flux_error_t *errp)
{
    struct shell_rexec *rexec = arg;
    uint32_t userid;

    if (!rexec->parent_is_trusted
        || flux_msg_get_userid (msg, &userid) < 0
        || userid != getuid ()) {
        errno = EPERM;
        return errprintf (errp, "Access denied");
    }
    return 0;
}

static struct shell_rexec *rexec_create (flux_shell_t *shell)
{
    struct shell_rexec *rexec;

    if (!(rexec = calloc (1, sizeof (*rexec))))
        return NULL;
    rexec->shell = shell;

    /* Determine if this shell is running as the instance owner, without
     * trusting the instance owner to tell us.  Since the parent of a guest
     * shell is flux-imp(1), kill(2) of the parent pid should fail for guests.
     */
    pid_t ppid = getppid (); // 0 =  parent is in a different pid namespace
    if (ppid > 0 && kill (getppid (), 0) == 0)
        rexec->parent_is_trusted = true;

    /* N.B. subprocess_server_create() registers the methods: exec, write,
     * kill, list, and disconnect.  Give the server its own namespace.  The
     * full topic strings will be like "5588-shell-381933322240.rexec.kill".
     */
    if (asprintf (&rexec->name, "%s.rexec", shell_svc_name (shell->svc)) < 0)
        goto error;
    if (!(rexec->server = subprocess_server_create (flux_shell_get_flux (shell),
                                                    rexec->name,
                                                    getenv ("FLUX_URI"),
                                                    shell_llog,
                                                    NULL)))
        goto error;
    subprocess_server_set_auth_cb (rexec->server,
                                   rexec_auth_cb,
                                   rexec);
    shell_debug ("registered rexec service as %s", rexec->name);
    return rexec;
error:
    rexec_destroy (rexec);
    return NULL;
}

static int rexec_init (flux_plugin_t *p,
                      const char *topic,
                      flux_plugin_arg_t *arg,
                      void *data)
{
    flux_shell_t *shell = flux_plugin_get_shell (p);
    struct shell_rexec *rexec;

    if (!(rexec = rexec_create (shell)))
        return -1;
    if (flux_plugin_aux_set (p,
                             "rexec",
                             rexec,
                             (flux_free_f)rexec_destroy) < 0) {
        rexec_destroy (rexec);
        return -1;
    }
    return 0;
}

struct shell_builtin builtin_rexec = {
    .name = FLUX_SHELL_PLUGIN_NAME,
    .init = rexec_init,
};

// vi:ts=4 sw=4 expandtab
