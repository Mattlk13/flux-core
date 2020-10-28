/************************************************************\
 * Copyright 2019 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef _FLUX_SCHEDUTIL_HELLO_H
#define _FLUX_SCHEDUTIL_HELLO_H

#include <flux/core.h>

#include "init.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Callback for ingesting R + metadata for jobs that have resources
 * Return 0 on success, -1 on failure with errno set.
 * Failure of the callback aborts iteration and causes schedutil_hello()
 * to return -1 with errno passed through.
 */
typedef int (schedutil_hello_cb_f)(flux_t *h,
                                   flux_jobid_t id,
                                   int priority,
                                   uint32_t userid,
                                   double t_submit,
                                   const char *R,
                                   void *arg);

/* Send hello announcement to job-manager.
 * The job-manager responds with a list of jobs that have resources assigned.
 * This function looks up R for each job and passes R + metadata to 'cb'
 * with 'arg'.
 */
int schedutil_hello (schedutil_t *util, schedutil_hello_cb_f *cb, void *arg);

#ifdef __cplusplus
}
#endif

#endif /* !_FLUX_SCHEDUTIL_HELLO_H */

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
