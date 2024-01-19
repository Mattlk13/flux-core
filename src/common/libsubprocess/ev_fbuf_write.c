/************************************************************\
 * Copyright 2015 Lawrence Livermore National Security, LLC
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
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include "src/common/libev/ev.h"

#include "ev_fbuf_write.h"

#include "fbuf_private.h"

static void buffer_write_cb (struct ev_loop *loop, ev_io *iow, int revents)
{
    struct ev_fbuf_write *ebw = iow->data;

    if (revents & EV_WRITE) {

        if (fbuf_read_to_fd (ebw->fb, ebw->fd, -1) < 0)
            return;

        if (!fbuf_bytes (ebw->fb) && ebw->eof) {
            if (close (ebw->fd) < 0)
                ebw->close_errno = errno;
            ebw->fd = -1;
            ebw->closed = true;
            ebw->eof = false;
            if (ebw->cb)
                ebw->cb (loop, ebw, revents);
        }

        if (!fbuf_bytes (ebw->fb) && !ebw->eof)
            ev_io_stop (ebw->loop, &(ebw->io_w));
    }
    else {
        if (ebw->cb)
            ebw->cb (loop, ebw, revents);
    }
}

/* data is available, start ev io watcher assuming user has
 * started the watcher.
 */
void ev_fbuf_write_wakeup (struct ev_fbuf_write *ebw)
{
    if (ebw->start)
        ev_io_start (ebw->loop, &(ebw->io_w));
}

static void buffer_data_available_cb (struct fbuf *fb, void *arg)
{
    struct ev_fbuf_write *ebw = arg;
    ev_fbuf_write_wakeup (ebw);
}

int ev_fbuf_write_init (struct ev_fbuf_write *ebw,
                        int fd,
                        int size,
                        ev_fbuf_write_f cb,
                        struct ev_loop *loop)
{
    ebw->cb = cb;
    ebw->fd = fd;
    ebw->loop = loop;
    ebw->start = false;

    if (!(ebw->fb = fbuf_create (size)))
        goto cleanup;

    /* When any data becomes available, call buffer_data_available_cb,
     * which will start io reactor
     */
    if (fbuf_set_low_read_cb (ebw->fb,
                              buffer_data_available_cb,
                              0,
                              ebw) < 0)
        goto cleanup;

    ev_io_init (&ebw->io_w, buffer_write_cb, ebw->fd, EV_WRITE);
    ebw->io_w.data = ebw;

    return 0;

cleanup:
    ev_fbuf_write_cleanup (ebw);
    return -1;
}

void ev_fbuf_write_cleanup (struct ev_fbuf_write *ebw)
{
    if (ebw) {
        fbuf_destroy (ebw->fb);
        ebw->fb = NULL;
    }
}

void ev_fbuf_write_start (struct ev_loop *loop, struct ev_fbuf_write *ebw)
{
    if (!ebw->start) {
        ebw->start = true;
        /* do not start watcher unless there is data or EOF to be written out */
        if (fbuf_bytes (ebw->fb) || ebw->eof)
            ev_io_start (ebw->loop, &(ebw->io_w));
    }
}

void ev_fbuf_write_stop (struct ev_loop *loop, struct ev_fbuf_write *ebw)
{
    if (ebw->start) {
        ev_io_stop (loop, &ebw->io_w);
        ebw->start = false;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

