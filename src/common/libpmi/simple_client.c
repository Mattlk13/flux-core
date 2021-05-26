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
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <assert.h>

#include "src/common/libutil/aux.h"

#include "simple_client.h"
#include "simple_server.h"
#include "clique.h"
#include "dgetline.h"
#include "keyval.h"
#include "pmi.h"

int pmi_simple_client_init (struct pmi_simple_client *pmi)
{
    int result = PMI_FAIL;
    unsigned int vers, subvers;
    char buf[SIMPLE_MAX_PROTO_LINE];
    int rc;

    if (!pmi)
        return PMI_ERR_INIT;
    if (dprintf (pmi->fd, "cmd=init pmi_version=1 pmi_subversion=1\n") < 0)
        goto done;
    if (dgetline (pmi->fd, buf, sizeof (buf)) < 0)
        goto done;
    if (keyval_parse_isword (buf, "cmd", "response_to_init") < 0)
        goto done;
    if (keyval_parse_int (buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    if (keyval_parse_uint (buf, "pmi_version", &vers) < 0
            || keyval_parse_uint (buf, "pmi_subversion", &subvers) < 0)
        goto done;
    if (vers != 1 || subvers != 1)
        goto done;

    if (dprintf (pmi->fd, "cmd=get_maxes\n") < 0)
        goto done;
    if (dgetline (pmi->fd, buf, sizeof (buf)) < 0)
        goto done;
    if (keyval_parse_isword (buf, "cmd", "maxes") < 0)
        goto done;
    if (keyval_parse_int (buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    if (keyval_parse_uint (buf, "kvsname_max", &pmi->kvsname_max) < 0
            || keyval_parse_uint (buf, "keylen_max", &pmi->keylen_max) < 0
            || keyval_parse_uint (buf, "vallen_max", &pmi->vallen_max) < 0)
        goto done;
    pmi->buflen = pmi->keylen_max + pmi->vallen_max + pmi->kvsname_max
                                  + SIMPLE_MAX_PROTO_OVERHEAD;
    if (!(pmi->buf = calloc (1, pmi->buflen))) {
        result = PMI_ERR_NOMEM;
        goto done;
    }
    pmi->initialized = 1;
    result = PMI_SUCCESS;
done:
    return result;
}

int pmi_simple_client_finalize (struct pmi_simple_client *pmi)
{
    int result = PMI_FAIL;
    int rc;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (dprintf (pmi->fd, "cmd=finalize\n") < 0)
        goto done;
    if (dgetline (pmi->fd, pmi->buf, pmi->buflen) < 0)
        goto done;
    if (keyval_parse_isword (pmi->buf, "cmd", "finalize_ack") < 0)
        goto done;
    if (keyval_parse_int (pmi->buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    result = PMI_SUCCESS;
done:
    return result;
}

int pmi_simple_client_get_appnum (struct pmi_simple_client *pmi, int *appnum)
{
    int result = PMI_FAIL;
    int rc;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (!appnum)
        return PMI_ERR_INVALID_ARG;
    if (dprintf (pmi->fd, "cmd=get_appnum\n") < 0)
        goto done;
    if (dgetline (pmi->fd, pmi->buf, pmi->buflen) < 0)
        goto done;
    if (keyval_parse_isword (pmi->buf, "cmd", "appnum") < 0)
        goto done;
    if (keyval_parse_int (pmi->buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    if (keyval_parse_int (pmi->buf, "appnum", appnum) < 0)
        goto done;
    result = PMI_SUCCESS;
done:
    return result;
}

int pmi_simple_client_get_universe_size (struct pmi_simple_client *pmi,
                                         int *universe_size)
{
    int result = PMI_FAIL;
    int rc;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (!universe_size)
        return PMI_ERR_INVALID_ARG;
    if (dprintf (pmi->fd, "cmd=get_universe_size\n") < 0)
        goto done;
    if (dgetline (pmi->fd, pmi->buf, pmi->buflen) < 0)
        goto done;
    if (keyval_parse_isword (pmi->buf, "cmd", "universe_size") < 0)
        goto done;
    if (keyval_parse_int (pmi->buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    if (keyval_parse_int (pmi->buf, "size", universe_size) < 0)
        goto done;
    result = PMI_SUCCESS;
done:
    return result;
}

int pmi_simple_client_barrier (struct pmi_simple_client *pmi)
{
    int result = PMI_FAIL;
    int rc;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (dprintf (pmi->fd, "cmd=barrier_in\n") < 0)
        goto done;
    if (dgetline (pmi->fd, pmi->buf, pmi->buflen) < 0)
        goto done;
    if (keyval_parse_isword (pmi->buf, "cmd", "barrier_out") < 0)
        goto done;
    if (keyval_parse_int (pmi->buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    result = PMI_SUCCESS;
done:
    return result;
}

int pmi_simple_client_kvs_get_my_name (struct pmi_simple_client *pmi,
                                       char *kvsname,
                                       int length)
{
    int result = PMI_FAIL;
    int rc;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (!kvsname || length <= 0)
        return PMI_ERR_INVALID_ARG;
    if (dprintf (pmi->fd, "cmd=get_my_kvsname\n") < 0)
        goto done;
    if (dgetline (pmi->fd, pmi->buf, pmi->buflen) < 0)
        goto done;
    if (keyval_parse_isword (pmi->buf, "cmd", "my_kvsname") < 0)
        goto done;
    if (keyval_parse_int (pmi->buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    if (keyval_parse_word (pmi->buf, "kvsname", kvsname, length) < 0)
        goto done;
    result = PMI_SUCCESS;
done:
    return result;
}

int pmi_simple_client_kvs_put (struct pmi_simple_client *pmi,
                               const char *kvsname,
                               const char *key,
                               const char *value)
{
    int result = PMI_FAIL;
    int rc;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (!kvsname || !key || !value)
        return PMI_ERR_INVALID_ARG;
    if (dprintf (pmi->fd, "cmd=put kvsname=%s key=%s value=%s\n",
                 kvsname, key, value) < 0)
        goto done;
    if (dgetline (pmi->fd, pmi->buf, pmi->buflen) < 0)
        goto done;
    if (keyval_parse_isword (pmi->buf, "cmd", "put_result") < 0)
        goto done;
    if (keyval_parse_int (pmi->buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    result = PMI_SUCCESS;
done:
    return result;
}

int pmi_simple_client_kvs_get (struct pmi_simple_client *pmi,
                               const char *kvsname,
                               const char *key,
                               char *value,
                               int len)
{
    int result = PMI_FAIL;
    int rc;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (!kvsname || !key || !value || len <= 0)
        return PMI_ERR_INVALID_ARG;
    if (dprintf (pmi->fd, "cmd=get kvsname=%s key=%s\n", kvsname, key) < 0)
        goto done;
    if (dgetline (pmi->fd, pmi->buf, pmi->buflen) < 0)
        goto done;
    if (keyval_parse_isword (pmi->buf, "cmd", "get_result") < 0)
        goto done;
    if (keyval_parse_int (pmi->buf, "rc", &rc) == 0 && rc != 0) {
        result = rc;
        goto done;
    }
    if (keyval_parse_string (pmi->buf, "value", value, len) < 0)
        goto done;
    result = PMI_SUCCESS;
done:
    return result;
}

/* Helper for get_clique_size(), get_clique_ranks().
 * Fetch 'PMI_process_mapping' from the KVS and parse.
 * On success, results are placed in blocks, nblocks, nodeid.
 * The caller must free 'blocks'.
 */
static int fetch_process_mapping (struct pmi_simple_client *pmi,
                                  struct pmi_map_block **blocks,
                                  int *nblocks,
                                  int *nodeid)
{
    const char *key = "PMI_process_mapping";
    int result;
    char *nom;
    char *val;

    assert (pmi != NULL);
    assert (pmi->initialized);

    nom = calloc (1, pmi->kvsname_max);
    val = calloc (1, pmi->vallen_max);
    if (!nom || !val) {
        result = PMI_ERR_NOMEM;
        goto done;
    }
    result = pmi_simple_client_kvs_get_my_name (pmi, nom, pmi->kvsname_max);
    if (result != PMI_SUCCESS)
        goto done;
    result = pmi_simple_client_kvs_get (pmi, nom, key, val, pmi->vallen_max);
    if (result != PMI_SUCCESS)
        goto done;
    result = pmi_process_mapping_parse (val, blocks, nblocks);
    if (result != PMI_SUCCESS)
        goto done;
    if (pmi_process_mapping_find_nodeid (*blocks, *nblocks,
                                         pmi->rank, nodeid) != PMI_SUCCESS)
        *nodeid = -1;
done:
    free (nom);
    free (val);
    return result;
}

int pmi_simple_client_get_clique_size (struct pmi_simple_client *pmi,
                                       int *size)
{
    int result;
    struct pmi_map_block *blocks = NULL;
    int nblocks;
    int nodeid;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (!size)
        return PMI_ERR_INVALID_ARG;
    result = fetch_process_mapping (pmi, &blocks, &nblocks, &nodeid);
    if (result != PMI_SUCCESS || nodeid == -1) {
        *size = 1;
        result = PMI_SUCCESS;
    }
    else
        result = pmi_process_mapping_find_nranks (blocks, nblocks, nodeid,
                                                  pmi->size, size);
    free (blocks);
    return result;
}

int pmi_simple_client_get_clique_ranks (struct pmi_simple_client *pmi,
                                        int ranks[],
                                        int length)
{
    int result;
    struct pmi_map_block *blocks = NULL;
    int nblocks;
    int nodeid;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (!ranks)
        return PMI_ERR_INVALID_ARG;
    result = fetch_process_mapping (pmi, &blocks, &nblocks, &nodeid);
    if (result != PMI_SUCCESS || nodeid == -1) {
        if (length != 1)
            return PMI_ERR_INVALID_SIZE;
        *ranks = pmi->rank;
        result = PMI_SUCCESS;
    }
    else
        result = pmi_process_mapping_find_ranks (blocks, nblocks, nodeid,
                                                 pmi->size, ranks, length);
    free (blocks);
    return result;
}

int pmi_simple_client_abort (struct pmi_simple_client *pmi,
                             int exit_code,
                             const char *msg)
{
    int result = PMI_FAIL;

    if (!pmi || !pmi->initialized)
        return PMI_ERR_INIT;
    if (exit_code < 0)
        return PMI_ERR_INVALID_ARG;
    if (dprintf (pmi->fd,
                 "cmd=abort exitcode=%d%s%s\n",
                 exit_code,
                 msg ? " error_msg=" : "",
                 msg ? msg : "") < 0)
        goto done;
    exit (exit_code);
    /* NOTREACHED */
done:
    return result;
}

void *pmi_simple_client_aux_get (struct pmi_simple_client *pmi,
                                 const char *name)
{
    if (!pmi) {
        errno = EINVAL;
        return NULL;
    }
    return aux_get (pmi->aux, name);
}

int pmi_simple_client_aux_set (struct pmi_simple_client *pmi,
                               const char *name,
                               void *aux,
                               flux_free_f destroy)
{
    if (!pmi) {
        errno = EINVAL;
        return -1;
    }
    return aux_set (&pmi->aux, name, aux, destroy);
}

void pmi_simple_client_destroy (struct pmi_simple_client *pmi)
{
    if (pmi) {
        int saved_errno = errno;
        aux_destroy (&pmi->aux);
        if (pmi->fd != -1)
            (void)close (pmi->fd);
        free (pmi->buf);
        free (pmi);
        errno = saved_errno;
    }
}

struct pmi_simple_client *pmi_simple_client_create_fd (const char *pmi_fd,
                                                       const char *pmi_rank,
                                                       const char *pmi_size,
                                                       const char *pmi_spawned)
{
    struct pmi_simple_client *pmi;

    if (!pmi_fd || !pmi_rank || !pmi_size) {
        errno = EINVAL;
        return NULL;
    }
    if (!(pmi = calloc (1, sizeof (*pmi))))
        return NULL;
    errno = 0;
    pmi->fd = strtol (pmi_fd, NULL, 10);
    pmi->rank = strtol (pmi_rank, NULL, 10);
    pmi->size = strtol (pmi_size, NULL, 10);
    if (errno != 0 || pmi->fd < 0 || pmi->rank < 0 || pmi->size < 1)
        goto error;
    if (pmi_spawned) {
        errno = 0;
        pmi->spawned = strtol (pmi_spawned, NULL, 10);
        if (errno != 0)
            goto error;
    }
    return pmi;
error:
    pmi_simple_client_destroy (pmi);
    return NULL;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
