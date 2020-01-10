##############################################################
# Copyright 2019 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

from __future__ import print_function

import os
import sys
import logging
import argparse
import time
import pwd
from datetime import timedelta

import flux.job
import flux.constants
import flux.util
from flux.core.inner import raw

logger = logging.getLogger("flux-jobs")


def runtime(job, roundup):
    if job["t_cleanup"] > 0.0:
        t = job["t_cleanup"] - job["t_run"]
        if roundup:
            t = round(t + 0.5)
    elif job["t_run"] > 0.0:
        t = time.time() - job["t_run"]
        if roundup:
            t = round(t + 0.5)
    else:
        t = 0.0
    return t


def runtime_fsd(job, hyphenifzero):
    t = runtime(job, False)
    if t < 60.0:
        s = "%.2gs" % t
    elif t < (60.0 * 60.0):
        s = "%.2gm" % t
    elif t < (60.0 * 60.0 * 24.0):
        s = "%.2gh" % t
    else:
        s = "%.2gd" % t
    if hyphenifzero and s == "0s":
        return "-"
    return s


def runtime_hms(job):
    t = runtime(job, True)
    return str(timedelta(seconds=t))


def statetostr(job, singlechar):
    return raw.flux_job_statetostr(job["state"], singlechar).decode("utf-8")


def output_format(fmt, jobs):
    for job in jobs:
        s = fmt.format(
            id=job["id"],
            userid=job["userid"],
            username=pwd.getpwuid(job["userid"]).pw_name,
            priority=job["priority"],
            state=statetostr(job, False),
            state_single=statetostr(job, True),
            job_name=job["job-name"],
            task_count=job["task-count"],
            t_submit=job["t_submit"],
            t_depend=job["t_depend"],
            t_sched=job["t_sched"],
            t_run=job["t_run"],
            t_cleanup=job["t_cleanup"],
            t_inactive=job["t_inactive"],
            runtime=runtime(job, False),
            runtime_fsd=runtime_fsd(job, False),
            runtime_fsd_hyphen=runtime_fsd(job, True),
            runtime_hms=runtime_hms(job),
        )
        print(s)


@flux.util.CLIMain(logger)
def main():
    parser = argparse.ArgumentParser()
    # -a equivalent to -s "pending,running,inactive" and -u set to userid
    parser.add_argument(
        "-a", action="store_true", help="List all jobs for current user"
    )
    # -A equivalent to -s "pending,running,inactive" and -u set to "all"
    parser.add_argument("-A", action="store_true", help="List all jobs for all users")
    parser.add_argument(
        "-c",
        "--count",
        type=int,
        metavar="N",
        default=1000,
        help="Limit output to N jobs(default 1000)",
    )
    parser.add_argument(
        "-s",
        "--states",
        type=str,
        metavar="STATES",
        default="pending,running",
        help="List jobs in specific states(pending,running,inactive)",
    )
    parser.add_argument(
        "--suppress-header",
        action="store_true",
        help="Suppress printing of header line",
    )
    parser.add_argument(
        "-u",
        "--user",
        type=str,
        metavar="[USERNAME|UID]",
        default=str(os.geteuid()),
        help='Limit output to specific username or userid (Specify "all" for all users)',
    )
    parser.add_argument(
        "-o",
        "--format",
        type=str,
        metavar="FORMAT",
        help="Specify output format using Python's string format syntax",
    )

    args = parser.parse_args()

    h = flux.Flux()

    # Future optimization, reduce attrs based on what is in output
    # format, to reduce potentially large return RPC.
    attrs = [
        "userid",
        "priority",
        "state",
        "job-name",
        "task-count",
        "t_submit",
        "t_depend",
        "t_sched",
        "t_run",
        "t_cleanup",
        "t_inactive",
    ]

    if args.a:
        args.user = str(os.geteuid())
    if args.A:
        args.user = str(flux.constants.FLUX_USERID_UNKNOWN)
    if args.a or args.A:
        args.states = "pending,running,inactive"

    if args.user == "all":
        userid = flux.constants.FLUX_USERID_UNKNOWN
    else:
        try:
            userid = pwd.getpwnam(args.user).pw_uid
        except KeyError:
            try:
                userid = int(args.user)
            except ValueError:
                print("invalid user specified", file=sys.stderr)
                sys.exit(1)

    flags = 0
    for state in args.states.split(","):
        if state.lower() == "pending":
            flags |= flux.constants.FLUX_JOB_LIST_PENDING
        elif state.lower() == "running":
            flags |= flux.constants.FLUX_JOB_LIST_RUNNING
        elif state.lower() == "inactive":
            flags |= flux.constants.FLUX_JOB_LIST_INACTIVE
        else:
            print("Invalid state specified", file=sys.stderr)
            sys.exit(1)

    if flags == 0:
        flags |= flux.constants.FLUX_JOB_LIST_PENDING
        flags |= flux.constants.FLUX_JOB_LIST_RUNNING

    rpc_handle = flux.job.job_list(h, args.count, attrs, userid, flags)
    try:
        jobs = flux.job.job_list_get(rpc_handle)
    except EnvironmentError as e:
        print("{}: {}".format("rpc", e.strerror), file=sys.stderr)
        sys.exit(1)

    if args.format:
        output_format(args.format, jobs)
    else:
        fmt = (
            "{id:>18} {username:<8.8} {job_name:<10.10} {state:<8.8} "
            "{task_count:>6} {runtime_fsd_hyphen}"
        )
        if not args.suppress_header:
            s = fmt.format(
                id="JOBID",
                username="USER",
                job_name="NAME",
                state="STATE",
                task_count="NTASKS",
                runtime_fsd_hyphen="TIME",
            )
            print(s)
        output_format(fmt, jobs)


if __name__ == "__main__":
    main()

# vi: ts=4 sw=4 expandtab