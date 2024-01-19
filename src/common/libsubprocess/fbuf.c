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
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include "fbuf.h"

#include "src/common/liblsd/cbuf.h"

#define FBUF_MIN   4096

struct fbuf {
    int size;
    bool readonly;
    cbuf_t cbuf;
    char *buf;                  /* internal buffer for user reads */
    int buflen;
    fbuf_notify_f cb;
    void *cb_arg;
};

struct fbuf *fbuf_create (int size)
{
    struct fbuf *fb = NULL;
    int minsize = FBUF_MIN;

    if (size <= 0) {
        errno = EINVAL;
        goto cleanup;
    }

    if (!(fb = calloc (1, sizeof (*fb)))) {
        errno = ENOMEM;
        goto cleanup;
    }

    fb->size = size;
    if (size < FBUF_MIN)
        minsize = size;
    else
        minsize = FBUF_MIN;
    fb->readonly = false;

    /* buffer can grow to size specified by user */
    if (!(fb->cbuf = cbuf_create (minsize, fb->size)))
        goto cleanup;

    if (cbuf_opt_set (fb->cbuf, CBUF_OPT_OVERWRITE, CBUF_NO_DROP) < 0)
        goto cleanup;

    /* +1 for possible NUL on line reads */
    fb->buflen = minsize + 1;

    if (!(fb->buf = malloc (fb->buflen))) {
        errno = ENOMEM;
        goto cleanup;
    }

    return fb;

cleanup:
    fbuf_destroy (fb);
    return NULL;
}

void fbuf_set_notify (struct fbuf *fb, fbuf_notify_f cb, void *arg)
{
    if (fb) {
        fb->cb = cb;
        fb->cb_arg = arg;
    }
}

void fbuf_destroy (void *data)
{
    struct fbuf *fb = data;
    if (fb) {
        cbuf_destroy (fb->cbuf);
        free (fb->buf);
        free (fb);
    }
}

int fbuf_size (struct fbuf *fb)
{
    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    return fb->size;
}

int fbuf_bytes (struct fbuf *fb)
{
    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    return cbuf_used (fb->cbuf);
}

int fbuf_space (struct fbuf *fb)
{
    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    return cbuf_free (fb->cbuf);
}

int fbuf_readonly (struct fbuf *fb)
{
    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    fb->readonly = true;
    return 0;
}

bool fbuf_is_readonly (struct fbuf *fb)
{
    if (!fb) {
        errno = EINVAL;
        return false;
    }
    return fb->readonly;
}

static void nonempty_transition_check (struct fbuf *fb, bool was_empty)
{
    if (was_empty && cbuf_used (fb->cbuf) > 0) {
        if (fb->cb)
            fb->cb (fb, fb->cb_arg);
    }
}

static void nonfull_transition_check (struct fbuf *fb, bool was_full)
{
    if (was_full && cbuf_free (fb->cbuf) > 0) {
        if (fb->cb)
            fb->cb (fb, fb->cb_arg);
    }
}

int fbuf_drop (struct fbuf *fb, int len)
{
    int ret;

    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    bool full = cbuf_free (fb->cbuf) == 0 ? true : false;

    if ((ret = cbuf_drop (fb->cbuf, len)) < 0)
        return -1;

    nonfull_transition_check (fb, full);

    return ret;
}

/* check if internal buffer can hold data from user */
static int return_buffer_check (struct fbuf *fb)
{
    int used = cbuf_used (fb->cbuf);

    if (used < 0)
        return -1;

    assert (used <= fb->size);

    /* +1 for potential NUL char */
    if (fb->buflen < (used + 1)) {
        size_t newsize = fb->buflen;
        char *newbuf;

        while ((newsize < (used + 1))) {
            newsize = (newsize - 1) * 2 + 1;
            if (newsize > (fb->size + 1))
                newsize = fb->size + 1;
        }

        if (!(newbuf = realloc (fb->buf, newsize)))
            return -1;
        fb->buf = newbuf;
        fb->buflen = newsize;
    }

    return 0;
}

const void *fbuf_peek (struct fbuf *fb, int len, int *lenp)
{
    int ret;

    if (!fb) {
        errno = EINVAL;
        return NULL;
    }

    if (return_buffer_check (fb) < 0)
        return NULL;

    if (len < 0)
        len = cbuf_used (fb->cbuf);

    if (len > fb->buflen)
        len = fb->buflen;

    if ((ret = cbuf_peek (fb->cbuf, fb->buf, len)) < 0)
        return NULL;
    fb->buf[ret] = '\0';

    if (lenp)
        (*lenp) = ret;

    return fb->buf;
}

const void *fbuf_read (struct fbuf *fb, int len, int *lenp)
{
    int ret;

    if (!fb) {
        errno = EINVAL;
        return NULL;
    }

    if (return_buffer_check (fb) < 0)
        return NULL;

    if (len < 0)
        len = cbuf_used (fb->cbuf);

    if (len > fb->buflen)
        len = fb->buflen;

    bool full = cbuf_free (fb->cbuf) == 0 ? true : false;

    if ((ret = cbuf_read (fb->cbuf, fb->buf, len)) < 0)
        return NULL;
    fb->buf[ret] = '\0';

    if (lenp)
        (*lenp) = ret;

    nonfull_transition_check (fb, full);

    return fb->buf;
}

int fbuf_write (struct fbuf *fb, const void *data, int len)
{
    int ret;

    if (!fb || !data || len < 0) {
        errno = EINVAL;
        return -1;
    }

    if (fb->readonly) {
        errno = EROFS;
        return -1;
    }

    bool empty = cbuf_is_empty (fb->cbuf);

    if ((ret = cbuf_write (fb->cbuf, (void *)data, len, NULL)) < 0)
        return -1;

    nonempty_transition_check (fb, empty);

    return ret;
}

int fbuf_lines (struct fbuf *fb)
{
    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    return cbuf_lines_used (fb->cbuf);
}

bool fbuf_has_line (struct fbuf *fb)
{
    char buf[1];
    if (!fb) {
        errno = EINVAL;
        return false;
    }
    return (cbuf_peek_line (fb->cbuf, buf, 0, 1) > 0);
}

int fbuf_drop_line (struct fbuf *fb)
{
    int ret;

    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    bool empty = cbuf_is_empty (fb->cbuf);

    if ((ret = cbuf_drop_line (fb->cbuf, fb->buflen, 1)) < 0)
        return -1;

    nonempty_transition_check (fb, empty);

    return ret;
}

const void *fbuf_peek_line (struct fbuf *fb, int *lenp)
{
    int ret;

    if (!fb) {
        errno = EINVAL;
        return NULL;
    }

    if (return_buffer_check (fb) < 0)
        return NULL;

    if ((ret = cbuf_peek_line (fb->cbuf, fb->buf, fb->buflen, 1)) < 0)
        return NULL;

    if (lenp)
        (*lenp) = ret;

    return fb->buf;
}

const void *fbuf_peek_trimmed_line (struct fbuf *fb, int *lenp)
{
    int tmp_lenp = 0;

    if (!fbuf_peek_line (fb, &tmp_lenp))
        return NULL;

    if (tmp_lenp) {
        if (fb->buf[tmp_lenp - 1] == '\n') {
            fb->buf[tmp_lenp - 1] = '\0';
            tmp_lenp--;
        }
    }
    if (lenp)
        (*lenp) = tmp_lenp;

    return fb->buf;
}

const void *fbuf_read_line (struct fbuf *fb, int *lenp)
{
    int ret;

    if (!fb) {
        errno = EINVAL;
        return NULL;
    }

    if (return_buffer_check (fb) < 0)
        return NULL;

    bool full = cbuf_free (fb->cbuf) == 0 ? true : false;

    if ((ret = cbuf_read_line (fb->cbuf, fb->buf, fb->buflen, 1)) < 0)
        return NULL;

    if (lenp)
        (*lenp) = ret;

    nonfull_transition_check (fb, full);

    return fb->buf;
}

const void *fbuf_read_trimmed_line (struct fbuf *fb, int *lenp)
{
    int tmp_lenp = 0;

    if (!fbuf_read_line (fb, &tmp_lenp))
        return NULL;

    if (tmp_lenp) {
        if (fb->buf[tmp_lenp - 1] == '\n') {
            fb->buf[tmp_lenp - 1] = '\0';
            tmp_lenp--;
        }
    }
    if (lenp)
        (*lenp) = tmp_lenp;

    return fb->buf;
}

int fbuf_write_line (struct fbuf *fb, const char *data)
{
    int ret;

    if (!fb || !data) {
        errno = EINVAL;
        return -1;
    }

    if (fb->readonly) {
        errno = EROFS;
        return -1;
    }

    bool empty = cbuf_is_empty (fb->cbuf);

    if ((ret = cbuf_write_line (fb->cbuf, (char *)data, NULL)) < 0)
        return -1;

    nonempty_transition_check (fb, empty);

    return ret;
}

int fbuf_peek_to_fd (struct fbuf *fb, int fd, int len)
{
    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    return cbuf_peek_to_fd (fb->cbuf, fd, len);
}

int fbuf_read_to_fd (struct fbuf *fb, int fd, int len)
{
    int ret;

    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    bool full = cbuf_free (fb->cbuf) == 0 ? true : false;

    if ((ret = cbuf_read_to_fd (fb->cbuf, fd, len)) < 0)
        return -1;

    nonfull_transition_check (fb, full);

    return ret;
}

int fbuf_write_from_fd (struct fbuf *fb, int fd, int len)
{
    int ret;

    if (!fb) {
        errno = EINVAL;
        return -1;
    }

    if (fb->readonly) {
        errno = EROFS;
        return -1;
    }

    bool empty = cbuf_is_empty (fb->cbuf);

    if ((ret = cbuf_write_from_fd (fb->cbuf, fd, len, NULL)) < 0)
        return -1;

    nonempty_transition_check (fb, empty);

    return ret;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
