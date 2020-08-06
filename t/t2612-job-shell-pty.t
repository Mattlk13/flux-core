#!/bin/sh
#
test_description='Test flux-shell pty support'

. `dirname $0`/sharness.sh

test_under_flux 4 job

# an existing FLUX_TERMINUS_SESSION will interfere with tests below
unset FLUX_TERMINUS_SESSION

FLUX_SHELL="${FLUX_BUILD_DIR}/src/shell/flux-shell"

INITRC_TESTDIR="${SHARNESS_TEST_SRCDIR}/shell/initrc"
INITRC_PLUGINPATH="${SHARNESS_TEST_DIRECTORY}/shell/plugins/.libs"
runpty="${SHARNESS_TEST_SRCDIR}/scripts/runpty.py -f asciicast"
waitfile="${SHARNESS_TEST_SRCDIR}/scripts/waitfile.lua"

shell_leader_rank() {
    flux job wait-event -f json -p guest.exec.eventlog $1 shell.init | \
        jq '.context["leader-rank"]'
}
shell_service() {
    flux job wait-event -f json -p guest.exec.eventlog $1 shell.init | \
        jq -r '.context["service"]'
}
terminus_jobid() {
    test $# -lt 2 && return 1
    local jobid=$1
    local cmd=$2
    shift 2
    flux terminus $cmd \
        -r $(shell_leader_rank $jobid) \
        -s $(shell_service $jobid).terminus "$@"
}

test_expect_success HAVE_JQ 'pty: submit a job with pty' '
	id=$(flux mini submit --flags waitable -o pty bash) &&
	terminus_jobid $id list &&
	flux job cancel ${id} &&
	test_must_fail flux job wait $id
'
test_expect_success HAVE_JQ,NO_CHAIN_LINT 'pty: run job with pty' '
	printf "PS1=XXX:\n" >ps1.rc
	id=$(flux mini submit -o pty bash --rcfile ps1.rc | flux job id)
	$runpty -o log.job-pty flux job attach ${id} &
	pid=$! &&
	terminus_jobid ${id} list &&
	$waitfile -t 20 -vp "XXX:" log.job-pty &&
	printf "flux job id \$FLUX_JOB_ID\r" | terminus_jobid ${id} attach -p 0 &&
	$waitfile -t 20 -vp ${id} log.job-pty &&
	printf "exit\r\n" | terminus_jobid ${id} attach -p 0 &&
	$waitfile -t 20 -vp exit log.job-pty &&
	wait $pid
'
# Interactively attach to pty many times to ensure no hangs, etc.
#
# Ensure `flux job attach` has a chance to attach to pty before `stty size`
#  is executed by blocking and waiting for `test-ready` key to appear in
#  the job's kvs namespace.
#
# Test driver waits for "shell.start" event to be output by `flux-job attach`
#  which guarantees we're attached to pty, since this is done synchronously
#  in flux-job attach after the `shell.init` event.
#
# Finally, put the test-ready key into kvs, unblocking the test job so
#  it can proceed to print result.
#
test_expect_success NO_CHAIN_LINT 'pty: interactive job with pty' '
	cat <<-EOF >stty-test.sh &&
	#!/bin/sh
	# Do not continue until test is ready:
	flux kvs get --waitcreate --count=1 --label test-ready
	stty size
	EOF
	chmod +x stty-test.sh &&
	for i in `seq 0 10`; do
	    id=$(flux mini submit -n1 -o pty ./stty-test.sh)
	    $runpty -w 80x25 -o log.interactive.$i \
	        flux job attach --show-exec ${id} &
	    pid=$! &&
	    $waitfile -t 20 -vp "shell.start" log.interactive.$i &&
	    flux kvs put -N $(flux job namespace ${id}) test-ready=${i} &&
	    $waitfile -t 20 -vp "25 80" log.interactive.$i
	done
'
test_done
