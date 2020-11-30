#!/bin/sh

test_description='Test flux job ingest service'

. $(dirname $0)/sharness.sh

if test "$TEST_LONG" = "t"; then
    test_set_prereq LONGTEST
fi
if ${FLUX_BUILD_DIR}/t/ingest/submitbench --help 2>&1 | grep -q sign-type; then
    test_set_prereq HAVE_FLUX_SECURITY
    SUBMITBENCH_OPT_R="--reuse-signature"
fi

test_under_flux 4 kvs

flux setattr log-stderr-level 1

JOBSPEC=${SHARNESS_TEST_SRCDIR}/jobspec
Y2J="flux python ${JOBSPEC}/y2j.py"
SUBMITBENCH="${FLUX_BUILD_DIR}/t/ingest/submitbench"
RPC=${FLUX_BUILD_DIR}/t/request/rpc
SCHEMA=${FLUX_SOURCE_DIR}/src/modules/job-ingest/schemas/jobspec.jsonschema
BINDINGS_VALIDATOR=${FLUX_SOURCE_DIR}/src/modules/job-ingest/validators/validate-jobspec.py
JSONSCHEMA_VALIDATOR=${FLUX_SOURCE_DIR}/src/modules/job-ingest/validators/validate-schema.py
FAKE_VALIDATOR=${SHARNESS_TEST_SRCDIR}/ingest/fake-validate.sh
BAD_VALIDATOR=${SHARNESS_TEST_SRCDIR}/ingest/bad-validate.sh

DUMMY_EVENTLOG=test.ingest.eventlog

DUMMY_MAX_JOBID=16777216000000
DUMMY_FLUID_TS=1000000

test_valid ()
{
    local rc=0
    for job in $*; do
        cat ${job} | ${Y2J} | ${SUBMITBENCH} - || rc=1
    done
    return ${rc}
}

test_invalid ()
{
    local rc=0
    for job in $*; do
        cat ${job} | ${Y2J} | ${SUBMITBENCH} - && rc=1
    done
    return ${rc}
}

# load|reload ingest modules (in proper order) with specified arguments
ingest_module ()
{
    cmd=$1; shift
    flux module ${cmd} job-ingest $* &&
    flux exec -r all -x 0 flux module ${cmd} job-ingest $*
}

test_expect_success 'job-ingest: convert basic.yaml to json' '
	${Y2J} <${JOBSPEC}/valid/basic.yaml >basic.json
'

test_expect_success 'job-ingest: convert use_case_2.6.yaml to json' '
	${Y2J} <${JOBSPEC}/valid/use_case_2.6.yaml >use_case_2.6.json
'

test_expect_success 'job-ingest: submit fails without job-ingest' '
	test_must_fail flux job submit basic.json 2>nosys.out
'

test_expect_success 'job-ingest: load job-manager-dummy module' '
	flux module load \
		${FLUX_BUILD_DIR}/t/ingest/.libs/job-manager-dummy.so
'

test_expect_success 'job-ingest: job-ingest fails with bad option' '
	test_must_fail flux module load job-ingest badopt=xyz
'

test_expect_success 'job-ingest: job-ingest fails with bad validator path' '
	test_must_fail flux module load job-ingest validator=/noexist
'

test_expect_success 'job-ingest: load job-ingest' '
	ingest_module load \
		validator=${BINDINGS_VALIDATOR}
'

test_expect_success HAVE_JQ 'job-ingest: dummy job-manager has expected max_jobid' '
	max_jobid=$(${RPC} job-manager.getinfo | jq .max_jobid) &&
	test ${max_jobid} -eq ${DUMMY_MAX_JOBID}
'

test_expect_success HAVE_JQ 'job-ingest: max_jobid <= rank 0 FLUID timestamp' '
	ts0=$(${RPC} job-ingest.getinfo | jq .timestamp) &&
	test ${DUMMY_FLUID_TS} -le ${ts0}
'

test_expect_success HAVE_JQ 'job-ingest: rank 0 FLUID timestamp <= rank 1' '
	ts1=$(flux exec -r1 ${RPC} job-ingest.getinfo | jq .timestamp) &&
	test ${ts0} -le ${ts1}
'

test_expect_success 'job-ingest: YAML jobspec is rejected' '
	test_must_fail flux job submit ${JOBSPEC}/valid/basic.yaml
'

test_expect_success 'job-ingest: jobspec stored accurately in KVS' '
	jobid=$(flux job submit basic.json) &&
	kvsdir=$(flux job id --to=kvs $jobid) &&
	flux kvs get --raw ${kvsdir}.jobspec >jobspec.out &&
	test_cmp basic.json jobspec.out
'

test_expect_success 'job-ingest: job announced to job manager' '
	jobid=$(flux job submit --priority=10 basic.json | flux job id) &&
	flux kvs eventlog get ${DUMMY_EVENTLOG} \
		| grep "\"id\":${jobid}" >jobman.out &&
	grep -q "\"priority\":10" jobman.out &&
	grep -q "\"userid\":$(id -u)" jobman.out
'

test_expect_success 'job-ingest: submit event logged with userid, priority' '
	jobid=$(flux job submit --priority=11 basic.json) &&
	kvspath=`flux job id --to=kvs ${jobid}` &&
	flux kvs eventlog get ${kvspath}.eventlog |grep submit >eventlog.out &&
	grep -q "\"priority\":11" eventlog.out &&
	grep -q "\"userid\":$(id -u)" eventlog.out
'

test_expect_success 'job-ingest: instance owner can submit priority=31' '
	flux job submit --priority=31 basic.json
'

test_expect_success 'job-ingest: priority range is enforced' '
	test_must_fail flux job submit --priority=32 basic.json &&
	test_must_fail flux job submit --priority="-1" basic.json
'

test_expect_success 'job-ingest: guest cannot submit priority=17' '
	! FLUX_HANDLE_ROLEMASK=0x2 flux job submit --priority=17 basic.json
'

test_expect_success 'job-ingest: valid jobspecs accepted' '
	test_valid ${JOBSPEC}/valid/*
'

test_expect_success 'job-ingest: invalid jobs rejected' '
	test_invalid ${JOBSPEC}/invalid/*
'

test_expect_success NO_ASAN 'job-ingest: submit job 100 times' '
	${SUBMITBENCH} -r 100 use_case_2.6.json
'

test_expect_success NO_ASAN 'job-ingest: submit job 100 times, reuse signature' '
	${SUBMITBENCH} ${SUBMITBENCH_OPT_R} -r 100 use_case_2.6.json
'

test_expect_success HAVE_FLUX_SECURITY 'job-ingest: submit user != signed user fails' '
	! FLUX_HANDLE_USERID=9999 flux job submit basic.json 2>baduser.out &&
	grep -q "signer=$(id -u) != requestor=9999" baduser.out
'

test_expect_success HAVE_FLUX_SECURITY 'job-ingest: non-owner mech=none fails' '
	! FLUX_HANDLE_ROLEMASK=0x2 flux job submit \
		--sign-type=none basic.json 2>badrole.out &&
	grep -q "only instance owner" badrole.out
'

test_expect_success 'submit request with empty payload fails with EPROTO(71)' '
	${RPC} job-ingest.submit 71 </dev/null
'

test_expect_success 'job-ingest: test validator with version 1 enforced' '
	ingest_module reload \
		validator=${BINDINGS_VALIDATOR} validator-args="--require-version,1"
'

test_expect_success 'job-ingest: v1 jobspecs accepted with v1 requirement' '
	test_valid ${JOBSPEC}/valid_v1/*
'

test_expect_success 'job-ingest: test non-python validator' '
	ingest_module reload \
		validator=${FAKE_VALIDATOR}
'

test_expect_success 'job-ingest: submit succeeds with non-python validator' '
    flux job submit basic.json
'

test_expect_success 'job-ingest: test python jsonschema validator' '
	ingest_module reload \
		validator=${JSONSCHEMA_VALIDATOR} validator-args=--schema,${SCHEMA}
'

test_expect_success 'job-ingest: YAML jobspec is rejected by jsonschema validator' '
	test_must_fail flux job submit ${JOBSPEC}/valid/basic.yaml
'

test_expect_success 'job-ingest: valid jobspecs accepted by jsonschema validator' '
	test_valid ${JOBSPEC}/valid/*
'

test_expect_success 'job-ingest: invalid jobs rejected by jsonschema validator' '
	test_invalid ${JOBSPEC}/invalid/*
'

test_expect_success 'job-ingest: validator unexpected exit is handled' '
	ingest_module reload \
		validator=${BAD_VALIDATOR} &&
	test_must_fail flux job submit basic.json 2>badvalidator.out &&
	grep "unexpectedly exited" badvalidator.out
'

test_expect_success 'job-ingest: remove modules' '
	flux module remove job-manager &&
	flux exec -r all flux module remove job-ingest
'

test_done
