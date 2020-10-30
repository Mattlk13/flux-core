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
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <czmq.h>
#include <jansson.h>

#include "src/common/libidset/idset.h"
#include "src/common/libhostlist/hostlist.h"
#include "rnode.h"
#include "rlist.h"
#include "rhwloc.h"

static int by_rank (const void *item1, const void *item2);

static int
sprintfcat (char **s, size_t *sz, size_t *lenp, const char *fmt, ...)
{
    int done = false;
    va_list ap;
    int n = 0;
    while (!done) {
        int nleft = *sz-*lenp;
        va_start (ap, fmt);
        n = vsnprintf ((*s)+*lenp, nleft, fmt, ap);
        if (n < 0 || n >= nleft) {
            char *p;
            *sz += 128;
            if (!(p = realloc (*s, *sz)))
                return -1;
            *s = p;
        }
        else
            done = true;
        va_end (ap);
    }
    *lenp += n;
    return (n);
}

void rlist_destroy (struct rlist *rl)
{
    if (rl) {
        int saved_errno = errno;
        zlistx_destroy (&rl->nodes);
        zhashx_destroy (&rl->noremap);
        json_decref (rl->scheduling);
        free (rl);
        errno = saved_errno;
    }
}

static void rn_free_fn (void **x)
{
    rnode_destroy (*(struct rnode **)x);
    *x = NULL;
}

static void valfree (void **item)
{
    if (item) {
        free (*item);
        *item = NULL;
    }
}

struct rlist *rlist_create (void)
{
    struct rlist *rl = calloc (1, sizeof (*rl));
    if (!(rl->nodes = zlistx_new ()))
        goto err;
    zlistx_set_destructor (rl->nodes, rn_free_fn);

    if (!(rl->noremap = zhashx_new ()))
        goto err;
    zhashx_set_destructor (rl->noremap, valfree);
    zhashx_set_duplicator (rl->noremap, (zhashx_duplicator_fn *) strdup);
    zhashx_insert (rl->noremap, "gpu", "gpu");
    return (rl);
err:
    rlist_destroy (rl);
    return (NULL);
}

/*  Append two scheduling JSON objects s1 and s2.
 *
 *  These objects are supposed to be opaque, so for now we punt on
 *   doing an actual merge and just return s1 if non-NULL or s2 if non-NULL.
 *
 *  In the future, perhaps a "deep merge" could be done here instead, though
 *   the actual implementation of the scheduling key, JGF, do not make this
 *   easy since the main components are two lists of nodes and edges.
 */
static json_t * scheduling_key_append (json_t *s1, json_t *s2)
{
    if (s1)
        return json_incref (s1);
    else if (s1 == NULL && s2)
        return json_incref (s2);
    else
        return NULL;
}

static struct rnode *rlist_find_rank (const struct rlist *rl, uint32_t rank)
{
    struct rnode *n = zlistx_first (rl->nodes);
    while (n) {
        if (n->rank == rank)
            return (n);
        n = zlistx_next (rl->nodes);
    }
    return NULL;
}

static void rlist_update_totals (struct rlist *rl, struct rnode *n)
{
    rl->total += rnode_count (n);
    if (n->up)
        rl->avail += rnode_avail (n);
}

static int rlist_add_rnode_new (struct rlist *rl, struct rnode *n)
{
    if (!zlistx_add_end (rl->nodes, n))
        return -1;
    rlist_update_totals (rl, n);
    return 0;
}

/*  Add rnode 'n' to the rlist 'rl'. The memory for rnode n is stolen
 *   by this function (either the rnode is consumed by the rlist, or
 *   the rnode is destroyed after its resources are applied to 'rl')
 */
static int rlist_add_rnode (struct rlist *rl, struct rnode *n)
{
    struct rnode *found = rlist_find_rank (rl, n->rank);
    if (found) {
        if (rnode_add (found, n) < 0)
            return -1;
        rlist_update_totals (rl, n);
        rnode_destroy (n);
    }
    else if (rlist_add_rnode_new (rl, n) < 0)
        return -1;
    return 0;
}

typedef struct rnode * (*rnode_copy_f) (const struct rnode *);

static struct rlist *rlist_copy_internal (const struct rlist *orig,
                                          rnode_copy_f cpfn)
{
    struct rnode *n;
    struct rlist *rl = rlist_create ();
    if (!rl)
        return NULL;

    n = zlistx_first (orig->nodes);
    while (n) {
        struct rnode *copy = (*cpfn) (n);
        if (copy && rlist_add_rnode_new (rl, copy) < 0) {
            rnode_destroy (copy);
            goto fail;
        }
        n = zlistx_next (orig->nodes);
    }

    /*  Copy entire opaque scheduling key unless rlist is empty
     */
    if (rlist_nnodes (rl) > 0)
        rl->scheduling = scheduling_key_append (orig->scheduling, NULL);


    /*  Copy noremap hash from orignal rlist
     */
    zhashx_destroy (&rl->noremap);
    rl->noremap = zhashx_dup (orig->noremap);
    if (!rl->noremap)
        return NULL;

    return rl;
fail:
    rlist_destroy (rl);
    return NULL;
}

struct rlist *rlist_copy_empty (const struct rlist *orig)
{
    return rlist_copy_internal (orig, rnode_copy_empty);
}

struct rlist *rlist_copy_allocated (const struct rlist *orig)
{
    return rlist_copy_internal (orig, rnode_copy_alloc);
}

struct rlist *rlist_copy_down (const struct rlist *orig)
{
    struct rnode *n;
    struct rlist *rl = rlist_create ();
    if (!rl)
        return NULL;
    n = zlistx_first (orig->nodes);
    while (n) {
        if (!n->up) {
            struct rnode *copy = rnode_copy_empty (n);
            if (!copy || rlist_add_rnode_new (rl, copy) < 0)
                goto fail;
        }
        n = zlistx_next (orig->nodes);
    }

    /*  Copy entire opaque scheduling key unless rlist is empty
     */
    if (rlist_nnodes (rl) > 0)
        rl->scheduling = scheduling_key_append (orig->scheduling, NULL);

    /*  Copy noremap hash from orignal rlist
     */
    zhashx_destroy (&rl->noremap);
    rl->noremap = zhashx_dup (orig->noremap);
    if (!rl->noremap)
        return NULL;

    return rl;
fail:
    rlist_destroy (rl);
    return NULL;
}

struct rlist * rlist_copy_ranks (const struct rlist *rl, struct idset *ranks)
{
    unsigned int i;
    struct rnode *n;
    struct rlist *result = rlist_create ();
    if (!result)
        return NULL;

    i = idset_first (ranks);
    while (i != IDSET_INVALID_ID) {
        if ((n = rlist_find_rank (rl, i))) {
            struct rnode *copy = rnode_copy (n);
            if (!copy || rlist_add_rnode_new (result, copy) < 0) {
                rnode_destroy (copy);
                goto err;
            }
        }
        i = idset_next (ranks, i);
    }

    /*  Copy entire opaque scheduling key unless rlist is empty
     */
    if (rlist_nnodes (result) > 0)
        result->scheduling = scheduling_key_append (rl->scheduling, NULL);

    /*  Copy noremap hash from orignal rlist
     */
    zhashx_destroy (&result->noremap);
    result->noremap = zhashx_dup (rl->noremap);
    if (!result->noremap)
        return NULL;

    return result;
err:
    rlist_destroy (result);
    return NULL;
}

int rlist_remove_ranks (struct rlist *rl, struct idset *ranks)
{
    int count = 0;
    struct rnode *n;
    unsigned int i;
    i = idset_first (ranks);
    while (i != IDSET_INVALID_ID) {
        if ((n = rlist_find_rank (rl, i))) {
            zlistx_delete (rl->nodes, zlistx_cursor (rl->nodes));
            count++;
        }
        i = idset_next (ranks, i);
    }
    return count;
}

int rlist_remap (struct rlist *rl)
{
    uint32_t rank = 0;
    struct rnode *n;
    
    /*   Sort list by ascending rank, then rerank starting at 0
     */
    zlistx_set_comparator (rl->nodes, by_rank);
    zlistx_sort (rl->nodes);

    n = zlistx_first (rl->nodes);
    while (n) {
        n->rank = rank++;
        if (rnode_remap (n, rl->noremap) < 0)
            return -1;
        n = zlistx_next (rl->nodes);
    }
    return 0;
}

struct rnode * rlist_find_host (struct rlist *rl, const char *host)
{
    struct rnode *n = zlistx_first (rl->nodes);
    while (n) {
        if (n->hostname && strcmp (n->hostname, host) == 0)
            return n;
        n = zlistx_next (rl->nodes);
    }
    errno = ENOENT;
    return NULL;
}

static int rlist_rerank_hostlist (struct rlist *rl, struct hostlist *hl)
{
    uint32_t rank = 0;
    const char *host = hostlist_first (hl);
    while (host) {
        struct rnode *n = rlist_find_host (rl, host);
        if (!n)
            return -1;
        n->rank = rank++;
        host = hostlist_next (hl);
    }
    return 0;
}

int rlist_rerank (struct rlist *rl, const char *hosts)
{
    int rc = -1;
    struct hostlist *hl = NULL;
    struct hostlist *orig = NULL;

    if (!(hl = hostlist_decode (hosts)))
        return -1;

    if (hostlist_count (hl) > rlist_nnodes (rl)) {
        errno = EOVERFLOW;
        goto done;
    }
    else if (hostlist_count (hl) < rlist_nnodes (rl)) {
        errno = ENOSPC;
        goto done;
    }

    /* Save original rank mapping in case of undo
     */
    if (!(orig = rlist_nodelist (rl)))
        goto done;

    /* Perform re-ranking based on hostlist hl. On failure, undo
     *  by reranking with original hostlist.
     */
    if ((rc = rlist_rerank_hostlist (rl, hl)) < 0) {
        int saved_errno = errno;
        (void) rlist_rerank_hostlist (rl, orig);
        errno = saved_errno;
    }
done:
    hostlist_destroy (orig);
    hostlist_destroy (hl);
    return rc;
}

static struct rnode *rlist_detach_rank (struct rlist *rl, uint32_t rank)
{
    struct rnode *n = rlist_find_rank (rl, rank);
    if (n)
        zlistx_detach_cur (rl->nodes);
    return n;
}

struct rlist *rlist_diff (const struct rlist *rla, const struct rlist *rlb)
{
    struct rnode *n;
    struct rlist *rl = rlist_create ();

    if (!rl || rlist_append (rl, rla) < 0) {
        rlist_destroy (rl);
        return NULL;
    }

    n = zlistx_first (rlb->nodes);
    while (n) {
        /*  Attempt to find and "detach" the rank which we're diffing.
         */
        struct rnode *na = rlist_detach_rank (rl, n->rank);
        if (na) {
            /*  Diff the individual resource node.
             *  If the result is empty, then do nothing since we've
             *   already detached this node from the list. O/w, push
             *   the result back onto the rlist.
             */
            struct rnode *result = rnode_diff (na, n);
            if (!rnode_empty (result))
                rlist_add_rnode (rl, result);
            else
                rnode_destroy (result);

            /*  Always need to free the detached rnode */
            rnode_destroy (na);
        }
        n = zlistx_next (rlb->nodes);
    }
    return rl;
}

struct rlist *rlist_union (const struct rlist *rla, const struct rlist *rlb)
{
    struct rlist *result = NULL;

    /*  First take the set difference of b from a, such that there are no
     *   common resources in 'rlb' and 'result'.
     */
    if (!(result = rlist_diff (rla, rlb)))
        return NULL;

    /*  Now append 'rlb' to 'result' to get the union of 'rla' + 'rlb':
     */
    if (rlist_append (result, rlb) < 0) {
        rlist_destroy (result);
        return NULL;
    }

    return result;
}

struct rlist *rlist_intersect (const struct rlist *rla,
                               const struct rlist *rlb)
{
    struct rnode *n;
    struct rlist *result = rlist_create ();

    if (!result)
        return NULL;

    n = zlistx_first (rlb->nodes);
    while (n) {
        struct rnode *na = rlist_find_rank (rla, n->rank);
        struct rnode *nx = rnode_intersect (na, n);
        if (nx != NULL
            && !rnode_empty (nx)
            && rlist_add_rnode (result, nx) < 0)
            goto err;
        n = zlistx_next (rlb->nodes);
    }

    /*  Copy opaque scheduling key unless result is empty
     */
    if (rlist_nnodes (result) > 0)
        result->scheduling = scheduling_key_append (rla->scheduling, NULL);

    return result;
err:
    rlist_destroy (result);
    return NULL;
}

static void free_item (void **x)
{
    if (x) {
        free (*x);
        x = NULL;
    }
}

static zlistx_t *errlist_create ()
{
    zlistx_t *l = zlistx_new ();
    if (!l)
        return NULL;
    zlistx_set_destructor (l, free_item);
    return l;
}

static void errlist_destroy (zlistx_t *l)
{
    zlistx_destroy (&l);
}

static int errlist_append (zlistx_t *l, const char *fmt, ...)
{
    char *s = NULL;
    va_list ap;
    va_start (ap, fmt);
    if (vasprintf (&s, fmt, ap) < 0)
        return -1;
    va_end (ap);
    if (!zlistx_add_end (l, s)) {
        free (s);
        return -1;
    }
    return 0;
}

static int errlist_concat (zlistx_t *l, char *buf, size_t len)
{
    int n = 0;
    char *s;

    memset (buf, 0, len);

    s = zlistx_first (l);
    while (s) {
        if (len - n > 0)
            strncpy (buf + n, s, len - n);
        n += strlen (s);
        s = zlistx_next (l);
        if (s) {
            strncat (buf, ", ", len - n);
            n += 2;
        }
    }
    return len;
}

static int rnode_namecmp (const void *s1, const void *s2)
{
    const char *a = s1;
    const char *b = s2;

    if (strcmp (a, "core") == 0)
        return -1;
    else if (strcmp (b, "core") == 0)
        return 1;
    else return strcmp (a, b);
}

static int rnode_sprintfcat (const struct rnode *n,
                             char **dest,
                             size_t *sizep,
                             size_t *lenp)
{
    int rc = -1;
    zlistx_t *keys = NULL;
    char *ids = NULL;
    char *name = NULL;
    char *comma = "";

    /*  Generate sorted list of resources on those ranks.
     *  (Ensuring that "core" is always first)
     */
    keys = zhashx_keys (n->children);
    zlistx_set_comparator (keys, rnode_namecmp);
    zlistx_sort (keys);

    /*  Output all resource strings as name[ids]
     */
    name = zlistx_first (keys);
    while (name) {
        struct rnode_child *c = zhashx_lookup (n->children, name);

        /*  Skip empty sets */
        if (idset_count (c->avail) > 0) {
            ids = idset_encode (c->avail,
                                IDSET_FLAG_RANGE | IDSET_FLAG_BRACKETS);
            if (!ids)
                goto fail;
            if (sprintfcat (dest, sizep, lenp,
                            "%s%s%s",
                            comma,
                            c->name,
                            ids) < 0)
                goto fail;

            free (ids);
            ids = NULL;
            comma = ",";
        }
        name = zlistx_next (keys);
    }
    rc = 0;
fail:
    zlistx_destroy (&keys);
    free (ids);
    return rc;
}

static char * rnode_child_dumps (struct rnode *rnode)
{
    size_t n = 0;
    size_t len = 0;
    char *s = NULL;
    if (rnode_sprintfcat (rnode, &s, &n, &len) < 0) {
        free (s);
        return NULL;
    }
    return s;
}


int rlist_verify (rlist_error_t *errp,
                  const struct rlist *expected,
                  const struct rlist *rl)
{
    struct rnode *n = NULL;
    struct rnode *exp = NULL;
    struct rnode *diff = NULL;
    zlistx_t *errors = NULL;
    int saved_errno;
    int rc = -1;

    if (!(errors = errlist_create ())) {
        if (errp)
            snprintf (errp->text, sizeof (errp->text),
                      "Internal error: Out of memory");
        errno = ENOMEM;
        goto done;
    }

    if (rlist_nnodes (rl) != 1) {
        errlist_append (errors,
                        "Verification supported on single rank only");
        errno = EINVAL;
        goto done;
    }

    n = zlistx_first (rl->nodes);
    if (!(exp = rlist_find_rank (expected, n->rank))) {
        errlist_append (errors,
                        "rank %d not found in expected ranks",
                        n->rank);
        errno = EINVAL;
        goto done;
    }
    if (rnode_hostname_cmp (n, exp) != 0) {
        errlist_append (errors,
                        "rank %d got hostname '%s', expected '%s'",
                        n->rank,
                        n->hostname ? n->hostname : "unknown",
                        exp->hostname ? exp->hostname : "unknown");
        goto done;
    }
    if (!(diff = rnode_diff (exp, n))) {
        errlist_append (errors,
                        "Internal error: rnode_diff failed: %s",
                        strerror (errno));
        goto done;
    }
    if (!rnode_empty (diff)) {
        char *s = rnode_child_dumps (diff);
        errlist_append (errors,
                        "rank %d (%s) missing resources: %s",
                        n->rank, n->hostname ? n->hostname : "unknown", s);
        free (s);
        goto done;
    }
    rnode_destroy (diff);
    if (!(diff = rnode_diff (n, exp))) {
        errlist_append (errors,
                        "Internal error: rnode_diff failed: %s",
                        strerror (errno));
        goto done;
    }
    if (rnode_empty (diff))
        rc = 0;
    else {
        char *s = rnode_child_dumps (diff);
        errlist_append (errors,
                        "rank %d (%s) has extra resources: %s",
                        n->rank, n->hostname ? n->hostname : "unknown", s);
        free (s);
        rc = 1;
    }
done:
    saved_errno = 0;
    rnode_destroy (diff);
    memset (errp->text, 0, sizeof (errp->text));
    if (errors) {
        errlist_concat (errors, errp->text, sizeof (errp->text));
        errlist_destroy (errors);
    }
    errno = saved_errno;
    return rc;
}

int rlist_append (struct rlist *rl, const struct rlist *rl2)
{
    json_t *o;
    struct rnode *n = zlistx_first (rl2->nodes);
    while (n) {
        struct rnode *copy = rnode_copy_avail (n);
        if (!copy || rlist_add_rnode (rl, copy) < 0)
            return -1;
        n = zlistx_next (rl2->nodes);
    }

    o = scheduling_key_append (rl->scheduling, rl2->scheduling);
    json_decref (rl->scheduling);
    rl->scheduling = o;

    return 0;
}

static int rlist_append_by_rank (struct rlist *rl,
                                 const char *ranks,
                                 json_t *e,
                                 const char *name)
{
    int rc = -1;
    unsigned int n;
    unsigned int i;
    const char *corelist = NULL;
    const char *hostname = NULL;
    struct rnode *node;
    struct idset *ids = idset_decode (ranks);
    json_error_t err;

    if (!ids || json_unpack_ex (e, &err, 0, "{s:i s?s s?s}",
                                "Core", &n,
                                name, &corelist,
                                "hostname", &hostname) < 0)
        goto out;
    i = idset_first (ids);
    while (i != IDSET_INVALID_ID) {
        if (corelist)
            node = rnode_create (hostname, i, corelist);
        else
            node = rnode_create_count (hostname, i, n);
        if (!node || rlist_add_rnode (rl, node) < 0) {
            rnode_destroy (node);
            goto out;
        }
        i = idset_next (ids, i);
    }
    rc = 0;
out:
    idset_destroy (ids);
    return rc;
}

struct rlist *rlist_from_hwloc_by_rank (const char *by_rank, bool sched_pus)
{
    struct rlist *rl = NULL;
    const char *key = NULL;
    json_t *entry = NULL;

    json_t *o = json_loads (by_rank, 0, NULL);
    if (o == NULL)
        return NULL;
    if (!(rl = rlist_create ()))
        goto err;

    json_object_foreach (o, key, entry) {
        if (rlist_append_by_rank (rl,
                                  key,
                                  entry,
                                  sched_pus ? "cpuset" : "coreids") < 0)
            goto err;
    }
    json_decref (o);

    return (rl);
err:
    json_decref (o);
    rlist_destroy (rl);
    return NULL;
}

static int rlist_append_rank (struct rlist *rl,
                              const char *hostname,
                              unsigned int rank,
                              json_t *children)
{
    struct rnode *n = rnode_create_children (hostname, rank, children);
    if (!n || rlist_add_rnode (rl, n) < 0) {
        rnode_destroy (n);
        return -1;
    }
    return 0;
}

int rlist_append_rank_cores (struct rlist *rl,
                             const char *hostname,
                             unsigned int rank,
                             const char *core_ids)
{
    int rc;
    json_t *children = json_pack ("{s:s}", "core", core_ids);
    if (!children)
        return -1;
    rc = rlist_append_rank (rl, hostname, rank, children);
    json_decref (children);
    return rc;
}

int rlist_rank_add_child (struct rlist *rl,
                          unsigned int rank,
                          const char *name,
                          const char *ids)
{
    struct rnode *n = rlist_find_rank (rl, rank);
    if (!n) {
        errno = ENOENT;
        return -1;
    }
    if (rnode_add_child (n, name, ids) == NULL)
        return -1;
    return 0;
}

static int rlist_append_ranks (struct rlist *rl,
                               const char *rank,
                               json_t *children)
{
    int rc = -1;
    unsigned int i;
    struct idset * ranks = idset_decode (rank);
    if (!ranks)
        return -1;
    i = idset_first (ranks);
    while (i != IDSET_INVALID_ID) {
        if (rlist_append_rank (rl, NULL, i, children) < 0)
            goto err;
        i = idset_next (ranks, i);
    }
    rc = 0;
err:
    idset_destroy (ranks);
    return rc;
}

static int rlist_append_cores (struct rlist *rl,
                               const char *hostname,
                               int rank,
                               struct idset *idset)
{
    struct rnode *n = rnode_create_idset (hostname, rank, idset);
    if (!n || rlist_add_rnode (rl, n) < 0) {
        rnode_destroy (n);
        return -1;
    }
    return 0;
}

static int rlist_append_rank_entry (struct rlist *rl,
                                    json_t *entry,
                                    json_error_t *ep)
{
    const char *ranks;
    json_t *children;
    if (json_unpack_ex (entry, ep, 0,
                        "{s:s s:o}",
                        "rank", &ranks,
                        "children", &children) < 0) {
        return -1;
    }
    return rlist_append_ranks (rl, ranks, children);
}

static struct hostlist * hostlist_from_array (json_t *o)
{
    size_t index;
    json_t *val;

    struct hostlist *hl = hostlist_create ();
    if (hl == NULL)
        return NULL;

    json_array_foreach (o, index, val) {
        const char *hosts = json_string_value (val);
        if (hostlist_append (hl, hosts) < 0)
            goto err;
    }

    return hl;
err:
    hostlist_destroy (hl);
    return NULL;
}

static int rlist_assign_hostlist (struct rlist *rl, struct hostlist *hl)
{
    struct rnode *n;

    if (!hl || hostlist_count (hl) != zlistx_size (rl->nodes))
        return -1;

    /*  Reset default sort to order nodes by "rank" */
    zlistx_set_comparator (rl->nodes, by_rank);
    zlistx_sort(rl->nodes);

    /*  Consume a hostname for each node in the rlist */
    n = zlistx_first (rl->nodes);
    (void) hostlist_first (hl);
    while (n) {
        free (n->hostname);
        if (!(n->hostname = strdup (hostlist_current (hl))))
            return -1;
        (void) hostlist_next (hl);
        n = zlistx_next (rl->nodes);
    }
    return 0;
}

int rlist_assign_hosts (struct rlist *rl, const char *hosts)
{
    int rc;
    struct hostlist *hl = hostlist_decode (hosts);
    rc = rlist_assign_hostlist (rl, hl);
    hostlist_destroy (hl);
    return rc;
}

static int rlist_assign_nodelist (struct rlist *rl, json_t *nodelist)
{
    int rc = -1;
    struct hostlist *hl = hostlist_from_array (nodelist);
    rc = rlist_assign_hostlist (rl, hl);
    hostlist_destroy (hl);
    return rc;
}

struct rlist *rlist_from_json (json_t *o, json_error_t *errp)
{
    int i, version;
    struct rlist *rl = NULL;
    json_t *entry = NULL;
    json_t *R_lite = NULL;
    json_t *nodelist = NULL;
    json_t *scheduling = NULL;
    double starttime = -1.;
    double expiration = -1.;

    if (json_unpack_ex (o, errp, 0,
                        "{s:i s?O s:{s:o s?o s?F s?F}}",
                        "version", &version,
                        "scheduling", &scheduling,
                        "execution",
                          "R_lite", &R_lite,
                          "nodelist", &nodelist,
                          "starttime", &starttime,
                          "expiration", &expiration) < 0)
        goto err;
    if (version != 1)
        goto err;
    if (!(rl = rlist_create ()))
        goto err;

    if (scheduling)
        rl->scheduling = scheduling;
    if (starttime > 0.)
        rl->starttime = starttime;
    if (expiration > 0.)
        rl->expiration = expiration;

    json_array_foreach (R_lite, i, entry) {
        if (rlist_append_rank_entry (rl, entry, errp) < 0)
            goto err;
    }
    if (nodelist && rlist_assign_nodelist (rl, nodelist) < 0)
        goto err;
    return (rl);
err:
    rlist_destroy (rl);
    json_decref (o);
    return (NULL);
}

struct rlist *rlist_from_R (const char *s)
{
    struct rlist *rl = NULL;
    json_t *o = json_loads (s, 0, NULL);
    if (o)
        rl = rlist_from_json (o, NULL);
    json_decref (o);
    return rl;
}

/* Helper for rlist_compressed */
struct multi_rnode {
    struct idset *ids;
    const struct rnode *rnode;
};

static int multi_rnode_cmp (struct multi_rnode *x, const struct rnode *n)
{
    int rv = rnode_cmp (x->rnode, n);

    /* Only collapse nodes with same avail idset + same up/down status */
    if (rv == 0 && n->up == x->rnode->up)
        return 0;

    /* O/w, order doesn't matter too much, but put up nodes first */
    return n->up ? -1 : 1;
}

static void multi_rnode_destroy (struct multi_rnode **mrn)
{
    if (mrn && *mrn) {
        (*mrn)->rnode = NULL;
        idset_destroy ((*mrn)->ids);
        free (*mrn);
        *mrn = NULL;
    }
}

struct multi_rnode * multi_rnode_create (struct rnode *rnode)
{
    struct multi_rnode *mrn = calloc (1, sizeof (*mrn));
    if (mrn == NULL)
        return NULL;
    if (!(mrn->ids = idset_create (0, IDSET_FLAG_AUTOGROW))
        || (idset_set (mrn->ids, rnode->rank) < 0))
        goto fail;
    mrn->rnode = rnode;
    return (mrn);
fail:
    multi_rnode_destroy (&mrn);
    return NULL;
}

json_t *multi_rnode_tojson (struct multi_rnode *mrn)
{
    return rnode_encode (mrn->rnode, mrn->ids);
}


static int multi_rnode_by_rank (const void *item1, const void *item2)
{
    const struct multi_rnode *mrn1 = item1;
    const struct multi_rnode *mrn2 = item2;

    unsigned int x = idset_first (mrn1->ids);
    unsigned int y = idset_first (mrn2->ids);

    return (x - y);
}

static zlistx_t * rlist_mrlist (const struct rlist *rl)
{
    struct rnode *n = NULL;
    struct multi_rnode *mrn = NULL;
    zlistx_t *l = zlistx_new ();

    zlistx_set_comparator (l, (czmq_comparator *) multi_rnode_cmp);
    zlistx_set_destructor (l, (czmq_destructor *) multi_rnode_destroy);

    n = zlistx_first (rl->nodes);
    while (n) {
        if (zlistx_find (l, n)) {
            if (!(mrn = zlistx_handle_item (zlistx_cursor (l)))
                    || idset_set (mrn->ids, n->rank) < 0) {
                goto fail;
            }
        }
        else {
            if (!(mrn = multi_rnode_create (n))
                    || !zlistx_add_end (l, mrn)) {
                goto fail;
            }
        }
        n = zlistx_next (rl->nodes);
    }
    return (l);
fail:
    zlistx_destroy (&l);
    return NULL;
}

static json_t * rlist_compressed (struct rlist *rl)
{
    struct multi_rnode *mrn = NULL;
    json_t *o = json_array ();
    zlistx_t *l = rlist_mrlist (rl);

    if (!l)
        return NULL;
    zlistx_set_comparator (l, (czmq_comparator *) multi_rnode_by_rank);
    zlistx_sort (l);
    mrn = zlistx_first (l);
    while (mrn) {
        if (rnode_avail_total (mrn->rnode) > 0) {
            json_t *entry = multi_rnode_tojson (mrn);
            if (!entry || json_array_append_new (o, entry) != 0) {
                json_decref (entry);
                goto fail;
            }
        }
        mrn = zlistx_next (l);
    }
    zlistx_destroy (&l);
    return (o);
fail:
    zlistx_destroy (&l);
    json_decref (o);
    return NULL;
}

static int mrnode_sprintfcat (struct multi_rnode *mrn,
                              char **resultp,
                              size_t *sizep,
                              size_t *lenp)
{
    int flags = IDSET_FLAG_RANGE | IDSET_FLAG_BRACKETS;
    int rc = -1;
    char *ranks = NULL;

    /* Do not output anything if there are no available resources */
    if (rnode_avail_total (mrn->rnode) == 0)
        return 0;

    /*  First encode set of ranks into a string:
     */
    if (!(ranks = idset_encode (mrn->ids, flags)))
        goto fail;
    if (sprintfcat (resultp, sizep, lenp,
                    "%srank%s/",
                    (*resultp)[0] != '\0' ? " ": "",
                    ranks) < 0)
        goto fail;

    if (rnode_sprintfcat (mrn->rnode, resultp, sizep, lenp) < 0)
        goto fail;

    rc = 0;
fail:
    free (ranks);
    return rc;
}

char * rlist_dumps (const struct rlist *rl)
{
    char * result = NULL;
    size_t len = 0;
    size_t size = 64;
    struct multi_rnode *mrn = NULL;
    zlistx_t *l = NULL;

    if (rl == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if (!(l = rlist_mrlist (rl))
        || !(result = calloc (size, sizeof (char))))
        goto fail;

    mrn = zlistx_first (l);
    while (mrn) {
        if (mrnode_sprintfcat (mrn, &result, &size, &len) < 0)
            goto fail;
        mrn = zlistx_next (l);
    }
    zlistx_destroy (&l);
    return (result);
fail:
    free (result);
    zlistx_destroy (&l);
    return NULL;
}

static json_t *hostlist_to_nodelist (struct hostlist *hl)
{
    json_t *o = NULL;
    char *hosts;
    if ((hosts = hostlist_encode (hl)))
        o = json_pack ("[s]", hosts);
    free (hosts);
    return o;
}

struct idset *rlist_ranks (const struct rlist *rl)
{
    struct rnode *n;
    struct idset *ids = idset_create (0, IDSET_FLAG_AUTOGROW);
    if (!ids)
        return NULL;

    n = zlistx_first (rl->nodes);
    while (n) {
        if (idset_set (ids, n->rank) < 0)
            goto fail;
        n = zlistx_next (rl->nodes);
    }
    return ids;
fail:
    idset_destroy (ids);
    return NULL;
}

struct hostlist *rlist_nodelist (const struct rlist *rl)
{
    struct rnode *n;
    struct hostlist *hl = hostlist_create ();

    if (!hl)
        return NULL;

    /*  List must be sorted by rank before collecting nodelist
     */
    zlistx_set_comparator (rl->nodes, by_rank);
    zlistx_sort (rl->nodes);

    n = zlistx_first (rl->nodes);
    while (n) {
        if (!n->hostname || hostlist_append (hl, n->hostname) < 0)
            goto fail;
        n = zlistx_next (rl->nodes);
    }
    return hl;
fail:
    hostlist_destroy (hl);
    return NULL;
}

static json_t *rlist_json_nodelist (struct rlist *rl)
{
    json_t *o = NULL;
    struct hostlist *hl = rlist_nodelist (rl);
    if (!hl)
        return NULL;
    o = hostlist_to_nodelist (hl);
    hostlist_destroy (hl);
    return o;
}

json_t *rlist_to_R (struct rlist *rl)
{
    json_t *R = NULL;
    json_t *R_lite = rlist_compressed (rl);
    json_t *nodelist = rlist_json_nodelist (rl);

    if (!R_lite)
        goto fail;
    if (!(R = json_pack ("{s:i, s:{s:o s:f s:f}}",
                         "version", 1,
                         "execution",
                         "R_lite", R_lite,
                         "starttime", rl->starttime,
                         "expiration", rl->expiration)))
        goto fail;
    if (nodelist
        && json_object_set_new (json_object_get (R, "execution"),
                                "nodelist", nodelist) < 0)
        goto fail;
    if (rl->scheduling
        && json_object_set (R, "scheduling", rl->scheduling) < 0)
        goto fail;

    return (R);
fail:
    json_decref (R);
    json_decref (nodelist);
    return NULL;
}

static int by_rank (const void *item1, const void *item2)
{
    const struct rnode *x = item1;
    const struct rnode *y = item2;
    return (x->rank - y->rank);
}

static int by_avail (const void *item1, const void *item2)
{
    int n;
    const struct rnode *x = item1;
    const struct rnode *y = item2;
    if ((n = rnode_avail (x) - rnode_avail (y)) == 0)
        n = by_rank (x, y);
    return n;
}

static int by_used (const void *item1, const void *item2)
{
    int n;
    const struct rnode *x = item1;
    const struct rnode *y = item2;
    if ((n = rnode_avail (y) - rnode_avail (x)) == 0)
        n = by_rank (x, y);
    return n;
}

static int rlist_rnode_alloc (struct rlist *rl, struct rnode *n,
                              int count, struct idset **idsetp)
{
    if (!n || rnode_alloc (n, count, idsetp) < 0)
        return -1;
    rl->avail -= idset_count (*idsetp);
    return 0;
}

#if 0
static uint32_t rlist_rnode_rank (struct rlist *rl)
{
    struct rnode *n = zlistx_item (rl->nodes);
    if (n)
        return n->rank;
    else
        return (uint32_t)-1;
}
#endif

static struct rnode *rlist_first (struct rlist *rl)
{
    return zlistx_first (rl->nodes);
}

static struct rnode *rlist_next (struct rlist *rl)
{
    return zlistx_next (rl->nodes);
}

/*
 *  Allocate the first available N slots of size cores_per_slot from
 *   resource list rl after sorting the nodes with the current sort strategy.
 */
static struct rlist * rlist_alloc_first_fit (struct rlist *rl,
                                             int cores_per_slot,
                                             int slots)
{
    int rc;
    struct idset *ids = NULL;
    struct rnode *n = NULL;
    struct rlist *result = NULL;

    zlistx_sort (rl->nodes);

    if (!(n = rlist_first (rl)))
        return NULL;

    if (!(result = rlist_create ()))
        return NULL;

    /* 2. assign slots to first nodes where they fit
     */
    while (n && slots) {
        /*  Try to allocate a slot on this node. If we fail with ENOSPC,
         *   then advance to the next node and try again.
         */
        if ((rc = rlist_rnode_alloc (rl, n, cores_per_slot, &ids)) < 0) {
            if (errno != ENOSPC)
                goto unwind;
            n = rlist_next (rl);
            continue;
        }
        /*  Append the allocated cores to the result set and continue
         *   if needed
         */
        rc = rlist_append_cores (result, n->hostname, n->rank, ids);
        idset_destroy (ids);
        if (rc < 0)
            goto unwind;
        slots--;
    }
    if (slots != 0) {
unwind:
        rlist_free (rl, result);
        rlist_destroy (result);
        errno = ENOSPC;
        return NULL;
    }
    return result;
}

/*
 *  Allocate `slots` of size cores_per_slot from rlist `rl` and return
 *   the result. Sorts the node list by smallest available first, so that
 *   we get something like "best fit". (minimize nodes used)
 */
static struct rlist * rlist_alloc_best_fit (struct rlist *rl,
                                            int cores_per_slot,
                                            int slots)
{
    zlistx_set_comparator (rl->nodes, by_avail);
    return rlist_alloc_first_fit (rl, cores_per_slot, slots);
}

/*
 *  Allocate `slots` of size cores_per_slot from rlist `rl` and return
 *   the result. Sorts the node list by least utilized first, so that
 *   we get something like "worst fit". (Spread jobs across nodes)
 */
static struct rlist * rlist_alloc_worst_fit (struct rlist *rl,
                                             int cores_per_slot,
                                             int slots)
{
    zlistx_set_comparator (rl->nodes, by_used);
    return rlist_alloc_first_fit (rl, cores_per_slot, slots);
}


static zlistx_t *rlist_get_nnodes (struct rlist *rl, int nnodes)
{
    struct rnode *n;
    zlistx_t *l = zlistx_new ();
    if (!l)
        return NULL;
    n = zlistx_first (rl->nodes);
    while (nnodes > 0) {
        if (n == NULL) {
            errno = ENOSPC;
            goto err;
        }
        if (n->up) {
            if (!zlistx_add_end (l, n))
                goto err;
            nnodes--;
        }
        n = zlistx_next (rl->nodes);
    }
    return (l);
err:
    zlistx_destroy (&l);
    return NULL;
}

/*  Allocate 'slots' of size 'cores_per_slot' across exactly `nnodes`.
 *  Works by getting the first N least utilized nodes and spreading
 *  the nslots evenly across the result.
 */
static struct rlist *rlist_alloc_nnodes (struct rlist *rl, int nnodes,
                                         int cores_per_slot, int slots)
{
    struct rlist *result = NULL;
    struct rnode *n = NULL;
    zlistx_t *cl = NULL;

    if (rlist_nnodes (rl) < nnodes) {
        errno = ENOSPC;
        return NULL;
    }
    if (slots < nnodes) {
        errno = EINVAL;
        return NULL;
    }
    if (!(result = rlist_create ()))
        return NULL;

    /* 1. sort rank list by used cores ascending:
     */
    zlistx_set_comparator (rl->nodes, by_used);
    zlistx_sort (rl->nodes);

    /* 2. get a list of the first up n nodes
     */
    if (!(cl = rlist_get_nnodes (rl, nnodes)))
        goto unwind;

    /* We will sort candidate list by used cores on each iteration to
     *  ensure even spread of slots across nodes
     */
    zlistx_set_comparator (cl, by_used);

    /*
     * 3. divide slots across all nodes, placing each slot
     *    on most empty node first
     */
    while (slots > 0) {
        int rc;
        struct idset *ids = NULL;
        n = zlistx_first (cl);
        /*
         * if we can't allocate on this node, give up. Since it is the
         *  least loaded node from the least loaded nodelist, we know
         *  we don't have enough resources to satisfy request.
         */
        if (rlist_rnode_alloc (rl, n, cores_per_slot, &ids) < 0)
            goto unwind;
        rc = rlist_append_cores (result, n->hostname, n->rank, ids);
        idset_destroy (ids);
        if (rc < 0)
            goto unwind;

        /*  If a node is empty, remove it from consideration.
         *  O/w, force it to the back of the list to ensure all N
         *   nodes are considered at least once.
         */
        zlistx_reorder (cl, zlistx_cursor (cl), false);
        if (rnode_avail (n) == 0)
            zlistx_detach (cl, zlistx_cursor (cl));
        else
            zlistx_move_end (cl, zlistx_cursor (cl));
        slots--;
    }
    zlistx_destroy (&cl);
    return result;
unwind:
    zlistx_destroy (&cl);
    rlist_free (rl, result);
    rlist_destroy (result);
    errno = ENOSPC;
    return NULL;
}

static struct rlist *rlist_try_alloc (struct rlist *rl, const char *mode,
                                      int nnodes, int slots, int cores_per_slot)
{
    struct rlist *result = NULL;

    if (!rl) {
        errno = EINVAL;
        return NULL;
    }

    /*  Reset default sort to order nodes by "rank" */
    zlistx_set_comparator (rl->nodes, by_rank);

    if (nnodes > 0)
        result = rlist_alloc_nnodes (rl, nnodes, cores_per_slot, slots);
    else if (mode == NULL || strcmp (mode, "worst-fit") == 0)
        result = rlist_alloc_worst_fit (rl, cores_per_slot, slots);
    else if (mode && strcmp (mode, "best-fit") == 0)
        result = rlist_alloc_best_fit (rl, cores_per_slot, slots);
    else if (mode && strcmp (mode, "first-fit") == 0)
        result = rlist_alloc_first_fit (rl, cores_per_slot, slots);
    else
        errno = EINVAL;
    return result;
}

/*  Determine if allocation request is feasible for rlist `rl`.
 */
static bool rlist_alloc_feasible (const struct rlist *rl, const char *mode,
                                  int nnodes, int slots, int slotsz)
{
    bool rc = false;
    struct rlist *result = NULL;
    struct rlist *all = rlist_copy_empty (rl);
    if (all && (result = rlist_try_alloc (all, mode, nnodes, slots, slotsz)))
        rc = true;
    rlist_destroy (all);
    rlist_destroy (result);
    return rc;
}

struct rlist *rlist_alloc (struct rlist *rl, const char *mode,
                          int nnodes, int slots, int slotsz)
{
    int total = slots * slotsz;
    struct rlist *result = NULL;

    if (slots <= 0 || slotsz <= 0 || nnodes < 0) {
        errno = EINVAL;
        return NULL;
    }
    if (total > rl->total) {
        errno = EOVERFLOW;
        return NULL;
    }
    if (total > rl->avail) {
        if (rlist_alloc_feasible (rl, mode, nnodes, slots, slotsz))
            errno = ENOSPC;
        else
            errno = EOVERFLOW;
        return NULL;
    }

    /*
     *   Try allocation. If it fails with not enough resources (ENOSPC),
     *    then try again on an empty copy of rlist to see the request could
     *    *ever* be satisfied. Adjust errno to EOVERFLOW if not.
     */
    result = rlist_try_alloc (rl, mode, nnodes, slots, slotsz);
    if (!result && (errno == ENOSPC)) {
        if (rlist_alloc_feasible (rl, mode, nnodes, slots, slotsz))
            errno = ENOSPC;
        else
            errno = EOVERFLOW;
    }
    return (result);
}

static int rlist_free_rnode (struct rlist *rl, struct rnode *n)
{
    struct rnode *rnode = rlist_find_rank (rl, n->rank);
    if (!rnode) {
        errno = ENOENT;
        return -1;
    }
    if (rnode_free_idset (rnode, n->cores->ids) < 0)
        return -1;
    if (rnode->up)
        rl->avail += idset_count (n->cores->ids);
    return 0;
}

static int rlist_alloc_rnode (struct rlist *rl, struct rnode *n)
{
    struct rnode *rnode = rlist_find_rank (rl, n->rank);
    if (!rnode) {
        errno = ENOENT;
        return -1;
    }
    if (rnode_alloc_idset (rnode, n->cores->avail) < 0)
        return -1;
    rl->avail -= idset_count (n->cores->avail);
    return 0;
}

int rlist_free (struct rlist *rl, struct rlist *alloc)
{
    zlistx_t *freed = NULL;
    struct rnode *n = NULL;

    if (!(freed = zlistx_new ()))
        return -1;

    n = zlistx_first (alloc->nodes);
    while (n) {
        if (rlist_free_rnode (rl, n) < 0
            || !zlistx_add_end (freed, n))
            goto cleanup;
        n = zlistx_next (alloc->nodes);
    }
    zlistx_destroy (&freed);
    return (0);
cleanup:
    /* re-allocate all freed items */
    n = zlistx_first (freed);
    while (n) {
        rlist_alloc_rnode (rl, n);
        n = zlistx_next (freed);
    }
    zlistx_destroy (&freed);
    return (-1);
}

int rlist_set_allocated (struct rlist *rl, struct rlist *alloc)
{
    zlistx_t *allocd = NULL;
    struct rnode *n = NULL;
    if (!alloc || !(allocd = zlistx_new ()))
        return -1;
    n = zlistx_first (alloc->nodes);
    while (n) {
        if (rlist_alloc_rnode (rl, n) < 0
            || !zlistx_add_end (allocd, n))
            goto cleanup;
        n = zlistx_next (alloc->nodes);
    }
    zlistx_destroy (&allocd);
    return 0;
cleanup:
    n = zlistx_first (allocd);
    while (n) {
        rlist_free_rnode (rl, n);
        n = zlistx_next (allocd);
    }
    zlistx_destroy (&allocd);
    return -1;
}

size_t rlist_nnodes (const struct rlist *rl)
{
    return zlistx_size (rl->nodes);
}

size_t rlist_count (const struct rlist *rl, const char *type)
{
    struct rnode *n;
    size_t count = 0;
    n = zlistx_first (rl->nodes);
    while (n) {
        count += rnode_count_type (n, type);
        n = zlistx_next (rl->nodes);
    }
    return count;
}

/* Mark all nodes in state 'up'. Count number of cores that changed
 *  availability state.
 */
static int rlist_mark_all (struct rlist *rl, bool up)
{
    int count = 0;
    struct rnode *n = zlistx_first (rl->nodes);
    while (n) {
        if (n->up != up)
            count += idset_count (n->cores->avail);
        n->up = up;
        n = zlistx_next (rl->nodes);
    }
    return count;
}

static int rlist_mark_state (struct rlist *rl, bool up, const char *ids)
{
    int count = 0;
    unsigned int i;
    struct idset *idset = idset_decode (ids);
    if (idset == NULL)
        return -1;
    i = idset_first (idset);
    while (i != IDSET_INVALID_ID) {
        struct rnode *n = rlist_find_rank (rl, i);
        if (n->up != up)
            count += idset_count (n->cores->avail);
        n->up = up;
        i = idset_next (idset, i);
    }
    idset_destroy (idset);
    return count;
}

int rlist_mark_down (struct rlist *rl, const char *ids)
{
    int count;
    if (strcmp (ids, "all") == 0)
        count = rlist_mark_all (rl, false);
    else
        count = rlist_mark_state (rl, false, ids);
    rl->avail -= count;
    return 0;
}

int rlist_mark_up (struct rlist *rl, const char *ids)
{
    int count;
    if (strcmp (ids, "all") == 0)
        count = rlist_mark_all (rl, true);
    else
        count = rlist_mark_state (rl, true, ids);
    rl->avail += count;
    return 0;
}


struct rlist *rlist_from_hwloc (int rank, const char *xml)
{
    char *ids = NULL;
    struct rnode *n = NULL;
    hwloc_topology_t topo = NULL;
    const char *name;
    struct rlist *rl = rlist_create ();

    if (!rl)
        return NULL;

    if (xml)
        topo = rhwloc_xml_topology_load (xml);
    else
        topo = rhwloc_local_topology_load ();
    if (!topo)
        goto fail;
    if (!(ids = rhwloc_core_idset_string (topo))
        || !(name = rhwloc_hostname (topo)))
        goto fail;

    if (!(n = rnode_create (name, rank, ids))
        || rlist_add_rnode (rl, n) < 0)
        goto fail;

    free (ids);

    if ((ids = rhwloc_gpu_idset_string (topo))
        && rnode_add_child (n, "gpu", ids) < 0)
        goto fail;

    hwloc_topology_destroy (topo);
    free (ids);
    return rl;
fail:
    rlist_destroy (rl);
    rnode_destroy (n);
    free (ids);
    hwloc_topology_destroy (topo);
    return NULL;
}

/* vi: ts=4 sw=4 expandtab
 */
