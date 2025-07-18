flux-core version 0.76.0 - 2025-07-01
-------------------------------------

## New Features
 * add `flux multi-prog` for MPMD support (#6881)
 * add flux module remove --cancel option (#6894)
 * jobtap: add `flux_jobtap_call()` (#6897)
 * broker: add broker.getenv RPC (#6891)
 * resource: log drain/undrain events (#6882)
 * add `FLUX_JOB_ID_PATH` to the job environment (#6885)
 * idset: add round-robin allocation flag (#6884)
 * content: remove checkpoint key (#6863)
 * broker: let a upmi client relay extra bootstrap info (#6879)
 * add more arguments to Python `JobspecV1` factory methods (#6858)
 * python: add `unbuffered` getter and setter to `Jobspec` class (#6875)

## Fixes
 * cron: fix potential module hang in `cronodate_next()` (#6889)
 * python: unify behavior of `Jobspec` `getattr` and `setattr` methods
   (#6874)
 * python: rename `Jobspec` stdio properties to match CLI options (#6877)
 * Python: Jobspec repr change (#6870)
 * flux-fsck: fix error message output (#6867)

## Build/CI/Testsuite/Documentation
 * t: fix race in job info update watch test (#6871)
 * testsuite: reduce parallelism in `t4000-issues-test-driver.t` for test
   reliability (#6868)

flux-core version 0.75.0 - 2025-06-02
-------------------------------------

## New Features
 * support `singleton` dependency scheme in job submission (#6827)
 * broker: add 'mincrit' topology (#6808)
 * add method to request extra brokers on node 0 (#6822)
 * use local ipc for TBON when brokers are co-located (#6823)
 * Support new flux-fsck tool to check integrity of content store (#6787, #6828)
 * kvs: support initial-rootref option (#6775)
 * flux-kvs: support sync command (#6818)
 * flux-jobs: output message if results truncated (#6802)

## Fixes
 * fix compile issue with GCC13.2.0 on Zen4 (#6855)
 * content-s3: remove module (#6840)
 * content-sqlite: deprecate version 0 checkpoint (#6839)
 * require Lua `posix` in `configure` and document some tips for running
   the testsuite (#6836)
 * broker: avoid malloc in event distribution critical path (#6811)
 * flux-module: require argument for flux module stats (#6821)
 * remove restriction that taskmaps cannot assign differing counts of tasks
   to nodes in a job (#6815)

## Cleanup
 * broker: document/clean up event publishing code (#6809)

## Build/CI/Testsuite/Documentation
 * doc: remove `jinja2<3.1`, `sphinx<6.0.0` requirements (#6844)
 * testsuite: fix potential failure in `t2602-job-shell.t` (#6841)
 * doc: correct `flux_future_set_reactor()` prototype (#6830)
 * doc: update glossary with parallel program terms + trivial broker cleanup
   (#6829)
 * kvs: add rst format to inline docs (#6503)
 * testsuite: add one missing test and fix failing test in ci (#6817)
 * doc: add workflows page (#6803)
 * doc: add note to README about Sphinx debugging (#6805)


flux-core version 0.74.0 - 2025-05-06
-------------------------------------

## New Features

 * job-manager: add `stop-queues-on-restart` configuration parameter (#6801)
 * job-exec: only adjust timelimit for jobs when start delay exceeds a
   configurable percent of job duration (#6791)
 * python: add jobspec `__repr__` method (#6778)
 * enable daily backups of system KVS (#6763)
 * drop content.backing-module=none support (#6764)
 * content: require backing store for checkpoint (#6255)

## Fixes

 * job-manager: avoid starting jobs in a stopped anonymous queue on restart
   (#6792)
 * jobtap: fix issues at restart with dependent jobs (#6790)
 * libsubprocess: remove confusing `flux_future_push: Invalid argument`
   error message in bulk-exec (#6783)
 * sdmon: only match job related systemd units (#6762)
 * task-exit: improve clarity of log message (#6760)
 * libeventlog: cleanup failed batch in eventlogger (#6756)
 * libeventlog: fix uncalled error callback (#6755)

## Cleanup

 * Misc cleanups (#6744)

## Build/CI/Testsuite/Documentation

 * doc: correct description of job termination in flux-config-exec(5) (#6793)
 * doc: add troubleshooting guide (#6747)
 * Update README.md with pre-built container information (#6767)
 * testsuite: fix flux-proxy TMPDIR test (#6752)
 * libeventlog: fix flaky eventlog formatter tests (#6748)
 * t: fix potential race in backgrounded processes (#6746)


flux-core version 0.73.0 - 2025-04-01
-------------------------------------

## New Features
 * execute rc2 in same process group as broker and send `SIGHUP` before
   broker exit (#6740)
 * flux-job: forward some signals to jobs in `flux job attach` (#6734)
 * support resource counts in `flux queue list` output, display effective
   limits by default (#6726)
 * python: add `QueueList` class for access to queue config, limits, status
   and resource counts (#6723)
 * export `FLUX_JOB_RANKS` to housekeeping scripts (#6728)
 * set `exit-timeout=none` by default in `flux alloc` and `flux batch`
   (#6702)
 * accept target queue as non-option parameter in most `flux queue`
   subcommands (#6706)

## Fixes
 * flux-proxy: fall back to `/tmp` if `TMPDIR` is invliad (#6741)
 * libjob: fixes for rabbit jobspec (#6727)
 * job-info: index watchers for quick cancels (#6720)
 * kvs-watch: index watchers to avoid search iterations (#6719)
 * kvs-watch: subscribe/unsubscribe to events asynchronously (#6701)
 * shell: clear signal mask and handlers in shell for plugins that call
   `fork(2)` (#6716)
 * fix failure to signal a process group after the process group leader exits
   (#6714)
 * python: make FluxExecutor catch submission errors (#6700)
 * sdmon: reconnect when bus is lost (#6697)
 * Remove version check from libjob and change to pass with warning in shell
   (#6682)

## Cleanup
 * flux-queue: correct some comment grammar (#6736)

## Build/CI/Testsuite/Documentation
 * ci: update pre-commit and fix spurious flake8 error (#6739)
 * t: add checkpoint integrity test (#6733)
 * test: fix checks-lib script (#6731)
 * testsuite: fix non-bourne shell test failure (#6686)
 * github: upgrade codecov-action to v5 (#6688)


flux-core version 0.72.0 - 2025-03-04
-------------------------------------

## New Features
 * add `FLUX_JOB_RANKS` environment variable to prolog and epilog (#6670)
 * avoid scheduling jobs on compute nodes that are not cleaned up (#6616)
 * flux-hostlist: add `-F, --find=HOSTS` option (#6671)
 * raise non-fatal exception on epilog failure (#6669)
 * shrink available resource set for jobs that lose nodes (#6652, #6676)
 * add support for undrain reason (#6659)
 * flux-jobs: filter against all users with --include (#6646)
 * add `ResourceJournalConsumer` and `EventLogFormatter` classes and use
   them to standardize `flux resource eventlog`  (#6614, #6635)
 * add resource journal (#6586, #6633)
 * heartbeat: add optional timeout (#6679)

## Fixes
 * improve error message on invalid formatting of shell options (#6680)
 * fix prolog/epilog timeout handling (#6677)
 * sdexec: add stop timeout to handle unkillable processes (#6666, #6673)
 * libsubprocess: fix error path bug (#6663)
 * improve management of systemd stop timeout to avoid SIGKILL while making
   progress (#6661)
 * fix non-ascii character handling in `flux resource eventlog` (#6650)
 * fix duplicated output in `flux run` (#6649)
 * flux-version: report hwloc.api=x.y.z instead of hwloc=x.y.z (#6639)
 * fix incorrect combination of lines in `flux resource status` for
   expandable fields that differ past the minimum width (#6638)
 * log helpful info when shutdown is stuck (#6623)
 * etc: update bash completions (#6618)
 * Require exactly one of total or `per_slot` be specified in jobspec
   tasks.count (#6605)
 * flux-job: do not override `-L, --color` when `-H, --human` is used in
   `eventlog` and `wait-event` subcommands (#6612)
 * flux-queue: do not default to --all with configured queues (#6681)

## Cleanup
 * rename broker quorum and shutdown warn period attributes (#6660)
 * libsubprocess: cosmetic cleanup (#6642)
 * kvs: move `wait_version` API into kvsroot API (#6628)
 * kvs: remove dead code (#6622, #6611, #6604)
 * kvs: misc cleanup (#6627, #6615)

## Build/CI/Testsuite/Documentation
 * gitignore: ignore PLY autogenerated parsetab.py (#6664)
 * testsuite: fix flaky test for handling of invalid `tbon.topo` (#6657)
 * doc: add job classifications to glossary (#6647)
 * doc: expand job termination documentation in `flux-config-exec(5)` (#6640)
 * ci: ensure unit tests are run during coverage (#6617)
 * arm64 Docker cleanup (#6575)
 * github: update typos check ci action


flux-core version 0.71.0 - 2025-02-04
-------------------------------------

## New Features
 * do not export `PYTHONPATH` by default in Flux commands (#6597)
 * python: support relative and absolute path-like targets in jobid URI
   resolver (#6562)
 * libflux: support ancestor paths as alternative to URI in `flux_open(3)`
   and `flux_open_ex(3)` (#6573)
 * broker: add year to log timestamps (#6574)
 * kvs: add transaction stats (#6556)
 * support `afterexcept` dependency scheme (#6566)
 * add `FLUX_ENCLOSING_ID` to initial program environment for instances
   with a `jobid` broker attribute (#6558)
 * add `flux --root` option (#6557)
 * shell: refactor output plugin and enable per-node/task output files
   (#6539)

## Fixes
 * fix housekeeping reconfiguration problem (#6598)
 * improve scalability of `flux overlay errors` (#6593)
 * shell: fix incorrect assignment of shell rank ids when broker ranks
   appear unordered in R (#6584)
 * job-exec: fix confusing "job shell exec error" log message (#6579)
 * shell: fix lost log messages during initialization (#6578)
 * broker: ensure `parent-uri` and `parent-kvs-namespace` are only set for
   jobs (#6561)
 * ensure only jobs that entered the RUN state can satisfy
   `--dependency=afterany|afternotok` (#6565)
 * prevent log message truncation (#6554)
 * flux-exec: fix credits list update race (#6544)

## Cleanup
 * kvs: remove unused internal transaction request API code (#6595)
 * kvs: remove fence support (#6592)
 * kvs: misc minor cleanups (#6591)
 * libflux: drop child watchers and the `FLUX_REACTOR_SIGCHLD` flag (#6543)

## CI/Testsuite/Documentation

 * doc: reword initial program section of `flux-environment(7)` (#6601)
 * testsuite: fix flaky test in t0019-tbon-config.t (#6563)
 * libsubprocess/test: Fix racy fd count in test (#6553)
 * build: remove unnecessary `check-prep` target (#6550)


flux-core version 0.70.0 - 2025-01-07
-------------------------------------

## New Features
 * add `resource.rediscover` config key to force rediscovery of subinstance
   resources (#6541)
 * allow mustache templates in job environment variables (#6506)
 * shell: make shell tasks available in `shell.init` callback (#6537)
 * reactor: improve reactor referencing API (#6534)
 * shell: support more mustache tags (#6525)
 * kvs-watch: support `FLUX_KVS_STREAM` flag (#6523)
 * support specification of file permissions in `--add-file` submission
   option (#6505)
 * add `flux.conf_builtin.conf_builtin_get()` to give Python access to
   compiled-in config values (#6486)
 * jobtap: add `flux_jobtap_jobspec_update_id_pack()` (#6500)
 * kvs-watch: only fetch new data for appends (#6444)
 * flux-hostlist: allow idset argument to `--nth` and `--exclude` options
   (#6478)

## Fixes
 * shell: minor fixes for shell pty plugin (#6538)
 * job-info: stream events even if job is inactive (#6518)
 * flux-hostlist: fix `available` hostlist (#6536)
 * fix possible truncation of strings with "expandable" output fields (#6533)
 * flux-kvs: improve eventlog error messages (#6531)
 * shell: fix signal plugin to work with Fluxion scheduler (#6522)
 * libschedutil: use preprocessor symbols for flags (#6520)
 * support partially allocated jobs across scheduler reload (#6445)
 * job-exec: improve drain message for unkillable procs (#6515)
 * fix macos portability issues (#6508, #6479, #6476, #6473)
 * fix handling of `\n` in submission cli `--add-file=` option (#6504)
 * job-info: avoid error response on failed rpc (#6502)
 * libflux: update API to use `size_t` where appropriate (#6467)
 * build: use -Wl,--gc-sections when appropriate (#6497)
 * shell: ignore SIGPIPE (#6489)

 * resource: only read `resource.scheduling` config on rank 0 (#6482)
## Cleanup
 * libflux: refactor reactor/watcher implementation (#6494)
 * modernize code formatting (#6481)
 * drop caliper support (#6475)

## CI/Testsuite/Documentation
 * minor man page improvements (#6519)
 * ci: add macos builder (#6499, #6509)
 * testsuite: don't assume /bin/true and /bin/false (#6507)


flux-core version 0.69.0 - 2024-12-03
-------------------------------------

## New Features
 * add flux module stats `--rusage=[self|children|thread]` optional argument
   (#6471)
 * add `-S, --setattr` and `-c, --config-path` options directly to `flux
   start` (#6452)
 * job-ingest: improve cleanup and stats output (#6438)
 * libflux: add `flux_watcher_is_active()` (#6436)
 * indicate held jobs in the` INFO` column of `flux jobs` output (#6430)
 * recursively update instead of replacing tables when loading Flux
   configuration (#6424)
 * convert ISO timestamp output from UTC to local time + offset in `flux
   dmesg` and eventlog commands (#6423)
 * support flux uri --wait JOBID (#6443)
 * skip "empty" lines of output in `flux resource list` with `--skip-empty`
   or `--include` (#6460)

## Fixes
 * libfileref: fix segfault for files >2G (#6462)
 * fix macos portability issues (#6454, #6468)
 * fix multiple issues in the `flux job attach` statusline (#6442)
 * librlist: avoid unnecessary hwloc dependencies (#6450)
 * python: call shutdown() on executor in job validator (#6435)
 * increase default prolog kill-timeout from 10s to 1m (#6431)
 * job-manager/history: optimize list insertion (#6422)

## Cleanup
 * kvs-watch: misc cleanup (#6458)
 * build: misc cleanup (#6451)
 * job-manager: clean up queue code (#6448)
 * remove `flux-perilog-run` (#6447)

## CI/Testsuite/Documentation
 * doc: improve `--include` documentation in flux-resource(1) (#6459)
 * doc: improve housekeeping documentation (#6425)
 * doc: launch jobs with systemd in the admin guide (#6427)


flux-core version 0.68.0 - 2024-11-06
-------------------------------------

This release requires flux-security >= 0.13.0 as the IMP signal handling
strategy has changed per RFC 15.

## New Features
 * update multi-user signaling to track flux-security 0.13.0 IMP changes
   (#6408)
 * add cleanup timeout for systemctl stop flux on rank 0 (#6397)
 * flux-exec: use subprocess credits to avoid overflowing stdin buffers
   (#6370)
 * libsubprocess: support flow control on writes via credits (#6353)
 * python: usability improvements for `JournalConsumer` class (#6390)
 * python: add `flux.job.JournalConsumer` class with a simplified interface
   for job manager journal consumers (#6386)
 * support `sort:` prefix for format strings and `--sort` option to `flux
   jobs` (#6380)
 * flux-housekeeping: add `-i, --include=TARGETS` option to `flux
   housekeeping list` (#6382)
 * show response result in message traces (#6359)
 * libsubprocess: invert SETPGRP flag logic (#6082)
 * add --full option to display payloads in message tracing programs (#6347)


## Fixes
 * libsubprocess: close extra file descriptors (#6416)
 * resolve perilog plugin issue that lets a job start after prolog timeout
   when cancellation fails (#6412)
 * frobnicator: allow queue-specific defaults to override global defaults
   (#6403)
 * sdexec: set KillMode=process SendSIGKILL=no for multi-user jobs (#6402)
 * broker: detect mismatched bootstrap.hosts configuration (#6393)
 * libsubprocess: take reference on callbacks (#6384)
 * python: cleanup, fixes, and unit tests for flux.util.OutputFormat (#6374)
 * libsubprocess: misc fixes (#6379)
 * sched-simple: improve unsupported resource type exception (#6372)
 * libsubprocess: ensure bulk-exec output is terminated (#6368)
 * libsubprocess: check bufsize is > 0 (#6365)
 * kvs: fix whitespace issues (#6356)
 * allow project to be built with NDEBUG (#6355)
 * systemd: make scripts fail if systemctl start does (#6346)
 * improve policy/queues config error messages (#6339)
 * make flux resource drain -o long reason expandable (#6338)

## Cleanup
 * job-archive: remove module (#6378)

## CI/Testsuite/Documentation
 * broker/test: avoid race in zeromq cleanup (#6405)
 * docker: add missing tag of flux-core el8 image (#6401)
 * doc: add debugging notes (#6369)
 * doc: update link to flux-accounting guide (#6373)
 * flux-jobs(1): document unlimited --count value (#6364)
 * testsuite: add --raw-response opt to rpc test program (#6342)
 * testsuite: improve test_must_fail_or_be_killed function (#6343)
 * docs: rfc flux-config-bootstrap diagram (#6411)


flux-core version 0.67.0 - 2024-10-01
-------------------------------------

## New Features
 * include hostnames in flux resource undrain error message (#6335)
 * libsubprocess: increase output watcher priority (#6317)
 * libflux: support modifying watcher priority via
   `flux_watcher_set_priority()` (#6316)
 * add `--force` option to `flux resource undrain` (#6312)
 * autoconf: support python 3.12 (#6303)
 * flux-bulksubmit: support `{}`  in more options like `--cwd=`, `--signal=`,
   `--taskmap=`, etc. (#6299)
 * add flux-lptest (#6301)
 * broker: use enclosing instance tbon.interface-hint unless overridden
   (#6283)
 * shell/oom: log more detail when tasks are killed (#6289)
 * support expandable width output formats and use them in `flux resource
   list` to avoid truncation of queue field (#6284)
 * python: update Flux handle location in validator plugin API (#6282)
 * broker: call `PMI_Abort()` if something goes wrong during PMI bootstrap
   (#6279)
 * add tbon.interface-hint broker attribute / configuration key with CIDR
   network support (#6277)
 * support configuration of require-instance and other job
   validator/frobnicator plugins in broker config TOML (#6305)
 * validator: allow configurable minimum job size in the require-instance
   validator plugin (#6258)
 * display nodes in housekeeping in `flux resource status` (#6263)
 * shell: output stdio output size warning (#6274)
 * python: add `conf_get()` convenience method to `flux.Flux` handle (#6267)
 * limit output to 1G in single user instances (#6268)

## Fixes
 * systemd: improve housekeeping drain message (#6334)
 * perilog: never send SIGKILL to prolog/epilog, drain active nodes after
   kill-timeout instead (#6330)
 * perilog: fix kill of prolog/epilog when using IMP (#6324)
 * perilog: fix `FLUX_JOB_USERID` in epilog after canceled prolog (#6320)
 * flux job info: improve error messages (#6331)
 * libsubprocess: fix bulk-exec reporting of active ranks (#6326)
 * libsubprocess: do not spin on large lines (#6281)
 * configure: add check for valid version (#6276)
 * etc: minor improvements for bash completions (#6332)
 * perilog: ensure default prolog timeout matches documentation (#6270)
 * hostlist: remove allocations in `hostrange_find` (#6259)

## CI/Testsuite/Documentation
 * libsubprocess: add extra documentation (#6307)
 * format: fix clang-format file (#6280)
 * doc: python: fix JobList default documentation (#6309)
 * doc: add dash to flux-job(1) manpage (#6313)
 * doc: add warning about Python argparse in flux-jobs(1) (#6285)
 * doc: fix typo in `FLUX_IPADDR_INTERFACE` entry in `flux-environment(7)`
   (#6271)
 * doc: update admin guide for systemd perilog (#6261)
 * doc: add warning about stdio paths in submission cli man pages (#6333)


flux-core version 0.66.0 - 2024-09-03
-------------------------------------

## New Features
 * support display of `allocated` nodes in `flux resource status` (#6253)
 * support `resource.scheduling` in config TOML to amend configured R with
   a scheduling key (#6252)
 * perilog: support direct execution of prolog/epilog on job ranks for
   better performance (#6235)
 * flux-hostlist: change default source from `local` to `stdin` (#6246)
 * support short option `-S` for `--setattr=` in job submission commands
   (#6238)
 * content-sqlite: set sqlite pragmas with flux-config (#6222)
 * content-sqlite: add sqlite config to module stats (#6221)
 * content: allow content.flush to return errors (#6175)
 * flux-jobs: add `-i, --include=HOSTS|RANKS` option (#6209)
 * add flux module trace (#6203, #6206)
 * include offline nodes in flux overlay errors output (#6201)

## Fixes
 * run epilog even if job prolog is failed or is canceled (#6249)
 * progress: ensure we don't access outside style (#6250)
 * housekeeping: deprecate use-systemd-config option (#6241)
 * Fix handling of `--` used to separate options from command in `flux
   submit`, `run`, and `batch` (#6234)
 * libsubprocess: minor improvements to bulk-exec (#6233)
 * update feasibility RPCs to conform to RFC 27 (#6223)
 * libev: fix memory fence on i386 under valgrind (#6224)
 * job-manager: skip housekeeping for alloc-bypass jobs (#6219)
 * cron: use `posix_spawn()` for cron tasks when possible (#6214)
 * kvs: correct code logic about what is an append (#6210)
 * avoid idset leak when nodes leave the broker.online group (#6198)
 * kvs: correct transaction-merge option parsing (#6204)

## Cleanup
 * content-sqlite: misc cleanup (#6220)

## CI/Testsuite/Documentation
 * doc: add dependency example (#6226)
 * mergify: use updated keys before deprecation bites (#6225)
 * testsuite: fix racy flux job attach test (#6212)
 * matrix: remove fedora40 arm again (#6200)


flux-core version 0.65.0 - 2024-08-06
-------------------------------------

## New Features
 * job-manager: add final flag to sched.free (#6197)
 * add `-B, --bank=` option to submission commands (#6195)
 * add flux overlay trace (#6174)
 * enhance `flux queue list` with `-q, --queue=` option and ability to
   report queue enabled/started status (#6185)
 * support multiple queues in `flux jobs`, `pgrep`, and `pkill` `-q,
   --queue` option (#6149)
 * accept `-q` for `--queue` in `flux-jobs`, `pgrep`, and `pkill`
   (#6142)
 * resource: mark torpid nodes down for scheduling instead of draining them
   (#6131)
 * job-exec: add timeout for job start barrier (#6140)
 * job-exec: fix per-job `active_ranks` module stats (#6138)
 * job-exec: fix job refcount leaks (#6134)
 * drain nodes with unkillable tasks after configurable retries (#6101)
 * move bad KVS job entries to a lost+found directory (#6139)
 * add error text to down nodes in flux overlay status (#6111)
 * job-manager: ignore bad KVS eventlogs (#6129)
 * add message queue counts to flux module list (#6120)
 * flux-job: add `-F, --follow` option to `flux job eventlog` (#6102)
 * wait for job `clean` event by default in `flux run`  and `flux job attach`
   (#6065)
 * systemd: add prolog/epilog service units (#6040)
 * broker: work around problem with launch by some versions of mpiexec.hydra
   (#6081)
 * reject jobs submitted as user root in a multi-user instance (#6194)

## Fixes
 * improve housekeeping logging and list management (#6191)
 * add configure time check for zeromq crypto and improve broker error
   handling at cert creation (#6189)
 * resource: error message cleanup, add test for invalid exclude (#6186)
 * flux-resource: fix missing queue in `flux resource list` output for
   states with no nodes (#6180)
 * flux-resource: suppress empty lines in output (#6096)
 * kvs: return ENOSYS on unfinished requests (#6049)
 * cmd/startlog: initialize enum to non zero (#6170)
 * address RSS growth when there are excessive annotation events (#6115)
 * resource: cache resource status information to reduce broker load (#6105)
 * flux-start: accept --recovery with --test-size (#6108)
 * doc: update flux admin guide URL (#6163)
 * doc: update flux-shell(1) manpage (#6094)
 * doc: add overlay network to overview section (#6092)

## Cleanup
 * kvs: minor cleanup (#6172)
 * job-manager: silence housekeeping debug logs (#6113)
 * kvs: replace list of message with msglist (#6047)

## CI/Testsuite
 * matrix/actions: add per-target timeout-minutes (#6192)
 * t: add initial ENOSPC tests (#6127)
 * matrix: avoid overwriting env elements (#6190)
 * docker: Demote fedora40 to `x86_64` only (#6188)
 * ci: ignore merge errors and increase parallelism (#6183)
 * testsuite: fix racy test in t2406-job-exec-cleanup.t (#6187)
 * add support for new wrap options in tests and heredoc support for sharness
   (#6150)
 * actions: increase ci timeout from 90 to 120 minutes (#6181)
 * matrix: fix matrix generation for arm64 and remove redundant fedora
   (#6156)
 * Update containers: el9, fedora40, noble, add arm64 for fedora40, el9,
   noble (#6128)
 * t: add job environment to tests (#6106)


flux-core version 0.64.0 - 2024-07-02
-------------------------------------

## Experimental Features
 * job-manager: add support for housekeeping scripts with partial release
   of resources (#5818)

## New Features
 * sdexec: add stats-get RPC handler (#6071)
 * job-exec: add `active_ranks` per-job stats (#6070)
 * job-list: add support for `ranks` constraint (#6048)
 * job-list: support "hostlist" constraint to allow jobs to be filtered by
   nodes (#5656)
 * add environment variable to avoid `RTLD_DEEPBIND` when loading modules
   (#6063)

## Fixes
 * job-manager: improve sched.alloc request tracking (#6076)
 * flux-job: fix prolog refcount in attach status message #6074 
 * job-exec: fix `active_shells` count in per-job stats (#6070)
 * job-manager/prioritize: update priority before sending (#6062)
 * job-exec: do not streq() an unbuffered string (#6058)
 * python: Update default Jobspec setattr() path (#6053)
 * shell: enhance exit-timeout and exit-on-error exceptions with hostname
   (#6056)
 * libidset: improve efficiency of idset intersection (#6045)
 * etc: update bash completions (#6039, #6060)
 * flux-python: add posix-compatible shebang wrapper (#6041)
 * libsubprocess: use matchtag instead of pid for flux_subprocess_write()
   (#6013)
 * sdexec: fix use after free during teardown (#6037)
 * broker: avoid 60s delay on follower shutdown (#6034)

## CI/Testsuite
 * t: suppress hwloc 2.10 memleaks (#6061)
 * ci: convert `system` test container to run under `podman` for better
   `systemd` support (#6043)


flux-core version 0.63.0 - 2024-06-04
-------------------------------------

## New Features
 * broker: add timezone abbrev to log message timestamps (#6027)
 * add output buffering to sdexec (#6023)
 * libsubprocess: support `FLUX_SUBPROCESS_FLAGS_LOCAL_UNBUF` (#5975)
 * job-exec: add `kill-signal` and `term-signal` module parameters (#6017)
 * broker: add tunable parameters for extreme fanout configurations (#6006)
 * bypass KVS for job stdin with `--input=FILE` (#6005)
 * flux-dump: add --ignore-failed-read option (#5989)
 * libsubprocess: improve read API (#5965)
 * job-manager: journal jobspec with submit event (#5955)

## Fixes
 * improve broker messages when nodes die (#6032)
 * sdexec: don't return ENODATA without start/finish (#6033)
 * speed up `flux overlay status` on large systems (#6030)
 * librlist: ensure properties is an array when parsing resource.config entries
   (#6029)
 * resource: stop scheduler updates during shutdown (#6028)
 * libsubprocess: reduce remote input prep/check (#6002)
 * job-exec: retry kill of job tasks and job shell during job termination
   (#6014)
 * libsubprocess: protect against double EOF (#6025)
 * sdexec: avoid double EOF notification (#6022)
 * job-exec: cmdline options for signals should override config settings
   (#6020)
 * job-manager: don't log expiration update when unnecessary (#6016)
 * libsubprocess: handle remote protocol errors (#6003)
 * shell: do not fail on oom init failure (#6004)
 * shell: split up and improve efficiency of input plugin (#5998)
 * suppress confusing log messages when broker is signaled during shutdown
   (#5996)
 * broker: don't post quorum-full in non-QUORUM state (#5994)
 * shell: fix repeated output truncation warning messages (#5993)
 * don't free alloc-bypass resources when scheduler reloads (#5980)
 * flux-job: improve EPERM error message (#5971)
 * libsubprocess: demote assert to warning (#5959)
 * job-exec: do not allow guest access to the testexec implementation by
   default (#5961)

## Documentation
 * doc: misc man page cleanup (#6008)
 * doc: fix typo in flux-jobs(1) (#6001)
 * doc: add PRIORITY state to flux-jobs(1) (#5988)

## Cleanup
 * libsubprocess: minor cleanup (#5974)
 * job-manager: misc cleanup in scheduler interface (#5969)
 * libsubprocess: remove unused len parameter from `flux_subprocess_read()`
   (#5950)
 * job-exec: remove layer of stats indirection (#5947)

## CI/Testsuite
 * testsuite: fix sdexec-memlimit config update test (#6007)


flux-core version 0.62.0 - 2024-05-07
-------------------------------------

## New Features
 * job-exec: support config reload (#5913)
 * job-exec: support module stats to see current bulk-exec configuration
   (#5943)
 * do not filter caches when loading hwloc topology (#5945)
 * shell: add `hwloc.restrict` option to restrict `HWLOC_XMLFILE` to assigned
   resources (#5944)
 * cleanup resource.eventlog and handle remapped ranks (#5914)
 * resource: upgrade KVS resource.eventlog if needed (#5936)
 * flux-resource: add `-q, --queue=QUEUE` option (#5935)
 * change default tbon.topo to kary:32 (#5930)
 * flux-job: add `flux job taskmap --to=hosts` (#5941)
 * notify user of stopped queue in `flux job attach` (#5911)
 * add options for offline config file parsing (#5907)
 * flux-exec: set up systemd environment to support sdexec debugging (#5903)
 * job-list: limit constraint comparisons to avoid DoS (#5681)
 * broker/module.c: name threads (#5895)
 * suppress epilog on jobs canceled during shutdown (try no. 2) (#5894)

## Fixes
 * libsubprocess: reduce remote output prep/check (#5932)
 * job-exec: fix potential use-after-free in bulk-exec implementation (#5937)
 * broker: avoid LOST due to EHOSTUNREACH messages during shutdown (#5928)
 * fix possible use-after-free in chained future implementation (#5927)
 * job-manager: perilog: do not set working directory on subprocesses (#5922)
 * cron: fix race if sync event changed or disabled (#5908)
 * job-manager: improve handling of offline ranks in job prolog (#5910)
 * Support optional arg in -L (--color) short option (#5890)
 * alloc-check: account for resources in scheduler hello failure (#5897)
 * librlist: sort rlist before generating R (#5887)
 * properly report signaled tasks in `flux job wait` and `flux job attach`
   (#5886)

## Documentation
 * doc: convert internals docs to readthedocs (#5939)
 * doc: enhance core dump instructions in admin guide (#5926)
 * doc: add pre-flight checklist to admin guide (#5899)
 * doc: various minor flux-cron(1) fixes and improvements (#5905)

## Cleanup
 * job-exec: silence nuisance logging (#5948)
 * assorted minor source cleanup (#5918, #5924)

## CI/Testsuite
 * docker-run-checks: ensure we match user's home (#5938)
 * deps/docker: add 'time' as a dependency for tests (#5933)
 * fix potential segfault in `test_terminus.t` (#5888)
 * docker: add script help message for macOS (#5779)


flux-core version 0.61.2 - 2024-04-12
-------------------------------------

## Fixes:

 * broker: improve handling of overlay network during shutdown (#5883)
 * job-manager: canceled job need not wait for sched (#5877)
 * broker: allow patch versions to interoperate (#5879)

## Testsuite:

 * testsuite: fix `t3203-instance-recovery.t` under `fakeroot` (#5875)

flux-core version 0.61.1 - 2024-04-09
-------------------------------------

## Fixes:

 * broker: reduce log noise (#5869)
 * shutdown: unload perilog plugin during shutdown (#5871)
 * fix broker crash in `resource.status` RPC handling when excluded ranks
   are also down (#5870)
 * broker: avoid slow offline child UUID lookup (#5867)
 * resource: round timestamp of drained ranks (#5866)
 * fix more performance issues in `flux resource` (#5865)
 * resource: improve `resource.status` response time with many drained ranks
   (#5863)

## Cleanup

 * minor cleanup in job-manager journal and job-list (mostly inline docs)
   (#5850)
 * job-list: remove jobspec/R update dead code (#5853)

## Documentation

 * doc: add path to 'manpages' to conf.py (#5855)
 * doc: fix whitespace issues in admin guide (#5854)

flux-core version 0.61.0 - 2024-04-02
-------------------------------------

## New Features

 * add utility for posting manual job events if required (#5848)
 * add `--taskmap=hostfile:FILE` support (#5844)
 * make KVS garbage collection automatic (#5840)
 * add a faster way to get resource allocation status than
   sched.resource-status RPC (#5796, #5834)
 * job-manager: drop sched.free response requirement (#5786)
 * job-manager: include R in sched.free request (#5783)

## Fixes

 * flux-job: fix `wait-event -m, --match-context`(#5846)
 * eliminate duplicate KVS restart in job-list and job-manager (#5837)
 * python: return empty string on epoch time for D conversion flag (#5841)
 * configure: add missing check for python ply >= 3.9 (#5839)
 * fix handling of `Ctrl-SPACE` (`NUL`) over job ptys (e.g. under `flux
   job attach`) (#5833)
 * job-ingest: fix handling of size > 16K and reserve some FLUID generator
   IDs for future use (#5831)
 * fix `flux submit` and `bulksubmit` handling of mustache templates in
   command and args (#5817)
 * flux-resource: improve performance of `flux resource list` (#5823)
 * optimize key librlist functions to improve `flux resource list`
   performance (#5824)
 * python: fix handle barrier method to return a Future (#5825)
 * job-manager: fix infinite loop when loading builtin jobtap plugin (#5822)
 * flux-top: skip poorly formed job list records (#5821)
 * job-exec: raise fatal job exception if rank 0 job shell is lost due to
   signal (#5814)
 * job-exec: improve cleanup after lost shell events (#5813)
 * job-exec: fix potential leak of job KVS namespaces (#5805)
 * job-exec: fix rare segfault at scale due to libsubprocess use-after-free
   (#5803)
 * kvs: remove excessive logging (#5804)
 * modules/content: drop incorrect assertion (#5781)

## Documentation
 * add doc/requirements.txt to dist (#5852)
 * doc: add missing R documentation to flux-jobtap-plugins(7) (#5838)

## CI/Testsuite

 * testsuite: fix potential hang in t2812-flux-job-last.t (#5835)
 * testsuite: fix test racing with exit-timeout (#5810)
 * python: update required versions of black and mypy (#5808)
 * testsuite: fix lingering processes after `make check` (#5769)
 * github: set `kernel.randomize_va_space=0` for asan build (#5802)
 * github: run asan build on fedora36 (#5800)
 * testsuite: fix test that kills the wrong processes (#5792)
 * improve handling of lost job shells (#5780)


flux-core version 0.60.0 - 2024-03-05
-------------------------------------

Note: This release replaces the flux-filemap(1) command with an enhanced new
command called flux-archive(1).  Simple invocations of flux-filemap(1) will
continue to work for a while to enable migration to flux-archive(1).

## New Features
 * job-manager: support suppression of prolog/epilog output with
   `perilog.log-ignore` pattern list (#5772)
 * flux-job: attach: support MPIR tool launch extension (#5758)
 * add `flux job hostpids` command (#5765)
 * shell: support opening output files with `-o output.mode=append` (#5766)
 * shell: add rexec plugin (#5605)
 * shell: unset `HWLOC_COMPONENTS` with `hwloc.xmlfile` (#5759)
 * flux-job: add shorthand paths for `flux job eventlog` and `wait-event`
   (#5749)
 * flux-archive: add new command for file broadcast (#5701)
 * completions: support completion of jobids with plain `f`, support
   flux-hostlist(1) (#5745)
 * libtaskmap: support decode of raw (semicolon-delimited) taskmaps (#5735)
 * shell: make kvs output limit configurable and default single-user jobs
   to unlimited (#5732)
 * add flux-hostlist(1) (#5724)

## Fixes
 * broker: catch an improper use of groups and handle it gracefully (#5762)
 * shell: fix incorrect values returned from `flux_shell_get_rank_info(3)`
   (#5756)
 * shell: generate fatal error if `block` taskmap scheme has an argument
   (#5730)
 * do not drain ranks when job is canceled during prolog (#5742)
 * optparse: fix segfault when subcommand registration fails due to invalid
   options table (#5740)
 * fix various typos (#5761)

## Documentation
 * doc: pull in admin guide (#5763)

## Cleanup
 * fix various typos (#5761)
 * libsubprocess: minor API cleanup (#5699)
 * cmd: split flux-job into separate source files (#5747)
 * job-info: support lookup of updated jobspec, remove manual construction
   of updated R / jobspec (#5635)
 * job-info: add `JSON_DECODE` & `CURRENT` flags, deprecate
   job-info.update-lookup (#5633)

## Build/Testsuite
 * github: update deprecated actions (#5768)
 * testsuite: fix t2617 to utilize /dev/urandom (#5757)
 * testsuite: fix random-generation of kvs archive files in t0021 (#5751)


flux-core version 0.59.0 - 2024-02-06
-------------------------------------

## New Features
 * broker: add `FLUX_IPADDR_INTERFACE` to select broker network interface
   (#5707)
 * python: support interface to perform KVS watch (#5389)
 * python: support Python 3.12 (#5691)
 * broker: allow single-user rexec to rank 0 (#5677)
 * add -x option to flux-alloc and flux-batch (#5665)
 * add flux filemap get --overwrite and change the default overwrite behavior
   (#5662)
 * shell: add shell.post-init plugin callback topic between shell.init
   and first task.init (#5179)
 * pmi: prepend Flux PMI directory to `LD_LIBRARY_PATH` (#5715)
 * shell: write hwloc XML to a file with `HWLOC_XMLFILE` set with
   `-o hwloc.xmlfile` (#5721)

## Fixes
 * job-list: initialize queue stats (#5712)
 * job-ingest: fix FLUID initialization error handling to allow scaling
   beyond 16K brokers (#5710)
 * python: fix `flux-watch: TypeError: Object of type 'bytes' is not JSON
   serializable` (#5704)
 * enable encode of pty data as base64 and make `flux alloc vi` test more
   reliable (#5703)
 * librlist: workaround xml buffer size issue in some hwloc versions (#5690)
 * librlist: fix segfault when initializing topology from XML in later
   hwloc versions (#5682)
 * fix broker hang under `flux proxy` (#5680)
 * set userid to instance owner in job manager exceptions (#5675)
 * job-manager: fix duplicate posting of jobspec-update event from plugins
   (#5673)
 * broker: only set parent-uri when instance is a job (#5663)
 * kvs: store correct namespace after getroot lookup (#5661)

## Documentation
 * docs: add `flux_msg_incref` manpage (#5624)
 * doc: correct typo in `flux_requeue(3)` (#5660)

## Cleanup
 * libsubprocess: make `flux_buffer` class private (#5683)
 * job-list: misc cleanup (#5687)
 * drop the flux-mini command (#5666)
 * libsubprocess:  minor clean up (#5667)

## Build/Testsuite
 * test: add some scaling test support (#5717)
 * github: update checkout action to v4 to silence warnings (#5716)
 * docker: add Dockerfile for fedora39 (#5713)
 * ci: add fedora39 build (#5698)
 * testsuite: fix testsuite errors discovered in conda-forge build
   environment (#5685)
 * drop jsonschema requirement (#5678)
 * libpmi: add `JANSSON_CFLAGS` to Makefile.am (#5672)


flux-core version 0.58.0 - 2024-01-04
-------------------------------------

## New Features
 * flux-batch: support `--quiet` option (#5645)
 * resource: get local hwloc XML from parent when possible (#5636)
 * python: Add `kvs.commit_async()` (#5627)
 * python: add `decode` method to Flux `Message` class (#5653)
 * python: add rolemask parameter to `msg_handler_create()` (#5650)

## Fixes
 * libflux: respond to denied, matchtagless requests (#5652)
 * minor updates and improvements to bash completions (#5647)
 * broker: improve handling of interactive initial programs (#5646)
 * resource: fix initialization of multiple brokers per node when fallback
   to job-info.lookup is used (#5639)
 * libflux: add include for `ssize_t` to message.h (#5638)
 * libflux: don't accept `FLUX_MSGTYPE_ANY` in `flux_msg_create()` (#5631)
 * job-list: fix json object mem-leak (#5626)
 * job-list: fix `flux job list-ids --wait-state` (#5620)

## Documentation
 * doc: add missing standard-io options to flux-batch(1) (#5648)
 * doc: expand glossary (#5634)
 * doc: reference flux-environment(7) in job submission cli man pages (#5629)
 * misc doc fixes (#5604)

## Cleanup
 * libflux: drop `flux_msg_frames()` (#5632)
 * libflux: deprecate some message flag accessors and add new and improved
   ones (#5630)
 * maint: remove flux spack docker (#5628)

## Testsuite
 * testsuite: fix fileref unit test failure on tmpfs (#5643)
 * testsuite: avoid artificial corefile in test (#5641)
 * testsuite: fixes for Lassen (#5551)
 * testsuite: fix test corner case on stderr (#5625)
 * testsuite: more reliability improvements in tests (#5621)
 * testsuite: fix t2260-job-list.t with `-d -v` and run inception tests
   with these args to prevent future similar errors (#5618)
 * libkvs: fix test failure in aarch64 CI (#5617)
 * testsuite: fix t2233-job-info-update.t with debug=t (#5616)


flux-core version 0.57.0 - 2023-12-07
-------------------------------------

## New Features
 * support colorized, human-readable output from eventlog commands (#5602)
 * python: add KVSTxn class to KVS module (#5374)
 * libidset: implement API for integer allocator use case (#5580)
 * port to Alpine linux (#5568)
 * job-ingest: make worker input buffer configurable with a default of 10MB
   (#5550)

## Fixes
 * kvs: limit the content of guest commits (#5612)
 * history: track root jobs (#5608)
 * improve ssh connector reliability with different installation paths (#5591)
 * flux-terminus: fix potential hang in terminus client commands (#5607)
 * support start under older versions of Flux without the
   job-info.update-watch RPC (#5589)
 * kvs-watch: improve performance of kvs-watch disconnect/cleanup handling
   (#5585)
 * cli: avoid KeyError when PATH isn't set in environment (#5590)
 * broker: eliminate some message copies (#5559)
 * libidset: improve decoding functions (#5584)
 * fix improper include directives in source files impacting portability
   (#5567)
 * make flux Python commands more resilient to changes in PYTHONPATH (#5553)
 * job-ingest: fix cleanup when a pipeline worker process fails (#5549)
 * libsubprocess: do not allow short writes with `flux_subprocess_write()`
   (#5548)
 * flux-submit: fix substitution of `{cc}` when cc=0 (#5541)

## Documentation
 * doc: use common format for commands with sub-commands (#5597)
 * flux-kvs(1): improve man page formatting (#5588)
 * clean up idset man pages (#5578)
 * doc: improve --urgency option description in job submission commands
   (#5571)
 * doc: improve RFC references in man pages (#5573)
 * man3: add Link with... instruction to SYNOPSIS (#5574)
 * flux-shell(1): improve option descriptions and x-ref (#5557)
 * doc: remove options from flux-alloc(1) et al that don't work (#5555)
 * flux-pmi(1): add new manual page (#5554)
 * flux-start(1): add more description and troubleshooting info (#5552)

## Build
 * build: reduce minimum jansson version to 2.9 (#5546)
 * build: add libmissing, a convenience library for replacements for missing
   functions (#5560)

## Cleanup
 * deprecate flux job cancel and improve flux-job(1) documentation (#5587)
 * job-info: misc cleanup (#5586)
 * broker: cleanup up attribute initialization code (#5543)

## Testsuite
 * testsuite: fix some test races and improve debugging (#5609)
 * testsuite: fix race in job info update lookups (#5598)
 * testsuite: improve reliability of a couple job signal/cancel tests (#5599)
 * testsuite: fix fancy f grep inconsistency (#5576)
 * get sharness tests working on alpine linux (#5564)
 * testsuite: add multiple key job-info lookup tests (#5575)
 * ci: add alpine Dockerfile and CI build (#5565)


flux-core version 0.56.0 - 2023-11-07
-------------------------------------

## New Features

 * support duration update for running jobs (#5522)
 * job-list: support resource-update event (#5463)
 * flux-job: get updated version of R (#5464)
 * cache R in the job manager (#5472)
 * job-info: support new update-lookup and update-watch service (#5467)
 * libflux: make local connector built-in (#5504)
 * make the loop:// connector built-in rather than a DSO (#5486)
 * libflux: add `flux_send_new()` (#5499)
 * add interthread connector that does not require zeromq (#5495)
 * broker: use interthread connector between broker modules (#5496)

## Fixes

 * fix job submission to a multi-user instance over flux-proxy (#5533)
 * job-manager: fix error message on duplicate plugin load (#5537)
 * set `FLUX_PMI_LIBRARY_PATH` only in the job environment when simple pmi
   is active (#5535)
 * job-exec: fix potential hang after exec kill error (#5534)
 * flux-proxy: fix double-free on lost connection (#5529)
 * cron: handle ENOMEM without asserting (#5515)
 * connectors: avoid future ABI issues with `_pad[]` (#5505)
 * python: return more precise result from `flux.util.parse_datetime("now")`
   (#5507)
 * python: fix handling of JobInfo properties and `to_dict()` method with
   missing attributes (#5493)

## Documentation

 * flux-environment(7): add new man page (#5527)
 * man1: improve HTML formatting of man pages (#5521)
 * man3: improve HTML formatting of man pages (#5517)
 * man3: improve HTML formatting of SYNOPSIS sections and C examples (#5503)
 * python: add flux.job.list docstrings (#5500)
 * doc: add interacting with flux section (#5492)

## Cleanup

 * drop broker `conf.module_path`, `conf.connector_path`, `conf.exec_path`
   attributes (#5536)
 * flux job info: drop multiple key support, clean up code, add man page
   entry (#5520)
 * build: reduce junk content in DSOs (#5516)
 * shell: drop job shell standalone mode (#5512)
 * misc build system cleanup (#5511)
 * libflux: refactor internal message queues (#5508)

## Testsuite

 * testsuite: trivial test fixes (#5498)
 * ci: add flux-pmix, flux-pam builds to CI (#5489)


flux-core version 0.55.0 - 2023-10-03
-------------------------------------

## New Features

 * drop libzcmq dependency (#5468)
 * allow hwloc topology to be loaded from a file with `FLUX_HWLOC_XMLFILE`
   (#5462)
 * improve begin-time representation in `flux-jobs(1)` output (#5473)
 * support update of job queue (#5424)
 * job-list: support getting job project and bank (#5413)
 * flux-job: get updated version of jobspec (#5428)
 * flux-top: use streaming job-stats RPC (#5432)
 * job-list: support streaming job-stats RPC (#5430)

## Fixes

 * libzmqutil: fix portability to libzmq-4.1.5 (#5481)
 * broker: move policy config check out to the modules that rely on it
   (#5478)
 * libzmqutil: add cert class and use it instead of CZMQ `zcert_t` (#5461)
 * broker: stop managing 0MQ sockets with czmq (#5454)
 * use zeromq interfaces directly where possible instead of via czmq (#5458)
 * rc: fix `flux start` failure when multiple scripts are present in `rc1.d`
   (#5453)
 * rc: avoid startup problems when `BASH_ENV` is set (#5448)
 * flux-keygen: drop libsodium requirement (#5446)
 * content: make the content cache a broker module (#5435)
 * job-manager: correct fsd output in error message (#5437)
 * modules: consistently return error on invalid module arguments (#5442)

## Documentation

 * doc: add a page on starting Flux (#5477)
 * doc: add build instructions and support info (#5476)
 * doc: add content to landing page and group man pages (#5470)

## Cleanup

 * libflux: drop `flux_panic()` (#5439)
 * job-manager: update: cleanup, small fixes, and documentation (#5434)
 * job-manager: stop publishing job-state event messages (#5433)

## Build/Testsuite/CI

 * switch qemu-user-static setup to fix setuid (#5469)
 * etc: remove ubuntu build-container action (#5474)
 * configure: use distutils if available to avoid extra python module
   dependency (#5459)
 * configure: avoid use of deprecated python distutils module (#5456)
 * testsuite: handle job signal race in more tests (#5438)
 * testsuite: increase sleep time in tests (#5431)


flux-core version 0.54.0 - 2023-09-05
-------------------------------------

## New Features
 * flux-perilog-run: support prolog and epilog timeouts (default 30m) (#5416)
 * cmd: add --with-imp options to flux-exec(1) and flux-perilog-run(1)
   (#5422)
 * shell/pmi: warn if application might be using slurm's libpmi2.so (#5420)
 * job-list: allow updates to all of jobspec (#5418)
 * job-manager: support jobspec update to all fields (#5419)
 * python: add namespace support to KVS module (#5373)
 * add job update service and new job-update(1) command (#5409)
 * job-list: support jobspec-update event (#5408)
 * job-manager: prevent jobspec-update events after a job has resources
   (#5406)
 * job-manager: add flux_jobtap_jobspec_update_pack(3) (#5386)

## Fixes
 * cmd: flux-perilog-run: avoid running prolog/epilog on offline ranks (#5417)
 * job-manager: fix duration limit check for jobs with unset/unlimited
   duration (#5405)
 * flux-top: fix title when connected to instances that are not jobs (#5394)
 * do not search for module and connector DSOs recursively (#5390)
 * python: fix __delitem__ in KVSDir with initial paths (#5376)
 * python: have kvs.isdir() return False if key missing (#5371)
 * python: clear kvs txn after all commits (#5369)

## Cleanup
 * libpmi: cleanup old code and optimize client reads (#5423)
 * job-list: misc cleanup (#5407)
 * broker: refactor broker module loading code and fix minor bugs (#5385)

## Build/Testsuite/CI
 * ci: fix failure detection builds with coverage and remove obsolete system
   tests (#5421)
 * actions: update typo checker version (#5410)
 * extend ci-checks timeout for arm64 build (#5402)
 * testsuite: handle job signal race in more tests (#5401)
 * matrix: add arm64 install-only builder (#5396)
 * testsuite: relax systemctl output parsing (#5388)
 * testsuite: fix race in t0005-exec.t signal test (#5383)
 * actions: add merge_group trigger (#5379)
 * mergify: remove status check conditions from config (#5381)
 * docker-deploy: only push latest when arch is 64-bit (#5377)
 * docker: drop bionic, el7 and fedora35 for bookworm and fedora38 (#5370)

flux-core version 0.53.0 - 2023-08-01
-------------------------------------

## New Features
 * add capability to run jobs in systemd (#5197)
 * job-exec: allow job memory limits to be set (#5359)
 * python: add API for job output (#5332)
 * python: add JobWatcher class and use for `submit` and `bulksubmit`
   `--watch` functionality (#5357)
 * cmd: add flux-watch(1) (#5360)
 * python: add FutureExt class for extensible futures from python (#5330)
 * shell: support `-o gpu-affinity=map:LIST` (#5356)
 * job-info: support WAITCREATE on eventlog watch  (#5358)
 * job-list: support job list constraints (#5126)

## Fixes
 * job-list: support older RPC protocol (#5364)
 * job-manager: prevent jobs with outstanding epilog-start events from
   becoming inactive (#5353)
 * ensure flux utilities agree that a job with a fatal exception has failed
   (#5355)
 * flux-job: suppress the `attach` status line in more situations (#5354)
 * flux-jobs: correct several filtering corner cases (#5164)
 * sdexec: fix memory leaks (#5349)
 * python: fix potential gc of Future ffi handle before Future destruction
   (#5346)
 * resource: fix problem with exclusions when R is dynamically discovered
   (#5339)
 * python: clear KVS txns on commit error (#5335)
 * python: do not return int status in kvs functions (#5329)
 * python: fix exists in KVSDir with initial paths (#5331)
 * python: fix writes in KVSDir with initial paths (#5322)
 * flux-job: fix invalid --original info output (#5318)
 * flux-job: fix `flux job status` handling of nonfatal exceptions (#5320)
 * job-manager: fix prolog/epilog exception handling (#5321)
 * job-info: ignore duplicate lookup keys (#5317)

## Cleanup
 * job-exec: remove systemd exec prototype (#5348)
 * job-manager: make exception note a requirement (#5336)
 * resource: ignore live resource.exclude changes (#5341)
 * python: add extra documentation to kvs.py module (#5328)

## Build/Testsuite/CI
 * testsuite: remove get-xml-test.py (#5340)


flux-core version 0.52.0 - 2023-07-06
-------------------------------------

## New Features

 * libjob: export `flux_unwrap_string(3)` function (#5312)
 * job-manager: add alloc-check plugin (#5304)
 * add f58plain job encoding (#5297)
 * libsubprocess: support user friendly error string  (#5294)
 * python: support convenience API for `job-info.lookup` RPC
   and `flux job info` (#5265, #5311)
 * support `[kKMG]` suffixes in command options that take a bytes argument
   (#5277)
 * libutil: add `parse_size()` (#5262)
 * flux-resource: add `R` subcommand (#5246)
 * job-exec: always use stdio for exec barrier (#5267)
 * sdbus: make debug logging configurable (#5264)

## Fixes

 * job-manager: publish event on jobtap exception (#5310)
 * librlist: fix RFC31 corner cases (#5137)
 * testsuite: workaround job start signal race (#5302)
 * shell: document signal race (#5299)
 * testsuite: fix occasional broker kill error (#5291)
 * do not suppress job shell and broker errors with `flux alloc` (#5274)
 * allow guests to use flux module list, flux module stats (#5280)
 * broker: load module with DSO version extension (#5283)
 * shell: ensure captured pty data does not come after stdout EOF in output
   eventlog (#5282)
 * Parse jobspec attributes.system optionally  (#5279)
 * broker: avoid spurious overlay peer warning (#5275)
 * flux-resource: fix `-i, --include=HOSTS` in `list` command when some
   hosts are excluded (#5268)
 * python: allow JobID to take a JobID (#5259)
 * flux-top: fix formatting with ASCII jobids (#5263)
 * shell: set correct HOSTNAME in job environment if necessary (#5261)
 * Ignore errors when starting flux from a restart dump containing giant
   blobs (#5254)
 * support utf-8 in broker logs (#5253)
 * flux-config-bootstrap(5): fix TOML error (#5252)
 * libjob: return on error in `unwrap_string()` (#5251)
 * libjob: fix leak in `sign_unwrap()` (#5248)
 * flux-job: fix attach notification with multiple prolog-start events (#5315)

## Cleanup

 * switch from decimal to f58 jobid encoding in most log messages and shell
   service name (#5256)
 * flux-job: add missing include of signal.h (#5247)
 * testsuite: improve alloc-check test (#5309)
 * fix assorted typos and adjust whitespace per project norms (#5298)

## Build/Testsuite/CI

 * build: require flux-security >= 0.9.0 (#5270)

flux-core version 0.51.0 - 2023-06-09
-------------------------------------

This release adds the flux-batch(1) and flux-alloc(1) `--conf` option,
which makes subinstance configuration significantly easier.  Sys admins
may note that _named configurations_ can be added to `/etc/xdg/flux/config`
for configurations that are anticipated to be commonly needed such as
node-exclusive scheduling.

For example, `/etc/xdg/flux/config/node-exclusive.toml` could contain:
```toml
[sched-fluxion-resource]
match-policy = "lonodex"
```
And then users could request this in a batch subinstance with:
```
flux batch --conf=node-exclusive ...
```

## New Features

 * add `--conf` option for registering instance config in `flux-batch` and
   `flux-alloc` (#5232)
 * support RFC 14 `files` file archive in jobspec, add `--add-file` option
   to cli submission commands (#5228, #5239)
 * support option to signal a job a configurable time before expiration (#5212)
 * flux-resource: add `-i, --include` option to filter hosts for `status`
   and `drain` commands (#5219)
 * flux-resource: add `-i, --include=TARGETS` option to `list`  and `info`
   subcommands (#5236)
 * broker: forward nonfatal signals to all running jobs (#5202)
 * broker: unset `SLURM_*` in initial program environment (#5237)
 * sdbus: add subscription filtering (#5227)
 * job-exec: provide IMP exec helper when possible and use stdin/out for
   shell exec protocol (#5186)
 * support emoji encoding for Flux jobids (#5174)
 * job-list: support retrieving current working directory for jobs (#5156)
 * job-list: allow list-id to wait for job state (#5213)
 * flux-jobs: support new `inactive_reason` output field and `endreason`
   format (#5055)
 * broker: redefine broker.quorum as a size (#5153)
 * broker: signal readiness to systemd in JOIN state (#5152)

## Fixes

 * completions: update bash completions with recent command changes (#5241)
 * shell/pmi: accept pmi1 and pmi2 as aliases for "simple" pmi (#5242)
 * `zhash_insert()`/`zhashx_insert()` return EEXIST  (#5217)
 * restrict remote access to sdbus on rank 0 (#5162)
 * flux-job: submit: strip unnecessary whitespace from pre-signed jobspec
   (#5185)
 * libsubprocess: fail with ESRCH when pid cannot be found (#5229)
 * flux-jobs: reduce likelihood of leaving bad terminal color state on
   empty output (#5211)
 * python: support non-JSON formatted data in KVS (#5204)
 * Add missing include of config.h (#5182)
 * liboptparse: correct message on missing argument (#5180)
 * python: improve handling of `!d` conversion specifier in output formats
   (#5169)
 * libioencode: fix memleaks on error paths (#5159)
 * shell: add missing `flux_` prefix to `shell_mustache_render(3)` (#5161)
 * shell: truncate output to KVS after 10MB (#5155)
 * python: open UtilConfig files as binary (#5157)
 * job-manager: make some replay errors non-fatal (#5150)
 * doc: fix python example (#5191)
 * flux-job: ignore stdin instead of aborting when unable to create stdin
   watcher (#5183)

## Cleanup
 * cleanup: use ccan/str in place of strcmp(), strncmp() (#5163)
 * job-list: misc cleanup (#5144)

## Testsuite/CI
 * testsuite: fix potential flux-top race (#5224)
 * testsuite: fix tests when run in debug mode (#5231)
 * testsuite: increase test expiration (#5222)
 * testsuite: fix race in flux top tests (#5194)
 * testsuite: skip tests that expect COLUMNS to be inherited across calls
   to flux(1) when that isn't true (#5166)
 * pre-commit: increase vermin version (#5173)
 * ci: fix pre-commit config to lint python files in testsuite (#5221)
 * ci: add fedora38 builder, update flux-security default to 0.8.0 (#5160)

flux-core version 0.50.0 - 2023-05-03
-------------------------------------

## New Features
 * broker: make `tbon.connect_timeout` configurable (#5140)
 * flux-job: support job ids to purge (#5047)
 * sdbus: enable flux to communicate with a systemd user instance (#5070, #5131)
 * shell: support `{{name}}` tag in output file templates (#5128)
 * flux-top: support ability to flip through queues via left/right arrow keys
   (#5052)
 * flux-ping: output header before output of main ping output (#5034)
 * broker: improve systemd integration with `sd_notify(3)` (#5078)

## Fixes
 * `flux(1)`: avoid prepending to PATH when unnecessary (#5138)
 * python: make `SchedResourceList` optional in `flux.job.info` (#5141)
 * fix parent-uri attribute under remote `flux-proxy(1)` (#5133)
 * job-list: make job stats consistent to job results (#5048)
 * fileref: fix compile on systems without `SEEK_DATA`/`SEEK_HOLE` (#5114)
 * fixes for build/test on Fedora 36 (GCC 12) (#5107)
 * shell: fix improper encoding of some hostnames in MPIR proctable (#5117)
 * python: fix parsing of special characters in submission directives (#5125)
 * job-validator: fix empty plugins list when one plugin fails to import
   (#5124)
 * broker: use human readable timestamp in local time when logging to stderr
   (#5129)
 * improve error on plugin load if `flux_plugin_init()` returns an error
   (#5135)
 * librlist: fix memleak + misc cleanup (#5110)
 * sched-simple: avoid assertion failure when trying to load scheduler twice
   (#5109)
 * job-manager: improve errors from jobtap initialization (#5099)
 * libsubprocess: avoid segfault on error path (#5096)
 * job-exec: improve error message when job shell/imp execution fails (#5088)
 * systemd: avoid leaving unit in failed state after flux-shutdown(1) (#5077)

## Cleanup
 * libjob: deprecate `flux_job_list()` and `flux_job_list_inactive()` (#4855)
 * broker: clean up module infrastructure (#5085)
 * libsubprocess: remove use of assert(0) (#5084)

## Testsuite/CI/Development
 * ensure license and other informational files appear in distribution
   tarball (#5113)
 * mergify: add spelling check to required checks (#5112)
 * Add false positives typos config (#5106)
 * Fix minor typos and formatting (#5019)
 * testsuite: fix test issues under nix (#5015)
 * testsuite: fix column width output corner case (#5103)
 * testsuite: fix setup error in system tests (#5102)
 * build: add `make deb` target to build test debian package (#5101)
 * build: applicable build and test fixes for conda (#5093)
 * testsuite: skip failing test on RHEL7 (#5090)
 * add spell check for news, readme, and user facing code (#5074)


flux-core version 0.49.0 - 2023-04-05
-------------------------------------

## New Features
 * libpmi: improve error messages from upmi plugins (#5066)
 * shell: support -o pmi=LIST (#5069)
 * flux-jobs: add --json option (#5054)
 * flux-job: add special exit code to flux job wait when not waitable (#5049)
 * libpmi: enable flux to bootstrap with cray libpmi2.so (#5051)
 * libpmi: improve tracing of dlopened PMI libraries (#5053)
 * resource: do not allow ranks to be both drained and excluded (#5039)
 * Support environment variable to override default output formats (#5028)
 * improve broker debugging on overlay connect failure (#5014)
 * rewrite flux-resource status (#4997)
 * flux-resource: support overwrite of drain timestamp with `--force --force`
   (#5000)
 * python: improve Hostlist class indexing (#4993)

## Fixes
 * openmpi: don't force flux MCA plugins (#5067)
 * PMI: ensure fallthrough when PMI library fails to initialize (#5058)
 * flux-top: fix queue specific output display (#5032)
 * flux-pgrep: fix warning about `sre_constants` on Python 3.11+ (#5043)
 * prevent orphaned job processes when terminating jobs due to exception
   (#4990)
 * python: recognize local timezone epoch adjustment (#5025)
 * fix rare `Unable to connect to JOBID` error from `flux alloc --bg` (#5012)
 * job-manager: ensure epilog-start event prevents resource release for
   job that never started (#5011)
 * librlist: drop V1 flag from hwloc XML generation (#5007)
 * fix: warning message to user should be actual command (#5002)
 * flux-mini: improve deprecation warning (#4989)

## Cleanup
 * job-list: minor code consistency cleanup (#5031)
 * mpi: drop mvapich.lua plugin (#5045)
 * libflux: remove extraneous +1s used in buffers (#5020)
 * cleanup: improve interface for subprocess logging (#5006)
 * cleanup: simplify remote subprocess protocol (#5004)
 * cleanup: allow subprocess service name to be configured (#5003)
 * cleanup: improve reusability of common message handlers (#5001)

## Documentation
 * flux-job(1): update WAIT section (#5042)
 * doc: document job environment variables (#5024)
 * doc: document `FLUX_URI_RESOLVE_LOCAL` (#5023)
 * cli: adjust description of `--begin-time` submission option (#5018)

## Testsuite/CI/Development
 * testsuite: increase test timeouts (#5017)
 * testsuite: speed up t4000-issues-test-driver.t (#5010)
 * testsuite: require jq(1) (#4995)


flux-core version 0.48.0 - 2023-03-07
-------------------------------------

This release adds submission directives ("see flux help batch") and
shortens the the job submission commands to "flux batch", "flux run",
etc.  The flux-mini(1) command is deprecated.

## New Features

 * support RFC 36 submission directives in `flux mini batch` (#4942)
 * make all flux-mini subcommands available as top level flux commands (#4961)
 * add flux-cancel(1) (#4983)
 * add flux-fortune (#4966)
 * flux-run: allow stdin to be directed to a subset of tasks (#4977)
 * cmd: add -u, --unbuffered option to submission commands (#4973)
 * allow flux-core to be configured in ascii-only mode (#4968)
 * Support {{tmpdir}} in shell mustache templates and simplify batch jobspec
   construction (#4951)
 * broker: allow a file argument to `-c, --config-path` in TOML or JSON
   (#4949)

## Fixes

 * completions: remove flux-mini and other updates (#4984)
 * flux-top: initialize f character before drawing panes (#4982)
 * libsubprocess:  don't abort remote processes that receive SIGSTOP (#4981)
 * shell: fix memory leak in doom plugin (#4979)
 * flux-resource: suppress NGPUS on systems without GPUs (#4959)
 * job-ingest: handle worker channel overflow (#4948)
 * improve error message in Python frontend utilities when broker is not
   running (#4950)
 * job-manager: fix for job priority not reset after a duplicate urgency
   update (#4941)
 * job-manager/history: track inactive jobs over purge/restart (#4932)

## Cleanup

 * flux-filemap: update to RFC 37 internally (#4974)
 * libsubprocess: rework internal logging (#4960)
 * libsubprocess: drop `FLUX_SUBPROCESS_EXEC_FAILED` state (#4955)
 * libsubprocess: fix bugs and clean up subprocess server (#4944)

## Documentation

 * Fix minor documentation errors and typos (#4934)
 * flux-batch(1) and flux-alloc(1): improve DESCRIPTION section (#4963)
 * README: fix a couple typos (#4970)
 * README: trim it down now that we have readthedocs (#4969)
 * divide flux(1) help output into sections (#4967)
 * doc: add shell help on flux-jobs(1) formatting (#4939)
 * doc: document attach in flux-job(1) (#4936)

## Testsuite/CI/Development

 * testsuite: unset `FLUX_F58_FORCE_ASCII` in some tests (#4976)
 * .devcontainer permissions fix (#4964)
 * Add/developer doc on commands (#4965)

flux-core version 0.47.0 - 2023-02-07
-------------------------------------

## New Features

 * add `flux job last` (#4908)
 * add `flux-pgrep` and `flux-pkill` (#4867, #4903)
 * add `flux-keygen --meta KEY=VAL` option (#4882)
 * add tools for querying job remaining time: `flux_job_timeleft(3)`, python
   `flux.job.timeleft()` and `flux-job timeleft` (#4845)
 * flux-shell: add `-opmi=off` option (#4841)
 * suggest use of `--force` in `flux resource drain` when target is already
   drained (#4924)
 * automatically provide job status for pending interactive jobs (#4916)
 * flux-resource: mark drained+offline nodes with asterisk (#4913)
 * support flux mini batch,alloc `--dump[=FILE]` (#4881)
 * flux-queue: support flux queue list (#4896, #4929)
 * support RFC 31 `hostlist` and `rank` job constraints (#4895, #4919)
 * python: add flux.constraint.parser for RFC 35 Constraint Query Syntax
   (#4871, #4925)
 * Support RFC 35 constraint syntax in `flux mini --requires` (#4897, #4923)
 * flux-top: limit jobs and summary to specific queue (#4847)
 * enable broker bootstrap methods to be provided by dso plugins, and drop
   compiled-in pmix support (#4865)
 * flux-resource: support QUEUE output in resource list (#4859)
 * flux-top: Support --color option (#4840)
 * libutil: support "infinity" in FSD (#4846)
 * add internal universal PMI client library (#4829)
 * job-manager: default queues to enabled and stopped (#4857)
 * libtaskmap: add `TASKMAP_ENCODE_RAW_DERANGED` (#4838)

## Fixes

 * job-list: do not assume alloc event context always contains annotations
   (#4907)
 * job-manager: fix alloc-bypass plugin (#4901)
 * flux-resource: increase width of queue field (#4905)
 * eliminate "safe mode" after improper shutdown (#4898)
 * flux-resource: handle queues with no configured constraints (#4893)
 * fix message encoding problem introduced in v0.46.1 (#4890)
 * flux-shell: truncate long log messages (#4878)
 * job-manager: switch to timer watcher in perilog plugin (#4864)
 * job-manager: do not checkpoint on every queue state change (#4856)
 * job-list: separate `t_submit`/`t_depend` calculation (#4853)
 * flux-top: honor `FLUX_F58_FORCE_ASCII` (#4842)
 * flux-job: fix potential segfault (#4827)
 * work around fluxion inbability to recover running jobs (#4894)
 * etc: update bash completions (#4928)

## Documentation

 * doc: document `--job-name` in flux-mini(1) (#4879)
 * doc: document format fields in flux-resource(1)  (#4850)
 * doc: document subcommand `wait` in flux-job(1) (#4851)

## Testsuite/CI/Development

 * clean up little used broker attribute functionality (#4870)
 * flux-queue: rewrite in python (#4889)
 * job-list: add jobspec and R parsing unit tests (#4883)
 * flux-top: add extra test coverage (#4833)
 * testsuite: increase `flux job wait-event` timeout (#4888)
 * testsuite: drop fragile broker.boot-method test (#4876)
 * docker: add site-packages to default python3 path (#4880)
 * ci: speed up coverage builds (#4828)


flux-core version 0.46.1 - 2022-12-11
-------------------------------------

## Fixes
 * build: fix installation of libpmi.so (#4824)
 * testsuite: fix failure on a system with fully-qualified hostname (#4825)

## Cleanup
 * libflux/message: cleanup with macros and CCAN pushpull class (#4823)

flux-core version 0.46.0 - 2022-12-10
-------------------------------------

## New Features
 * job-manager: support start / stop of individual queues (#4776)
 * add file broadcast toolset (#4789, #4818)
 * flux-pmi: add installed PMI test tool (#4817)
 * flux-mini: add --cwd option (#4811)
 * add flux start --recovery (#4783)
 * support cyclic, manual job taskmaps and `flux job taskmap` command (#4772)
 * shell: remove `pmi.clique` option, add `pmi.nomap` (#4785)
 * libtaskmap: support RFC 34 unknown task maps (#4788)
 * python: add `flux.job.get_job` (#4761)
 * allow guests to access flux configuration (#4766)
 * job-list: add inactive purge count to job-list stats (#4756)
 * shell: allow users to map specific cpus to tasks with `map` cpu-affinity
   option (#4819)

## Fixes
 * broker: fix startup failure when broker.mapping is not set (#4781)
 * encode broker.mapping as RFC 34 taskmap, drop unused pmi clique helper
   code (#4787)
 * add missing taskmap pkg-config file (#4782)
 * broker: fix potential crash after sending SIGKILL to job-manager prolog
   (#4793)
 * broker: improve detection of interactive shells (#4795)
 * flux-job: remove `finish_mpir_interface()` (#4808)
 * libhostlist: fix 8K limit on encoded hostlists (#4803)
 * add flux.taskmap to PMI kvs for better cyclic task distribution
   scalability (#4798)
 * job-manager: fix memory corruption due to json reference counting bug
   (#4773)
 * python: by default return all attrs in job listing functions (#4762)
 * shell: rlimit: improve handling of limits over hard limit (#4752)

## Cleanup
 * cleanup: remove some unused functions (#4806)
 * job-manager: use allocation terms consistently (#4775)
 * content: cleanup and minor tooling updates (#4770)

## Testsuite/CI/Development
 * testsuite: add flux job raise/cancel/kill tests (#4747)
 * github: fix automated tag release action (#4755)
 * Vscode setup (#4683)
 * ci: revert to ubuntu-20.04 for workflow jobs that fail on ubuntu-latest
   (#4794)
 * testsuite: fix several corner cases in t0029-filemap-cmd (#4812)


flux-core version 0.45.0 - 2022-11-01
-------------------------------------

## New Features
 * propagate process resource limits to jobs (#4745)
 * flux-job: support multiple jobids for `cancel`, `raise`, and `kill` (#4721)
 * flux-resource: unify output format options, support config files and
   named formats (#4710)
 * broker: support binomial tree topology (#4730)
 * broker: allow custom topology to be configured (#4675)
 * flux-mini: add -x short option for --exclusive (#4726)
 * flux-jobs: support emoji output formats (#4687)
 * flux config: add load subcommand (#4695)
 * broker: ignore ENOSYS from parent job-exec.critical-ranks RPC (#4680)
 * job-list: support retrieving job's core count (#4664)
 * job-list: add successful job count stat (#4739)
 * job-list: support queue specific stats (#4684)
 * etc: add functional bash completions (#4661, #4742)

## Fixes
 * job-list: ensure purged jobs are inactive (#4738)
 * flux-proxy: require owner == proxy user (#4712, #4735)
 * support mpi=none shell option and make it the default for `flux mini
   batch` and `flux mini alloc` (#4731)
 * unset job environment variables in initial program (#4717)
 * flux-resource: fix scalability issues with large sets of resources (#4713)
 * build: fix use of system bash-completion dir (#4667)
 * rc1: reload configuration on rank > 0 (#4665)
 * broker/test: Fix runat race on older glibc versions (#4660)
 * broker: launch non-interactive shells in a new process group (#4653)

## Cleanup
 * job-list: cleanup error logging, remove excess logging (#4744)
 * README: update flux help output (#4688)
 * python: indicate truncation for some fields in flux-jobs and flux-resource
   (#4670)
 * python: move and rename some classes for reusability (#4669)
 * job-list: refactor to abstract "idsync" logic better (#4644)
 * broker: don't log failed `CONTROL_DISCONNECT` (#4656)

## Testsuite/CI/Development
 * fix github action syntax for output and yaml formatting (#4733)
 * add devcontainer autocomplete (#4709)
 * lint: update devcontainer to work with pre-commit (#4690)
 * codeql: fix some critical issues found by security scanning (#4729)
 * Create codeql.yml (#4705)
 * ci: add isort to pre-commit and linting (#4691)
 * ci: update setup-python/buildx actions to v4/v2 (#4693)
 * Pre-commit extensions and multi-version setup (#4689)
 * ci: consolidate python linting and formatting (#4636)
 * Add devcontainer environment for vscode (#4674)
 * test: job-ingest: skip guest tests when default sign-type fails (#4655)

flux-core version 0.44.0 - 2022-10-04
-------------------------------------

This release includes initial support for job queues, which can be
configured to partition resources with different limits and defaults.
See `flux-config-queues(5)` for details.

Other highlights include:

 * Add ability to modify jobspec at the time of ingest with a new
   job frobnicator framework similar to the existing validators.
 * A new alternative to `system.R` for configuration of resources
   (See `flux-config-resource(5)` for details)
 * All child Flux instances are resilient to non-critical node failures
 * Updates to `flux jobs` including better default output, a set of
   new default named formats (See `flux jobs --format=help`), and support
   for config files to add more named formats.

## New Features
 * ingest: set configured queue constraints (#4587)
 * ingest: enable frobnicator when needed by [queues] or [policy] (#4608)
 * reject jobs submitted to a named queue when none are configured (#4627)
 * make queue state persist across instance restart (#4640)
 * flux-mini: add `-q, --queue=NAME` option (#4599)
 * flux-top: add minimal support for job queues (#4605)
 * flux-jobs: support getting job queue and filtering by job queue (#4579)
 * flux-jobs: support collapsible format fields (#4591)
 * flux-jobs: add configurable named formats (#4595)
 * flux-jobs: add queue to builtin format strings (#4607)
 * flux-jobs: add `--format=long` (#4642)
 * flux-jobs: support width and alignment with `!d` conversion specifier (#4597)
 * python: add `contextual_time` jobinfo field (#4641)
 * python: add `contextual_info` jobinfo field (#4626)
 * broker: reduce `tbon.tcp_user_timeout` (#4632)
 * make child instances resilient to non-critical node failures (#4615)
 * resource: support configuration of resources in TOML config (#4566)
 * drop environment from KVS jobspec (#4637)
 * docker: add namespaced builds for spack/ubuntu (#4577)

## Fixes
 * flux-mini: change default unit for `--time-limit` to minutes (#4565)
 * job-list: handle per-resource "cores" special cases (#4630)
 * job-list: handle per-resource ntasks special case (#4555)
 * job-list: use libjj to parse jobspec (#4611)
 * job-list: parse tasks with total count in jobspec (#4651)
 * job-info: return error on invalid eventlog append (#4624)
 * flux-shell: always use `pmi=pershell` by default (#4621)
 * ingest: require job queue if [queues] are configured (#4616)
 * job-ingest: assign jobspec defaults before scheduler feasibility check
   (#4529)
 * configure: remove `--without-python` (#4584)
 * configure: fix obsolete autotools warnings (#4588)
 * configure.ac: fix error message when running autogen.sh (#4590)
 * job-manager: print better errors on inactive move (#4586)
 * broker: fix use-after-free segfault (#4570)
 * python: uri: use path to current flux executable in lsf resolver (#4559)
 * spack: add flux-core container build (#4561)
 * improve signal/noise ratio in systemd journal logs (#4560)
 * flux-mini: improve an error message and documentation for per-resource
   options (#4549)
 * doc: document `{id.dec}` in `flux-jobs(1)` (#4548)
 * doc: add note about flux `--parent` option in `flux-mini(1)` (#4650)
 * job-manager: do not ignore failure to load configured plugins (#4647)

## Cleanup
 * job-list: remove circular header dependencies (#4639)
 * job-list: split out job information into new files (#4575)
 * job-list: misc cleanup (#4563)
 * Container base: remove view copy (#4551)

## Testsuite
 * testsuite: document `FLUX_TEST_VALGRIND` (#4643)
 * testsuite: remove errant `test_done` call (#4609)
 * testsuite: fix spurious resource norestrict test failures on some version
   of hwloc (#4550)


flux-core version 0.43.0 - 2022-09-06
-------------------------------------

This release includes changes after several weeks of Flux running as the
primary resource manager on two small production systems at LLNL.  Some
noteworthy changes are:  new options to flux-jobs(1) and flux-mini(1), show
detailed job state in flux-jobs(1) output, and automatic KVS garbage
collection.  Also included:  bug fixes for for cpu affinity, tcsh users,
users with non-UTF-8 compliant terminals, and a rank 0 broker segfault when
inactive job purging is enabled.

## New Features
 * cmd: add "per-resource" allocation options to flux-mini run and submit
   (#4544)
 * job-list: return nnodes if jobspec specifies nodes (#4542)
 * resource: add norestrict option to avoid restricting hwloc topology XML
   (#4538)
 * flux-mini: add --bg option to flux-mini alloc (#4531)
 * kvs: support gc-threshold config (#4528)
 * etc: support content.backing-module=none (#4492)
 * fetch J instead of jobspec in the job shell, support flux job info
   --original jobspec (#4523)
 * flux-jobs: add --since=WHEN and --name=NAME options (#4517)
 * add flux jobtap query subcommand (#4507)
 * libkvs: Support `KVS_CHECKPOINT_FLAG_CACHE_BYPASS` flag (#4477)
 * flux-mini: --setattr: place keys in `attributes.system` by default
   and default value to 1 (#4483)
 * kvs: add root sequence number to checkpoint object (#4475)

## Fixes
 * shell: inherit `FLUX_F58_FORCE_ASCII` from job environment (#4541)
 * shell: fix cpu-affinity=per-task (#4537)
 * flux-mini: fix bulksubmit help message (#4539)
 * fix ssh connector with csh/tcsh shells (#4532)
 * broker: log content store errors to `LOG_CRIT` (#4526)
 * broker: forward content.checkpoint-{get,put} RPCs to rank 0 (#4519)
 * cmd/flux-jobs: include job state in status output (#4515)
 * flux-jobs: improve bad username error message (#4503)
 * update fluid check to check explicitly for utf-8 (#4505)
 * doc: add TIMEOUT result to flux-jobs(1) (#4500)
 * fix formatting issues with large UIDs (#4489)
 * broker: fix content-cache flush list corruption (#4484)
 * top: fix detailed report in summary pane (#4479)
 * content-{sqlite,files,s3}: route checkpoint-get and checkpoint-put
   through broker (#4463)
 * job-list: avoid segfault after job purge (#4470)

## Cleanup
 * job-list: remove job-list.list-inactive RPC (#4513)
 * flux-job: point users to flux-jobs(1) (#4499)
 * docker: typo in path to Dockerfile (#4490)
 * add start of spack base image for flux-core (#4471)
 * docker: add pam development package to images (#4473)
 * refactor broker overlay for topology flexibility (#4474)
 * github: fixes for issues found when pushing a tag (#4462)

## Testsuite
 * testsuite: fix non-bourne shell test failure (#4543)
 * testsuite: add more checkpoint sequence tests (#4518)
 * testsuite: use flux jobs in valgrind workload (#4512)
 * testsuite: unset `FLUX_F58_FORCE_ASCII` during testsuite (#4509)
 * testsuite: add timeout job tests (#4501)
 * testsuite: misc valgrind cleanup (#4480)


flux-core version 0.42.0 - 2022-08-02
-------------------------------------

## New Features

 * add flux_open_ex(3) (#4450)
 * flux-top: support split of inactive jobs into completed and failed (#4449)
 * job-manager: add limits plugins for duration and job size (#4415)
 * kvs: add defensive checkpoint and kvs.checkpoint-period TOML config (#4383)
 * python: add LSF URI resolver plugin (#4385)
 * allow configurable defaults for jobspec system attributes (#4386)
 * jobtap: add conf.update callback (#4411)
 * Add a posix_spawn() implementation to libsubprocess and use it to launch
   job shells (#4395)
 * jobtap: add job.create, job.destroy callbacks (#4392)
 * job-manager: allow dependencies on inactive jobs (#4388)

## Fixes

 * content-sqlite,files,s3: register with cache after setup complete (#4458)
 * flux-overlay: add man page, open to guest users (#4459)
 * flux-relay: initialize log prefix to hostname when possible (#4454)
 * flux-top: avoid premature exit on recursive top error (#4452)
 * job-manager: improve robustness of max job id recovery on restart (#4443)
 * flux-config-bootstrap(5): improve hosts description (#4444)
 * libflux: handle flux_respond_error (errnum=0) (#4427)
 * flux-queue(1): add man page (#4426)
 * sched-simple: fix allocation of down nodes when using constraints (#4425)
 * job-archive: improve logging on parse job error (#4422)
 * job-info: handle invalid eventlog entry errors more carefully (#4416)
 * flux-dump: fix handling of empty blobref value (#4418)
 * job-manager: fix race in job eventlog commit and job shell start (#4412)
 * job-manager: fix dependency-add from job.state.depend callback (#4406)
 * job-manager: ensure job aux items are destroyed safely (#4397)
 * job-manager: fix restart code to handle jobs from earlier releases (#4399)

## Cleanup

 * use ccan ARRAY_SIZE() macro (#4445)
 * kvs: rename kvs.sync target to kvs.wait-version (#4410)
 * Use flux_error_t and errprintf() over char buf and snprintf() (#4407)
 * content-sqlite: fix double free (#4391)
 * kvs: misc cleanups (#4389)

## Testsuite

 * ci: create kvs dumpfile from previous tag for testing (#4402)

flux-core version 0.41.0 - 2022-07-04
-------------------------------------

## New Features

 * job-manager: transition NEW to DEPEND on "validate" event (#4366)
 * kvs: support FLUX_KVS_SYNC (#4348)
 * shell: obtain hwloc XML from enclosing instance (#4373)
 * libkvs: add `flux_kvs_commit_get_rootref()`  (#4374)

## Fixes

 * job-manager: fix case where a job manager epilog can't be configured
   without a prolog (#4382)
 * broker: return error in content.flush if no backing store exists  (#4376)
 * broker: content cache corner case corrections (#4380)
 * job-manager: transition back to PRIORITY state on urgency update (#4364)
 * wait for 'finish' instead of 'clean' event in flux-mini run and flux-job
   attach (#4361)

## Cleanup

 * kvs: remove excess logging of ENOTSUP (#4381)
 * job-manager: misc cleanup (#4362)

## Testsuite

 * testsuite: fix perilog sanity test (#4363)
 * t2201-job-cmd.t: fix bug in UTF-8 test (#4360)

flux-core version 0.40.0 - 2022-06-07
-------------------------------------

## New Features

 * content-sqlite: verify database integrity during module load (#4340)
 * job-exec: support new sdexec job launch plugin  (#4070)
 * job-manager: post submit event, instead of job-ingest (#4346)
 * shell: execute job tasks in their own process group (#4355)

## Fixes

 * shell: improve handling of TMPDIR (#4330)
 * job-manager: do not send purged events (#4334)
 * job-list: consistently return job attributes (#4327)
 * python: fix confusing error message from pid URI resolver (#4335)
 * improve logging of overlay peer authentication (#4342)
 * libflux: return better errno in future wait path (#4345)
 * shell: fix reconnect hang (#4293)
 * libsubprocess: avoid segfault on empty argv (#4350)
 * docs: add python resource_list docstrings (#4353)

## Cleanup

 * job-list: misc cleanup (#4332)
 * job-manager: misc cleanup (#4352)

## Testsuite

 * docker: update default version of flux-security to v0.7.0 (#4356)


flux-core version 0.39.0 - 2022-05-06
-------------------------------------

## New Features

 * job-list: support new "all" attribute to get all job attributes (#4324)
 * flux-overlay: replace --no-color with --color=WHEN (#4322)
 * flux-overlay: add -H, --highlight option (#4322)
 * flux-shutdown: add --gc garbage collection option (#4303)
 * content: track RFC 10 protocol changes (#4299)
 * flux-dmesg: colorize output and add -H, --human option (#4289)
 * job-manager: add inactive job purge capability (#4286)
 * libutil: support "ms" suffix for Flux Standard Duration (#4284)
 * add flux-resource info subcommand (#4272)

## Fixes

 * python: uri: fix intermittent failure of fallback pid resolver (#4320)
 * flux-job: fix purge usage message (#4318)
 * use correct type in content-sqlite, misc. content test cleanup (#4315)
 * job-archive: use safer pragmas (#4307)
 * select safer content.sqlite consistency pragmas (#4294)
 * sched-simple: do not allocate down nodes with --exclusive (#4292)
 * python: fix return from `flux.importer.import_plugins()` when no plugins
   found (#4288)
 * fix hang when job with an interactive pty fails early (#4283)
 * broker: prevent downstream peers from connecting during shutdown (#4277)

## Cleanup

 * flux-shutdown(1): document new options (#4323)
 * README: add libarchive prerequisite (#4319)
 * content-s3: cosmetic cleanup (#4314)
 * job-list: misc cleanups (#4297)
 * broker: suppress online message with no members (#4298)
 * job-manager: introduce config callback (#4279)
 * libev: update to version 4.33 (#4282)
 * libflux: convert `flux_conf_error_t` to `flux_error_t` (#4278)
 * resource: stop collecting/reducing hwloc XML  (#4263)
 * flux-hwloc: remove command (#4262)
 * flux-resource: remove ranks from default status output (#4271)
 * libsubprocess: remove prefix on server setup (#4268)

## Testsuite

 * testsuite: increase test timeout (#4321)
 * teststuite: document and fixup LONGTEST tests (#4305)
 * testsuite: minor README fixes / updates (#4291)
 * docker: update default flux-security version to v0.6.0 (#4274)
 * testsuite: fix failing tests on parallel testsuite runs (#4275)
 * ci: add build for Fedora 35 (#4270)

flux-core version 0.38.0 - 2022-04-04
-------------------------------------

This release makes a few improvements that are visible in the flux-mini(1)
command:

 * The `-N,--nnodes` option may be used without the `-n,--nprocs` option.
 * The `--exclusive` option requests the allocation of whole nodes.
 * The `--requires` option requests resources with generic properties.

Additionally, Flux system administrators should be aware of these changes:

 * Named properties may be assigned to resources in the configured R.
 * flux-shutdown(1) is now the preferred way to stop a Flux system instance.
 * The default `archive.dbpath` is now `/var/lib/flux/job-archive.sqlite`.
 * systemd-coredump(8) can now capture a Flux broker core dump.  Consider
   enabling this on the management nodes of early access systems to help
   gather information on critical rank 0 broker failures, should they occur.
 * `flux resource drain` now shows nodes as "draining" if they are still
   running work.
 * Flux may be configured to reject jobs that do not run in a Flux sub-instance.

For more information, see the Flux Administrator's Guide:

https://flux-framework.readthedocs.io/en/latest/adminguide.html

## New Features

 * add flux-shutdown command (#4250)
 * add flux-dump and flux-restore (#4208, #4225)
 * support for node exclusive allocations (#4245)
 * add support for resource properties (#4236)
 * flux-resource: support properties when listing resources (#4249)
 * job-ingest: add TOML config  (#4238)
 * flux-dmesg: add --new option, plus logging cleanup (#4237)
 * add 'require-instance' job validator plugin (#4239)
 * job-manager: add builtin job duration validator plugin (#4224)
 * sched-simple: set expiration of jobs with no duration to instance lifetime
   (#4223)
 * flux-resource: differentiate drained vs draining ranks (#4205)
 * librlist: support hwloc discovery of AMD RSMI GPUs (#4203)
 * broker: reject remote exec requests on rank 0 (#4258)
 * python: allow resource count of 0 in jobspec v1 (#4259)
 * job-archive: use statedir path if dbpath is not set (#4260)

## Fixes

 * content-sqlite: ensure that module load fails if initialization fails (#4265)
 * job-archive: use statedir path if dbpath not set (#4260)
 * broker: emit error when running interactive shell without tty (#4253)
 * broker: add statedir attribute, drop content.backing-path (#4248)
 * broker: prevent systemd restart if rc1 fails (#4246)
 * flux.service: use StateDirectory for content.sqlite (#4244)
 * rc3: ensure exit code reflects any errors (#4243)
 * broker: don't leave shutdown state prematurely (#4241)
 * libjob: improve `flux_job_statetostr()`, `flux_job_resulttostr()`
   interface (#4235)
 * job-list: fix bugs in some error paths (#4233)
 * broker: fine tune logging and enable core dump on SIGSEGV (#4231)
 * kvs: always store empty directory object to content store (#4229)
 * restrict access to content service used as KVS blob layer (#4216)
 * content-sqlite: check that file has rw permission (#4215)
 * broker: block SIGPIPE (#4211)
 * shell: add hostname to a couple key log messages (#4200)
 * python: add missing methods and improve efficiency of IDset class (#4209)
 * systemd: set SyslogIdentifier to flux (#4206)
 * misc minor fixes and cleanup (#4197)
 * job-exec: fix more potential hangs after early shell failure (#4199)
 * sched-simple: fix counting bug that can cause scheduler to fail after
   a restart (#4196)
 * flux-top: add man page, minor bug fixes (#4194)

## Cleanup

 * broker: clean up shutdown logs (#4257)
 * libsdprocess: minor fixups (#4252)
 * job-manager: misc cleanup (#4232)

## Testsuite

 * testsuite: fix a couple intermittent test failures (#4247)
 * ci: run 32bit build under linux32 personality (#4240)
 * testsuite: ensure tests can run concurrently with `--root=$FLUX_JOB_TMPDIR`
   (#4212)


flux-core version 0.37.0 - 2022-03-04
-------------------------------------

This release disables resource verification of GPUs by default to
workaround issues with GPU detection with system versions of hwloc.

### Fixes

 * resource: restrict resource verification to cores/hostnames only (#4192)
 * resource: assign ranks in R based on hostlist attribute (#4188)
 * add assertions that rank, size, hostlist broker attributes are cacheable
   (#4187)

### Testsuite

 * testsuite: fix racy tests in t0005-rexec (#4179)

### Cleanup

 * build: ensure autogen.sh updates package version (#4174)


flux-core version 0.36.0 - 2022-03-01
-------------------------------------

This release adds support for restarting a Flux system instance in safe
mode after a failure to shut down properly -- for example in the case of a
broker crash.  New `flux-startlog(1)` and `flux-uptime(1)` commands are
also introduced to give a quick review of the start and stop times and
status of the current Flux instance.

System instance users will want to update their configuration files to
set `tbon.tcp_user_timeout` and remove `tbon.keepalive_*`, if present.
For more information, see the Flux Admin Guide:

https://flux-framework.readthedocs.io/en/latest/adminguide.html

### Fixes

 * job-exec: fix job hang after early IMP/shell exit (#4155)
 * broker: allow `tbon.zmqdebug` to be set in config file and make sure it's
   really off if set to 0 (#4127)
 * broker: handle network partition (#4130)
 * shell: capture job shell error messages in designated output file (#4125)
 * resource: emit a more specific error when `rlist_rerank()` fails (#4126)
 * flux-overlay: fix timeout error message (#4131)
 * README: add libc development packages in requirements (#4133)
 * libflux/future: set missing errno in `flux_future_wait_for()`  (#4162)
 * flux-config-archive(5): fix TOML example (#4164)
 * shell: fix delay in completion of jobs with a single shell rank (#4159)

### New Features

 * flux-uptime: provide useful output for slow/stuck broker state (#4172)
 * improve KVS checkpoint protocol to allow for future changes (#4149)
 * add `flux config get` (#4166)
 * broker: use RPC not control message for broker module sync/status (#4110)
 * docs: add Python overview documentation (#4104)
 * Support new libsdprocess to launch processes under systemd (#3864)
 * rename keepalive messages to control messages (#4112)
 * resource: enhance resource.drain RPC with "update" and "overwrite" modes
   (#4121)
 * broker: replace keepalive tunables with `tcp_user_timeout` (#4118)
 * kvs: add date to kvs-primary checkpoint (#4136)
 * libpmi2: implement bits needed for Cray MPI (#4142)
 * add `flux-uptime` command (#4148)
 * add `flux-startlog` and enter safe mode after crash (#4153)
 * libflux: add `flux_hostmap_lookup(3)` (#4157)

### Cleanup

 * drop unused project meta files (#4170)
 * doc: update flux-broker-attributes(7) (#4119)
 * python: return `JobID` from flux.job.submit, not `int` (#4134)
 * consolidate multiple `*_error_t` structures into a common `flux_error_t`
   (#4165)
 * drop unused project meta files (#4170)

### Testsuite

 * testsuite: remove unportable cshism (#4115)
 * codecov: minor improvements for coverage reporting (#4147)
 * testsuite: add clarification comments (#4167)


flux-core version 0.35.0 - 2022-02-05
-------------------------------------

This release fixes a broker crash when a job receives an exception after
running a job prolog. Users of the prolog/epilog feature should update
to this version as soon as possible.

In addition, TCP keepalive support was added for detection of powered off
compute nodes.  For configuration info, see the Flux Admin Guide:

https://flux-framework.readthedocs.io/en/latest/adminguide.html


### Fixes

 * flux-ping: support hostnames in TARGET #4105
 * Fix broker segfault when an exception is raised on a job after prolog (#4096)
 * flux-overlay: improve timeouts, hostname handling (#4095)
 * flux resource: allow limited guest access (#4094)
 * shell: fix duplicate logging after evlog plugin is initialized (#4085)
 * shell: do not allow instance owner to use guest shell services (#4101)
 * shell: fix race in attach to interactive job pty (#4102)
 * libterminus: fix leak in pty client code (#4098)

### New Features

 * broker: use TCP keepalives (#4099)
 * systemd: set restart=always (#4100)
 * flux-mini: add --wait-event option to submit/bulksubmit (#4078)

### Testsuite

 * testsuite: fix spellcheck (#4082)
 * ci: rename centos images to el, and use rockylinux for el8 image (#4080)


flux-core version 0.34.0 - 2022-01-28
-------------------------------------

This release features the automatic draining of "torpid" (unresponsive)
nodes, to prevent new work from being scheduled on them until the instance
owner investigates and runs `flux resource undrain`.

### Fixes

 * libsubprocess: fix excess logging and logging corner cases (#4060)
 * doc: fix cross-references (#4063)
 * flux-proxy: improve experience when proxied Flux instance terminates
   (#4058)
 * flux-perilog-run: improve usefulness of logging when prolog/epilog fails
   (#4054)
 * Fix issues found on Cray Shasta (perlmutter) (#4050)
 * env: fix prepend of colon-separated paths in reverse order (#4045)
 * python: fix ImportError for collections.abc.Mapping (#4042)
 * job-list: fix "duplicate event" errors (#4043)
 * systemd: set linger on flux user (#4035)

### New Features

 * shell: enhance pty support (#4075)
 * add broker.starttime; add uptime to flux-top, flux-pstree (#4076)
 * libflux: add `flux_reconnect()`, revamp flux fatal error callback (#4016)
 * doc: add/improve man pages for config files (#4057, #4069)
 * resource: drain torpid nodes (#4052)

### Cleanup

 * broker/content: misc cleanup (#4074)
 * improve error message from flux-proxy and flux-jobs for invalid and
   unknown jobids (#4062)
 * cmd/flux-ping: make help output clearer (#4061)
 * configure: Add python docutils check, do not require doc utils to build
   flux help (#4056)

### Test

 * testsuite: fix non-bourne shell test failure (#4064)
 * sharness: unset `FLUX_CONF_DIR` for all tests (#4059)
 * ci: fix use of poison-libflux.sh and add poison `flux-*` commands (#4046)


flux-core version 0.33.0 - 2022-01-08
-------------------------------------

This release includes several improvements in the recursive tooling
in Flux to enhance the user experience when dealing with nested jobs.

Highlights include:

 * Improved interface for job URI discovery allows `flux proxy` and
   `flux top` to target jobids directly.
 * Addition of a `-R, --recursive` option to `flux jobs`
 * Support in `flux top` for selecting and recursing into sub-instances
 * A new  `flux pstree` command for displaying job hierarchies in a tree

### Fixes

 * systemd: fix typo in flux.service unit file (#3996)
 * libflux: check reactor flags (#4014)
 * fix uninterruptible hang when attached to terminated jobs with -o pty
   (#4010)
 * cmd/flux-jobs: re-work -A option and clarify -a option (#4012)
 * broker: avoid inappropriate quorum.timeout (#4027)
 * add workaround for invalid job timeouts when system is busy (#4037)

### New Features

 * add FluxURIResolver Python class and flux-uri command for job URI
   discovery (#3999)
 * cmd: support high-level URIs and JOBID arguments in flux-top and
   flux-proxy (#4004, #4015)
 * flux-top: allow top to recursively call itself (#4011)
 * flux-jobs: add --recursive option (#4019, #4024)
 * flux-jobs: support instance-specific fields in output (#4022)
 * add flux-pstree command (#4026)

### Cleanup

 * doc: add flux-resource(1),  clean up help output (#4021)
 * doc: audit / cleanup SEE ALSO and RESOURCES, add cross references (#4007)
 * doc: misc updates and fixes (#4009)
 * connector cleanup (#4013)
 * connectors: avoid embedded synchronous RPC for subscribe/unsubscribe
   (#3997)

### Test

 * testsuite: minor testsuite fixes (#4023)
 * ci: add ability to run tests under system instance (#3844)
 * fluxorama: allow user to sudo to flux user, add common systemd environment
   vars to flux user's bashrc (#4031)


flux-core version 0.32.0 - 2021-12-05
-------------------------------------

This release adds early groundwork for recovering running jobs across a
Flux restart.  It also includes improved log messages based on user feedback
about Flux failures on real workflow runs, a first draft of a new `flux top`
tool, and a critical fix for system deployments of Flux (#3958).

### Fixes

 * python: fix reference counting for Python Message objects (#3983)
 * python: avoid early garbage collection of Watcher objects (#3975)
 * libflux: improve safety against refcounting bugs in message functions (#3985)
 * shell: reject PMI clients that request v2 (#3953)
 * resource: don't abort if topo-reduce is received more than once (#3958)

### New Features

 * systemd: start flux systemd user service (#3872)
 * broker: record child instance URIs as job memo (#3986)
 * Support job memo events (#3984)
 * job-exec: checkpoint/restore KVS namespaces of running jobs (#3947)
 * set hostlist broker attribute when bootstrapped by PMI (#3966)
 * add `flux_get_hostbyrank()` and improve broker attribute caching (#3971)
 * broker: log slow nodes during startup (#3980)
 * add flux-top command (#3979)

### Cleanup

 * flux-overlay: improve default status output, rework options (#3974)
 * job-exec: improve job exception message/logging on broker disconnect (#3962)
 * drop flux-jobspec command (#3951)
 * improve flux-mini bulksubmit --dry-run output with --cc (#3956)
 * README: update LLNL-CODE (#3954)
 * broker/overlay: misc cleanup (#3948)
 * bring README.md up to date (#3990)
 * docker: fix and update ci dockerfiles (#3991)

### Test

 * testsuite: sanitize environment, fix hang in t2607-job-shell-input.t (#3968)

flux-core version 0.31.0 - 2021-11-05
-------------------------------------

This release includes two noteworthy system instance improvements:
crashed/offline nodes now marked offline for scheduling, and support
has been added for prolog/epilog scripts that run as root.

For prolog/epilog setup info, see the Flux Admin Guide:

https://flux-framework.readthedocs.io/en/latest/adminguide.html

### Fixes
 * build: allow python 3.10.0 to satisfy version check (#3939)
 * resource: avoid scheduling on nodes that have crashed (#3930)
 * broker: fail gracefully when rundir or local-uri exceed `AF_UNIX` path
   limits (#3932)
 * broker: ignore missing ldconfig when searching for libpmi.so (#3926)
 * job-manager: fix running job count underflow and use-after-free when an
   event is emitted in CLEANUP state (#3922)
 * fix problems building flux when 0MQ is not installed as a system package
   (#3917)
 * python: do not auto-stop ProgressBar by default (#3914)

### New Features
 * support job prolog/epilog commands (#3934)
 * job-manager: add prolog/epilog support for jobtap plugins (#3924)
 * libidset: add high level set functions (#3915)
 * kvs: optionally initialize namespace to a root reference (#3941)
 * rc: load job-archive module in default rc (#3942)

### Cleanup
 * improve broker overlay logging and debugging capability (#3913)
 * man: Add note about shell quoting/escaping (#3918)

### Test
 * mergify: set queue method to merge, not rebase (#3916)
 * mergify: replace strict merge with queue+rebase (#3907)
 * testsuite: fix non-bourne shell test failure (#3937)

flux-core version 0.30.0 - 2021-10-06
-------------------------------------

### Fixes

 * job-manager: small fixes for the alloc-bypass plugin (#3889)
 * job-manager: release after:JOBID dependencies after "start" instead of
   "alloc" event (#3865)
 * shell: avoid dropping stderr after a PMI abort (#3898)
 * shell: require `FLUX_SHELL_PLUGIN_NAME` in plugins to fix logging component
   discovery (#3879)
 * libflux: deactivate RPC message handlers after final response (#3853)
 * remove duplicate directories from `FLUX_RC_EXTRA`, `FLUX_SHELL_RC_PATH`
   (#3878)
 * t: fix incorrect method call in test-terminal.perl (#3888)
 * Fix a couple build and test issues on ppc64le with clang 6.0+ (#3875)

### New Features

 * jobtap: allow jobtap plugins to query posted events for jobs (#3863)
 * jobtap: allow jobtap plugins to subscribe to job events (#3861)
 * job-exec: enable manual override option for mock execution jobs (#3868)
 * shell: improve initrc extensibility, support version specific mpi plugin
   loading (#3890)
 * shell: fixes and enhancements for plugin loading (#3859)
 * shell: allow default rc path to be extended via `FLUX_SHELL_RC_PATH` (#3869)
 * shell: add taskids idset to `flux_shell_get_rank_info(3)` (#3873)
 * shell: add library of Lua convenience functions for use in plugins (#3856)
 * resource: fail get-xml request on quorum subset (#3885)

### Cleanup

 * libflux/future: fix comment typo (#3860)
 * NEWS.md: Fix typo for v0.29.0 (#3857)

### Testsuite

 * docker: add --build-arg to docker-run-checks, systemd-devel to centos8
   (#3871)
 * ci: add fedora 34 build and fix compiler errors from gcc 11.2 (#3854)


flux-core version 0.29.0 - 2021-09-03
-------------------------------------

This release of Flux includes a new fault mechanism which ensures that
unanswered RPCs receive error responses when the overlay network is
disrupted. Also included is a new `flux overlay` utility which can be
used to manage and report the status of the overlay network.

### Fixes
 * shell: fix in-tree pluginpath, add `shell_plugindir` (#3841)
 * python: fix bug in FluxExecutor.attach method (#3839)
 * rlist: fix erroneous collapse of nodes with different resource children
   when emitting R (#3814)
 * libkvs: compact only if ops len > 1 (#3807)
 * python: allow executor to attach to jobs (#3790)
 * python: fix version requirement in jobspec validator plugin (#3784)
 * broker: ensure subtree restart upon loss of router node (#3845)
 * broker: drop -k-ary option, rename tbon.arity attr to tbon.fanout (#3796)
 * add flux-job(1) manual page, plus minor fixes (#3763)
 * libjob: improve method for determining instance owner (#3761)

### New Features
 * add flux overlay status command (#3816)
 * broker: improve logging of 0MQ socket events (#3846)
 * broker: fail pending RPCs when TBON parent goes down (#3843)
 * broker: fail pending RPCs when TBON child goes down (#3822)
 * add `bootstrap.ipv6_enable = true` config option (#3827)
 * shell: add functions to access jobspec summary information (#3835)
 * add stats api for internal metric collection (#3806, #3824)
 * support io encode/decode of binary data (#3778)
 * add flag to bypass jobspec validation (#3766)
 * libflux: add time stamp to message trace (#3765)

### Cleanup
 * libzmqutil: generalize the zeromq authentication protocol server (#3847)
 * libflux: use iovec-like array over zmsg (#3773)
 * libflux: update flux msg route functions (#3746)
 * libflux: message API fixes and cleanup (#3771)
 * libjob: break up job.c (#3768)
 * build: consistently use CFLAGS / LIBS in Makefiles (#3785)
 * use CCAN base64 library over libsodium base64 library (#3789)
 * drop unnecessary 0MQ includes (#3782)
 * various other cleanup (#3762)

### Testsuite
 * update to flux-security v0.5.0 in docker images (#3849)
 * make valgrind test opt-in (#3840)
 * add valgrind suppression for opencl and libev on aarch64 (#3794, #3809)

flux-core version 0.28.0 - 2021-06-30
-------------------------------------

This release adds simple job dependencies - see the `flux_mini(1)`
DEPENDENCIES section.

### Fixes
 * shell: fix segfault when more slots are allocated than requested (#3749)
 * testsuite: avoid long `ipc://` paths in system test personality (#3739)
 * cron: fix race in task timeouts (#3728)
 * Python/FluxExecutor bug fixes (#3714)
 * flux-python: fix use of virtualenv python (#3713)
 * optparse: make `optional_argument` apply to long options only (#3706)
 * librlist: skip loading hwloc 'gl' plugin (#3693)

### New Features
 * allow jobs to bypass scheduler with alloc-bypass jobtap plugin (#3740)
 * libjob: add a library for constructing V1 jobspecs (#3662, #3734, #3748)
 * python: validate dependencies in Jobspec constructor (#3727)
 * libflux: make `flux_plugin_handler` topic member const (#3720)
 * job-manager: add builtin begin-time dependency plugin (#3704)
 * broker: send offline responses while broker is initializing (#3712)
 * python: add `flux.util.parse_datetime` (#3711)
 * job-manager: support simple `after*` job dependencies (#3696)
 * jobtap: fixes and api enhancements to support dependency plugins (#3698)
 * shell: add exit-on-error option (#3692)

### Cleanup/Testing/Build System
 * job-manager: minor cleanup and improvements for event handling (#3759)
 * libflux: make `flux_msg_fprint()` output clearer (#3742)
 * libflux: store fully decoded message in `flux_msg_t` (#3701, #3758)
 * libflux: msg API cleanup, test cleanup, and test enhancement  (#3745, #3699)
 * testsuite: generalize valgrind suppressions (#3743)
 * ci: use long path for builddir in test build (#3738)
 * job-list: cleanup & testsuite modernization & consistency updates (#3733)
 * testsuite: fix several tests on slower systems (#3730)
 * testsuite: fix intermittent test, speed up others (#3725)
 * broker: misc cleanup (#3721)
 * github: fix ci builds on master (#3716, #3717)
 * testsuite: add tests for late joining broker (#3709)
 * flux-start: build system instance test features (#3700)
 * ci: minor coverage testing fixes (#3703)
 * libflux: test: fix segfault of `test_plugin.t` under rpmbuild (#3695)

flux-core version 0.27.0 - 2021-05-28
-------------------------------------

This release features additional performance improvements that affect
job throughput over time (see issue #3583).

### Fixes
 * shell/pmi: always populate `PMI_process_mapping` to avoid mvapich2
   `MPI_Init` invalid tag error (#3673)
 * openmpi: ensure that shmem segments for co-located jobs don't conflict
   (#3672)
 * python: fix FluxExecutorFuture cancellation bug (#3655)
 * job-info, kvs-watch: support guest disconnect & credential checks (#3627)
 * libflux: plugin: make `FLUX_PLUGIN_ARG_UPDATE` the default (#3685)

### Performance
 * kvs: reduce cache expiration overhead (#3664)
 * kvs: remove client disconnect bottleneck (#3663)
 * kvs: use json object to find duplicate keys (#3658)
 * kvs: improve performance of transaction prep/check (#3654)
 * content-cache: avoid linear search for dirty blobs (#3639)
 * content-cache: make LRU purge more effective (#3632)
 * flux-shell: add minor optimizations for single node jobs (#3626)
 * libczmqcontainers: include zlistx, zhash, zlist, and convert internal
   users (#3620)

### New Features
 * shell: add plugin to detect first task exit (#3681)
 * job-manager: multiple jobtap plugin enhancements (#3687)
 * job-manager: support a list of loaded jobtap plugins (#3667)
 * shell: add tmpdir plugin to manage `FLUX_JOB_TMPDIR` (#3661)
 * jobtap: support for `job.dependency.*` callbacks (#3660)
 * flux-mini: avoid substitution without --cc/bcc, allow --setattr value
   to be read from file (#3659)
 * flux-start: add embedded server (#3650)
 * flux-proxy: add flux-core version check (#3653)
 * libflux: `msg_handler`: capture duplicate non-glob request handlers in a
   stack (#3616)

### Cleanup/Testing/Build System
 * testsuite: add mvapich2 to centos8 CI image (#3686)
 * testsuite: improve in-tree MPI testing (#3678)
 * libflux: `flux_modfind`: ignore DSOs with no `mod_name` symbol (#3675)
 * kvs: misc cleanup (#3671)
 * flux-start: rename `--scratchdir` to `--rundir` (#3670)
 * shell: misc environment-related fixes (#3669)
 * testsuite: modify jobid capture logic (#3657)
 * testsuite: handle hwloc issues and improve config file bootstrap test
   (#3648)
 * build: add and use autoconf variable for Flux plugin LDFLAGS (#3647)
 * libutil: replace hand written hex conversion code with libccan (#3646)
 * github: fixes for auto-release deployment (#3638)
 * content-cache: general cleanup, small bug fixes, and test improvement
   (#3645)
 * kvs: add errmsg on namespace create failure (#3644)
 * Use internal functions instead of zfile / zdigest (#3634)
 * libutil: avoid `zmq_strerror()` (#3628)
 * ci/test: switch to bulksubmit for inception tests, add throughput test,
   dismiss reviews after PR updates (#3621)
 * expand broker internal documentation to cover bootstrap phase (#3618)

flux-core version 0.26.0 - 2021-04-22
-------------------------------------

This release features several performance improvements that affect
job throughput over time (see issue #3583).

### Fixes

 * avoid mvapich segfault under flux start singleton (#3603)
 * python: avoid throwing 2nd exception on unknown errno (#3588)
 * avoid routing stale responses to restarted brokers (#3601)

### Performance

 * fix aggressive zhashx resize by importing fixed source (#3596, #3598)
 * use zhashx, LRU in content-cache (#3589, #3593)
 * drop root directory object from KVS setroot event (#3581)
 * add minor optimizations to aux container (#3586)
 * drop extra code in `flux_matchtag_free()` (#3590)
 * libkvs: save KVS copy/move aux data in future not handle (#3585)

### New Features

 * libjob: add `flux_job_result(3)` (#3582)
 * python: add explicit `service_(un)register` method (#3602)
 * add overlay network version/config check (#3597)
 * job-manager: enable job dependency management (#3563)

### Cleanup/Testing

 * flux-start: rename --size to --test-size, drop --bootstrap (#3605)

flux-core version 0.25.0 - 2021-04-01
-------------------------------------

### Fixes

 * kvs: fix assert due to busy KVS (#3560)
 * systemd: configure weak dependency on munge (#3577)
 * Fix various memleaks discovered by ASAN (#3568)
 * README: add missing dependency - pkgconfig (#3570)
 * fix `PMI_process_mapping` for multiple brokers per node (#3553)
 * Python: fix "no such file or directory" job exception resulting from
   bad jobspec (#3534)

### New Features

 * libflux: add `flux_plugin_aux_delete()` (#3565)
 * job-info: support LRU cache mapping job id -> owner (#3548)
 * python: expand FluxExecutor.submit parameters (#3562)
 * broker: add support for PMIx bootstrap (#3537)
 * job-ingest: add new plugin-based job validator (#3533)

### Cleanup/Testing

 * README.md: remove python3-six dependency (#3579)
 * clean up disconnect, cancel handlers (#3569)
 * broker: drop broker.rundir, select ipc vs tcp using broker.mapping (#3554)
 * broker: refactor overlay network send/receive interfaces (#3547)
 * github: add a stale issues and PR bot for flux-core (#3544)
 * build/test: remove stale heartbeat references (#3535)
 * job-info: consolidate watch RPC targets (#3525)
 * enhance testsuite reliability on RHEL8/TOSS4 (#3540)


flux-core version 0.24.0 - 2021-02-22
-------------------------------------

This release features multiple performance enhancements, including the
addition of the FluxExecutor Python class which allows rapid, asynchronous
submission of jobs.

### Fixes

 * broker: fix segfault/perf issues when hitting file descriptor limit (#3513)
 * module: reduce keepalive message traffic (#3516)
 * flux-kvs: fix --help output when not in an instance (#3500)
 * flux-kvs: fix help output in nested subcommands (#3497)
 * flux-mini: fix --progress counters with job exceptions (#3514)
 * portability: fix 32-bit issues (#3507)
 * portability: cross compilation fixes for Julia bindings (#3503)
 * libflux: restart continuation timeout in `flux_future_reset()` (#3518)

### New Features

 * python: add concurrent.futures executor (#3468)
 * libflux: add `flux_sync_create()` (#3524)
 * job-manager: allow jobtap plugins to reject jobs (#3494)
 * job-manager: support mode=limited (#3473)
 * flux-mini: support `--urgency` values "default", "hold", "expedite" (#3499)
 * broker: improve IP address heuristic in PMI bootstrap (#3489)
 * flux-mini: add --log and --log-stderr options (#3509)
 * use reactor time instead of heartbeats for internal time management (#3519)
 * heartbeat: convert to loadable module (#3512)

### Cleanup/Testing

 * job-info: split into two modules, job-info and job-list (#3510)
 * libflux: remove unnecessary `flux_future_then()` calls (#3520)
 * testsuite: cleanup job-manager tests (#3488)
 * testsuite: update hwloc-calc usage (#3523)
 * ci: add fedora33 docker image for testing (#3498)
 * ci: add 32 bit build to github ci checks (#3511)
 * ci: explicitly checkout tag if creating a release (#3531)


flux-core version 0.23.1 - 2021-01-27
-------------------------------------

### Fixes

 * flux resource: allow drain, undrain, and status to work on any rank (#3486)
 * job-manager: fix compilation error on gcc-10 (#3485)
 * job-manager: fix uninitialized variable warning in jobtap.c (#3481)

flux-core version 0.23.0 - 2021-01-25
-------------------------------------

This release adds a job priority plugin framework, enabling the
flux-accounting project to set job priorities with a fair share
algorithm.

The scheduler protocol (RFC 27) and libschedutil convenience API
have changed, therefore users of flux-sched must upgrade to 0.15.0.

### New features

 * jobtap: prototype job-manager plugin support (#3464)
 * flux-mini: add bulk job submission capabilities (#3426, #3478)
 * job-manager: send updated priorities to schedulers (#3442)
 * job-manager: support job hold and expedite (#3428)

### Fixes

 * connectors/ssh: forward `LD_LIBRARY_PATH` over ssh when set (#3458)
 * python: fix use of `Flux.reactor_run()` from multiple threads (#3471)
 * python: misc. fixes to docstrings and argument names in bindings (#3451)
 * python: fix circular reference in `check_future_error` decorator (#3437)
 * python: fix ctrl-c, re-throw unhandled exceptions in `reactor_run()` (#3435)
 * shell: fix dropped stdout from shell plugins in task.exec callback (#3446)

### Cleanup/Testing

 * ci: limit asan build to unit tests only (#3479)
 * libschedutil: API improvements and priority integration (#3447)
 * configure: add switch to allow flux to be built without python (#3459)
 * testsuite: remove sched-dummy, migrate testing to sched-simple (#3462)
 * testsuite: add debug, workarounds for failures in github actions (#3467)
 * test: fix test for installing poison libflux (#3461)
 * cleanup: update outdated terminology (#3456)
 * Globally standardize spelling of "canceled" (#3443)
 * ci: better script portability and other small updates (#3438)
 * testsuite: fix invalid tests, cleanup list-jobs, avoid hard-coding (#3436)
 * fix github actions on tag push (#3430)

flux-core version 0.22.0 - 2020-12-16
-------------------------------------

This release resolves an issue introduced in 0.20.0 where Flux would
occasionally hang during tear-down on RHEL/CentOS 7.  This release
should be suitable for use with production workflows on those systems.

System instance development and testing at < 256 node scale is on-going.
The system limitations of this release are documented in the Flux Admin
Guide:

https://flux-framework.readthedocs.io/en/latest/adminguide.html

### New features

 * flux-keygen is no longer necessary before starting Flux (#3409)
 * Add waitstatus and returncode JobID class and flux-jobs (#3414)
 * New `flux resource status` command (#3351)
 * Rename "administrative priority" to "urgency" (#3394)
 * Prepare for fair share priority plugin (#3371, #3339, #3350, #3402,
   #3405, #3404, #3410)
 * job-manager: cache jobspec for scheduler, exec (#3393, #3396, #3399)
 * python: add bindings for libflux-idset,hostlist (#3341)
 * resource: support hostnames for drain and exclude (#3318)
 * flux-jobs: Support nodelist in flux-jobs output (#3332)
 * flux-jobs: add flux-jobs --stats,--stats-only options (#3419)
 * flux-job: Add flux job attach --read-only option (#3320)
 * python: add ResourceSet python class (#3406)
 * python: allow future.then() variable and keyword args in callbacks (#3366)

### Fixes

 * Fix job shell segfault when jobspec contains JSON null (#3421)
 * job-manager: Fix annotation clear corner case #3418
 * broker: fix intermittent hang during instance tear-down on Centos7 (#3398)
 * job-exec: log early shell/imp errors (#3397)
 * shell: ensure TMPDIR exists for all jobs (#3389)
 * misc cleanups & fixups (#3392)
 * small fixes: resource memory leak, improve errors, check int size (#3388)
 * affinity: use comma separated list format for `CUDA_VISIBLE_DEVICES` (#3376)
 * libjob: repair interoperability with flux-security (#3356)
 * job-exec: fixes for multiuser mode (#3353)
 * shell: fix issues with `CUDA_VISIBLE_DEVICES` value (#3317)
 * job-manager: handle scheduler disconnect (#3304)
 * libjob: always sign owner jobs with the 'none' signing mechanism (#3306)
 * libsubprocess: do not allow ref/unref in hooks (#3303)

### Cleanup/Testing

 * doc: autogenerate python binding docs with Sphinx (#3412)
 * testsuite: support level N inception of flux testsuite (#3413)
 * github: fix missing docker tag in matrix builds (#3387)
 * github: fixes for workflow scripts (#3383)
 * ci: move from Travis-CI to GitHub Workflows (#3379)
 * docs: add explicit link to man pages section (#3365)
 * testsuite: replace loop in t2300-sched-simple.t with helper (#3367)
 * docker: install poison flux-core libs, cmds before build and test (#3369)
 * libflux: drop `flux_dmesg()` from public API (#3362)
 * testsuite: fix shed-simple test races (#3358)
 * build: allow Lua 5.4, drop -lutil, and improve sphinx warning (#3357)
 * testsuite: increase resource.monitor-waitup timeout (#3348)
 * broker: update log.dmesg to use rpc streaming (#3307)
 * testsuite: fix core idsets in resource module tests (#3314)
 * t/t2205-hwloc-basic: only use lstopo-no-graphics (#3309)

flux-core version 0.21.0 - 2020-11-04
-------------------------------------

This release enables resources to be configured in advance when Flux is
the native resource manager for a cluster, in lieu of dynamic discovery.
For details, refer to the Flux Admin Guide:

https://flux-framework.readthedocs.io/en/latest/adminguide.html

### New features

 * optparse: don't sort options/subcommands by default (#3298)
 * flux-job: Output key options for job info (#3210)
 * resource: load resources from config or R, and rework topo discovery (#3265)
 * add internal librlist library and flux-R utility for R version 1 (#3276)
 * job-info: use job manager journal to track job state (#3254)
 * job-manager: support events journal (#3261)
 * shell: support stdio buffering options (default stderr: unbuffered) (#3272)
 * flux-kvs: Add 'flux kvs eventlog wait-event' subcommand (#3200)
 * job-manager: send job annotations to journal instead of publishing (#3236)
 * add hostlist library for encoding/decoding RFC29 hostlists (#3247)

### Fixes

 * broker: convert broker [bootstrap] config to use libhostlist (#3283)
 * libflux: Add missing C++ header guards (#3280)
 * cmd: display jobid with flux-mini alloc -v, --verbose (#3279)
 * python: fix signal handler management in threads (#3266)
 * rc1: fix local connector retries (#3301)

### Cleanup

 * remove flux-hwloc reload command and aggregator module (#3296)
 * doc: add flux-jobs(1) examples (#3295)
 * job-manager / job-info: misc cleanup (#3246)
 * build: increase minimum version of jansson to 2.10 (#3240)
 * ci: ensure pylint script fails when lint warnings are produced (#3269)


flux-core version 0.20.0 - 2020-09-30
-------------------------------------

This release features changes to support Flux as the native resource
manager on small (<= 256 node) clusters, for testing only.  A draft system
administration guide is available at:

https://flux-framework.readthedocs.io/en/latest/adminguide.html

### New features

 * hwloc: add printing of num GPUs to `flux hwloc info` (#3217)
 * resource: mark nodes down when they are stopped (#3207)
 * broker:  allow late-joining brokers, execute rc1/rc3 on all ranks (#3168)
 * shell/pmi: add improved PMI key exchange mechanism (#3219)

### Fixes

 * job-manager: communicate job priority changes to job-info (#3208)
 * job-info: handle annotations race (#3196)
 * python/job: Update `state_single` default header (#3227)
 * libidset: reject idset strings that don't conform to RFC 22 (#3237)
 * job-info: handle job-priority changes (#3208)
 * doc: list sphinx as a doc dependency in README.md (#3225)
 * testsuite: fix race in python SIGINT test (#3224)
 * job-manager: fix segfault changing priority of a running job (#3220)
 * shell: allow multiple resources per level in jobspec (#3175)
 * python: allow Ctrl-C interrupt of `Future.get()` and `wait_for()` (#3215)
 * shell: use F58/alternate encodings in output file template {{id}} (#3206)
 * fallback to ASCII for F58 FLUIDs with `FLUX_F58_FORCE_ASCII` (#3204)
 * rc: load sched-simple only if no other scheduler is loaded (#3177)
 * docker: do not install Sphinx via pip in Centos8 image (#3195)
 * flux-jobs / python bindings: handle empty string conversions (#3183)

### Cleanup

 * reduce log noise (#3226)
 * flux-comms: remove obsolete command (#3211)


flux-core version 0.19.0 - 2020-08-31
-------------------------------------

Notable features and improvements in this release include enhanced
support for tools/debuggers (e.g. STAT, LaunchMON and TotalView), a
new set of `--env` environment manipulation options for flux-mini(1),
better support for listing jobs through the Python API, and a fix
for an annoying usability issue with F58 encoded jobids in non-UTF-8
locales.


### New features

 * switch to utf-8 for subprocess and job io encoding (#3086)
 * improve support for shell plugin developers (#3159, #3132)
 * flux-mini: add environment manipulation options (#3150)
 * flux-mini: add --debug option for tools support (#3130)
 * bash: top level command completions for flux (#2755)
 * add fluxorama system instance docker image sources (#3031, #3128)
 * content-s3: add configuration, support for libs3 variants (#3067, #3115)
 * Use F58 JOBIDs in most user-facing commands (#3111)
 * broker: state machine refactoring (#3107)
 * broker: restore client-side PMI logging (#3105)
 * libflux: add `flux_module_set_running()` (#3104)
 * python: Add JobInfo, JobInfoFormat, and JobList classes (#3174)

### Fixes

 * Fix F58 encoding in non-multibyte locales (#3144)
 * job-info,job-shell: allow non-V1 jobspec (#3160)
 * build: fix innocuous configure error (#3129)
 * travis-ci: fix ARGS when `DOCKER_TAG` set (#3125)
 * doc: fix flux-help(1) output and rendering of NODESET.rst (#3119)
 * flux-job: add `totalview_jobid` support and misc. fixes (#3130)
 * small build/test/doc fixes (#3100)
 * fix GitHub project license detection (#3089)
 * shell/lua.d/openmpi: set env vars to force the use of flux plugins (#3099)
 * job-info: do not fail on invalid jobspec / R / eventlog (#3171)
 * flux-module: extend first column of flux-module list output (#3178)

### Cleanup

 * python: split flux.job module into multiple files (#3162)
 * python: reformat with latest black formatter, pin black version (#3169)
 * libflux: fix comment in module.h to reference readthedocs (#3138)
 * Update rfc links to RTD site (#3137)
 * remove the simple dynamic string (sds) code from libutil (#3135)
 * Doc Cleanup (#3117)
 * AUTHORS: remove (#3090)

flux-core version 0.18.0 - 2020-07-29
-------------------------------------

This release features a more compact default representation for Flux JOBIDs,
manual pages converted to ReST format and published on
[readthedocs.io](https://flux-framework.readthedocs.io/projects/flux-core/),
and the ability for schedulers to add data to jobs which can be displayed
with `flux jobs`.

### New features

 * doc: man pages converted to ReST for publication on readthedocs.io
   (#3033, #3078, #3085)
 * Add support for RFC19 F58 encoded JOBIDs (#3045)
 * Support user and scheduler job annotations (#3065, #3062, #2960)
 * add content-s3, content-files alternate backing stores (#3025, #2992)
 * Python interface to 'mini batch' (#3020)

### Fixes

 * shell: fix bug in cpu-affinity=per-task (#3080)
 * flux-hwloc: remove ignore of `HWLOC_OBJ_GROUP` (#3046)
 * cmd: Make label io options consistent (#3068)
 * flux-resource list: Allow null/missing key to designate empty set (#3047)
 * flux-jobs: small functionality and testing updates (#3060)
 * job-manager: avoid segfault on priority change with pending alloc (#3072)

### Cleanup

 * doc: adjust dependency table to reflect hwloc v2.0+ support (#3053)
 * Update terminology to use more inclusive words (#3040)

### Testsuite enhancements

 * testsuite: remove use of -S option in `run_timeout` (#3079)
 * testsuite: minor valgrind test cleanup (#3077)
 * docker: small updates for testenv images, travis builds (#3058)
 * travis-ci: add python coverage (#3056)
 * travis-ci: Add `--localstatedir=/var` to docker tag builds (#3050)
 * pylint: Update pylint to 2.4.4 (#3035)
 * Fix testsuite for Lua 5.3 on Ubuntu 20.04 (#3028)
 * docker: really actually fix Ubuntu 20.04 (focal) docker tags (#3027)
 * travis-ci: enforce correct configure ARGS for docker tags (#3023)
 * travis: tag a docker image for ubuntu 20.04 (#3022)
 * python: add stdio properties to Jobspec class (#3019)
 * build and test fixes (#3016)


flux-core version 0.17.0 - 2020-06-18
-------------------------------------

*NOTE*: Support has been removed for Python 2.

### New features

 * Improved interface for batch jobs: `flux mini batch` and `flux mini alloc`
   (#2962)
 * Pty support for Flux jobs via `-o pty` shell option (#2894)
 * New resource module for monitoring and control of resources,
   including ability to exclude and drain/undrain ranks. (#2918, #2949)
 * New `flux resource` utility to drain and list resources. (#2949)
 * Multiple improvements for `flux jobs`: colorize output, add "status"
   and "exception" fields, allow jobids as positional arguments, and
   add a custom conversion type `h` for "-" (#2798, #2858, #2902, #2910,
   #2940, #2926, #2865)
 * Support for hwloc v2.0+ (#2944)
 * Support for MPIR debugging of jobs (#2654)
 * New job-archive module optionally stores job data in sqlite. (#2880)
 * single-broker system instance support, including minimal
   support for restart (archived job information is saved) (#2783, #2820,
   #2813, #2809)
 * Add support for multi-user execution (#2822, #2813)
 * Add support for enforcing job time limits (#2995)
 * python: Add bindings for job cancel and kill (#2976)
 * python: Add bindings for watching job eventlog events (#2986)

### Improvements

 * support systemctl reload flux (#2879)
 * enhance job throughput (#2777, #2792)
 * sched-simple: schedule cores instead of PUs by default (#2966)
 * broker: send service.disconnect requests on module unload (#2913)
 * broker: add interface for monitoring broker liveness (#2914)
 * broker: add cleanup phase (#2971)
 * broker: only allow userid- services to be registered by guests (#2813)
 * libflux: add `flux_msg_last_json_error(3)` (#2905)
 * flux-job: Use common attrs for list cmds (#2901)
 * doc: add flux job shell API manpages (#2793)
 * job-info: Support "exception" and "success" list attributes (#2831, #2858)
 * job-info: improve error responses from various list RPCs (#3010)
 * rc: load job-info on rank 0 only (#3009)
 * python: remove support for Python 2 (#2805)
 * python: cache python wrappers in the class (#2878)
 * python: tweaks in preparation for flux-tree-helper (#2804)
 * python: add 'flux_job_list_inactive' Python binding (#2790)
 * python: allow reactor_run() to be interrupted (#2974)
 * config: parse TOML once in broker, share with modules (#2866)
 * config: use config file for access policy (#2871)
 * docker: add default PS1 that includes flux instance size, depth (#2925)
 * docker: start munge in published docker images (#2922)

### Fixes

 * Fix compilation under GCC 10.1.0 (#2954)
 * librouter: avoid dropping messages on EPIPE (#2934)
 * README: update documentation link (#2929)
 * README.md: fix required Lua version (#2923)
 * README: add missing dependencies: aspell-en and make (#2889)
 * shell: make registered services secure by default (#2877)
 * cmd/flux-kvs: Fix segfault in dir -R (#2847)
 * job-exec: drop use of broker attrs, use conf file or cmdline instead
   (#2821)
 * broker: clean shutdown on SIGTERM (#2794)
 * flux-ping: fix problems with route string (#2811)
 * libsubprocess:  don't clobber errno in destructors, handle ENOMEM (#2808)
 * Fix flux-job status for jobs with exceptions before start (#2784)
 * shell: Add missing R member to shell info JSON object (#2989)
 * job-ingest: fix validation of V1 jobspec (duration required) (#2994)
 * doc: fixes and updates for idset manpages (#3012)

### Cleanup

 * removed outdated pymod module (#3008)
 * broker and flux-comms cleanup (#2907)
 * cmd/flux-kvs: Remove legacy --json options and json output (#2807)
 * doc: Fix typos in man pages (#2725)
 * libutil: improve out of memory handling, conform to RFC 7 (#2785)
 * content-sqlite, content-cache: cleanup and refactoring (#2786)

### Testsuite enhancements

 * Fix skipped tests in t2205-hwloc-basic.t (#2998)
 * t2204-job-info: Split up tests into new files (#2957)
 * t/t2800-jobs-cmd: Fix racy test (#2951)
 * t: add `HAVE_JQ` prereq to tests that use `jq` (#2936)
 * sharness: fix TEST_CHECK_PREREQS for tests using $jq (#2939)
 * job-info: module & test cleanup (#2932)
 * testsuite: add ability to ensure programs are used under appropriate
   prereqs (#2937)
 * ensure unit tests do not link against installed flux libraries (#2917)
 * t2204-job-info: Fix racy tests (#2862)
 * test rehab: new flexible run_timeout, speeding up asan, and many more
   timeouts and test repairs (#2849)
 * Mypy: add static type checking for python to travis (#2836)
 * testsuite: minor fixes and slight improvements (#2842)
 * README: update Travis CI badge after transition to travis-ci.com (#2843)
 * tests: timeout in automake harness (#2840)
 * t/t0005-exec: Increase timeout lengths (#2828)


flux-core version 0.16.0 - 2020-02-24
-------------------------------------

## New features

 * job-info: fix ordering of pending jobs (#2732)
 * job-info: add list-id service for race-free listing of 1 jobid (#2720)
 * sched-simple: add unlimited alloc mode (#2726)
 * flux-module: add `flux module reload` subcommand (#2736)
 * flux-queue: add `flux queue idle` subcommand (#2712)

## Improvements

 * broker: rework shutdown: rc3 no longer under grace-time timeout (#2733)
 * broker: log dropped responses sent down overlay (#2761)
 * libflux: fulfill empty "wait_all" futures immediately (#2714)
 * libflux: allow anonymous futures in `flux_future_push(3)` (#2714)
 * shell: report meaningful exit codes for ENOENT, EPERM & EACESS (#2756)
 * flux-jobs: refactor using new JobInfo and OutputFormat classes (#2734)
 * python: accept integer job duration (#2702)
 * python: switch from flags to boolean args in job.submit(), submit_async()
   (#2719)
 * python: return derived JobListRPC and JobWaitFuture objects from
   job.job_list and job.wait,wait_async for a better interface (#2753)

## Fixes

 * broker: fix bootstrap under openpmix PMI-1 compat library (#2748)
 * broker: mute modules during unload to avoid deadlock (#2710)
 * libflux: block `flux_send()` during handle destruction (#2713)
 * job-ingest: fixes for validation worker management (#2721, #2716)
 * build: fix compilation errors on clang < 6.0 (#2742)
 * testsuite: fix tests when run under Slurm and Flux jobs (#2766)
 * testsuite: fix for hangs in tests using rc3-job (#2744)
 * doc: fix URI format in flux-proxy(1) manpage (#2747)


flux-core version 0.15.0 - 2020-02-03
-------------------------------------

## Summary:

This release fixes a critical issue (#2676) with `flux module remove` in
flux-core-0.14.0 that causes rc3 to fail when flux-core is integrated
with flux-sched.

### New features

 * flux-job: add raiaseall, cancelall, killall (#2678)
 * flux-queue: new command to control job queue (#2659, #2687)
 * flux-jobs: support listing `nnodes` and `ranks` (#2656, #2705)

### Improvements

 * shell: expand lua api to improve error handling in shell rc scripts (#2699)
 * shell: improve error messages to users on exec failures (#2675)
 * flux-job: (attach) fetch log messages even when shell init fails (#2691)
 * flux-job: (attach) add `-v` option (adds file,line log messages) (#2691)
 * flux-job: (list) make filtering options match `flux jobs` (#2639)
 * flux-job: (list) make JSON output the default (#2636)
 * flux-job: (drain,undrain) drop subcommands (see flux queue) (#2659)
 * job-info: transition state _after_ retrieving data from KVS (#2655)
 * job-info: add checks in sharness test to avoid racyness (#2666)
 * job-info: rename attributes to ease parsing (#2643)
 * flux-jobs: add --from-stdin option and other small fixes (#2648)
 * python: allow JobspecV1 to accept 0 gpus_per_task (#2701)
 * optparse: always display `--help` usage first in command help output (#2691)
 * libflux: add message cred helpers (#2670)
 * github: check flux-sched@master against submitted flux-core PRs (#2680)

### Fixes

 * shell: fix bad exit from mvapich rc script, avoid flux.posix in rcs (#2699)
 * shell: fix race between stdin/out readers and eventlog creation (#2688)
 * shell: install `shell.h`: the public api for shell plugins (#2690)
 * shell: `chdir()` into current working directory (#2682)
 * rc: improve rc3 reliability, add `flux module remove -f` option  (#2676)
 * testsuite: fix unsafe getenv in libpmi tests, /tmp usage in sharness (#2669)
 * job-manager: fix counting problem that leads to scheduler sadness (#2667)


flux-core version 0.14.0 - 2020-01-12
-------------------------------------

## Summary:

This version of flux-core improves the reliability and performance
of the new execution system, and fills gaps in the previous release.
Some highlights are:

 * support for jobs reading standard input
 * improved job listing tool - see flux-jobs(1)
 * improved python support for building jobspec and waiting for job completion
 * ability to override job names displayed in listing output

### New features

 * Add porcelain `flux jobs` command (#2582)
 * job-info: use basename of arg0 for job-name (#2598)
 * job-info: honor `max_entries` option in job-info.list (#2596)
 * job-info: Support task-count in listing service (#2580)
 * Support job state times in job listing service (#2568)
 * python: add jobspec classes to main bindings (#2534)
 * initial job-name support (#2562)
 * job-manager: add `flux_job_wait()` (#2546)
 * shell: add support for debugger synchronization and `MPIR_proctable` gather
   (#2542)
 * job-info: Add stats for number of jobs in each state (#2540)
 * job-info: re-load job state from KVS (#2502)
 * libflux: add `flux_get_conf()` (#2501)
 * job-info: Store full job-history, allow users to query pending, running,
   and inactive jobs (#2471)
 * Initial shell stdin support (#2448)

### Improvements

 * libflux/mrpc: drop the mrpc class (#2612)
 * docker: add image and travis tests on CentOS 8 (#2610)
 * mergify: do not auto-merge PRs with invalid commits (#2603)
 * broker: new format for [bootstrap] configuration (#2578)
 * broker/boot_config: use new config file interfaces (#2524)
 * shell: add unpack-style helpers for get_info shell plugin api calls
   (#2573)
 * testing/asan: enable asan in test framework and travis-ci (#2466)
 * README.md: update build docs for Python 3 (#2565)
 * Update jobspec command key per RFC 14 changes (#2564)
 * replace exec "running" event with "shell.init" and "shell.start" (#2541)
 * shell: improve stdout/stderr performance (#2531)
 * modules/job-manager:  [cleanup] simplify queue listing and refactor
   internal context (#2536)
 * kvs: improve append performance (#2526)
 * shell: generate job exception on `MPI_Abort` (#2510)
 * `msg_handler`: make `topic_glob` `const char *`, fix fallout (#2496)
 * libflux: fall back to builtin connector search path (#2489)
 * README: minor source cleanup (#2509)
 * shell: implement shell-specific log facility, add support for log events
   to output eventlog (#2477)
 * flux-mini: improve handling of `--setattr` and `--setopt` (#2495)
 * bindings/python: reinstate python2 support (#2482)
 * bindings/python: change minimum python version to 3.6 (#2452)
 * libutil: replace fdwalk with version that uses getdents64 (#2479)
 * flux-shell: handle jobspec command as bare string (#2484)
 * librouter: factor common code from connector-local, flux-proxy (#2354)
 * mergify: fix rule that prevents merging of "WIP" PRs (#2453)
 * buffer: start buffer at 4K and grow to 4M as necessary (#2449)
 * libioencode: make rank parameters strings (#2441)
 * flux-kvs: Add eventlog namespace option (#2439)
 * testsuite: fix LONGTEST and other small improvements (#2444)
 * job-ingest: switch to v1 schema (#2433)
 * libtomlc99: update for TOML v0.5.0 support #2619
 * job-ingest: switch to a py bindings based jobspec validator (#2615)

### Fixes

 * flux-job: misc fixes for attach (#2618)
 * fix minor issues found by lgtm scan (#2605)
 * broker: increase nofile limit to avoid assertion failure in `zuuid_new()`
   (#2602)
 * use libuuid instead of zuuid (#2606)
 * github: enable a workflow to validate commits in a PR (#2586)
 * python: fix circular reference in `Future` class (#2570)
 * have future take a ref on `flux_t` handle (#2569)
 * bindings/python and libev: work around future leak (#2563)
 * kvs: Fix duplicate append corner case (#2555)
 * shell: stdin write to exited task should not cause fatal job exception
   (#2550)
 * job-manager: fix internal job hash lookup error handling (#2552)
 * shell: fix segfault if logging function is called in or after
   `shell_finalize()` (#2544)
 * kvs: fix memory use-after-free corner case (#2525)
 * t: fix tests prone to races or timeouts on constrained systems (#2523)
 * job-exec: fix memory errors detected by valgrind (#2521)
 * test: fix random cronodate test failure (#2520)
 * t1004-statwatcher: fix test on Ubuntu 19.10 (#2513)
 * job-ingest: launch `.py` validators with configured python (#2506)
 * doc: `flux_respond_raw` doesn't take an errnum (#2504)
 * Fix infinite recursion when wrapper.Wrapper object initialized with
   incorrect args (#2485)
 * sched-simple: fix `rlist_alloc_nnodes()` algorithm (#2474)
 * fix crash in `is_intree()` with EACCESS or ENOENT from builddir (#2468)
 * testsuite: extend some testing timeouts  (#2451)


flux-core version 0.13.0 - 2019-10-01
-------------------------------------

## Summary:

This version of flux-core enhances the new execution system to near full
functionality, including new tools for job submission, better MPI support,
task and GPU affinity options, and flexible job output handling including
redirection to bypass the KVS. A powerful shell plugin infrastructure allows
execution features to be selectively enabled by users.

See flux-mini(1) for more info on the new job submission interface.

Some deficiencies present in this release:

 * flux job list doesn't show inactive jobs
 * no per-task output redirection
 * output is space-inefficient in KVS (base64 encoding, one commit per line)
 * no stdin redirection
 * need better shell task cleanup and early task exit detection
 * no debugger support (MPIR)

### New features

 * flux-mini: new run/submit interface (#2409, #2390)
 * flux-version: make flux -V,--version an alias, add manpage (#2412, #2426)
 * shell: add gpu affinity support (#2406)
 * shell: add builtin core affinity plugin (#2397)
 * shell: Support stdout/stderr redirect to a file (#2395)
 * shell: add support for plugins and shell initrc (#2376, #2392, #2357, #2335)
 * shell: flush output to KVS on every line (#2366, #2332)
 * shell: limit the number of I/O requests in flight (#2296)
 * shell: use RFC 24 eventlog output (#2308)
 * flux-job attach: add timestamps, --show-exec option (#2388)
 * libioencode: convenience library for encoding io (#2293)
 * libsubprocess: add start/stop for streams (#2271, #2333)
 * libsubprocess: add `flux_subprocess_kill()` (#2297)
 * job-info: development in support of job output (#2341, #2374, #2360,
   #2303, #2307)
 * flux-in-flux: flux --parent option, add `instance-level`, `jobid`
   broker attributes (#2326, #2362)
 * flux-in-flux: set `local_uri`, `remote_uri` in enclosing instance KVS (#2322)

### Improvements

 * libflux/reactor: add `flux_reactor_active_incref()`, `_decref()` (#2387)
 * libflux/module: add `flux_module_debug_test()` (#2373)
 * libschedutil: export library for use by flux-sched and others (#2380)
 * libschedutil: destroy pending futures on scheduler unload (#2226)
 * libflux/message: drop `flux_msg_sendfd()`, `_recvfd()` from API (#2375)
 * libflux/message: add `flux_msg_incref()` and `_decref()` (#2334)
 * libflux: update message dispatch to support routers (#2367)
 * libflux/buffer: increase efficiency of line buffered I/O (#2294)
 * libsubprocess: cleanup ( #2343, #2286)
 * testsuite improvements (#2404, #2400)
 * build system cleanup (#2407)
 * documentation cleanup (#2327)
 * abstract in-tree detection into libutil (#2351)
 * libjob: `flux_job_kvs_namespace()` (#2315)

### Fixes

 * build: bump libflux-core.so version to 2 (#2427)
 * sched-simple: reject requests with unknown resource types (#2425)
 * restore libpmi2 to support MPICH/MVAPICH configured for slurm pmi2 (#2381)
 * broker: avoid accidentally consuming % format characters in initial
   program args (#2285)
 * connector-local: suppress EPIPE write errors (#2316)
 * libidset: fix `idset_last()` at size=32 (#2340, #2342)
 * connectors/loop: do not accidentally close STDIN (#2339)
 * job-exec: fix exception handling of jobs in transition states (#2309)
 * broker: don't read `FLUX_RCX_PATH` to set rc1,rc3 paths (#2431)
 * job-ingest: validator shebang can pick the wrong python (#2435)


flux-core version 0.12.0 - 2019-08-01
-------------------------------------

## Summary:

This version of flux-core replaces the old execution prototype, "wreck",
with a new job submission and execution protocol. The new system does
not yet have support for all the features of the prototype, however it
is capable of running jobs specified in version 1 jobspec format with
an advanced and performant job submission API.

For early adopters:
 
 * To generate jobspec, see `flux jobspec`
 * To submit jobspec, see `flux job submit`
 * Instead of `flux wreckrun` try `flux srun`
 * Instead of `flux wreck ls` try `flux job list`
 * Instead of `flux wreck kill/cancel` try `flux job kill/cancel`
 * Job events are recorded detailed eventlog, see `flux job eventlog <id>`
 * Experience job synchronization with `flux job wait-event`
 * Attach to submitted jobs with `flux job attach`
 * Want info about a job? Try `flux job info`
 * Waiting for all jobs to complete? Try `flux job drain`

### New Features:

 * new job submit api and `flux job submit` command (#1910, #1918)
 * add job exception and cancellation support (#1976)
 * support validation for submitted jobspec (#1913, #1922)
 * add `flux jobspec` jobspec generation script (#1920, #1964)
 * add a simple default node/core fcfs scheduler, sched-simple
   (#2038, #2053, #2203)
 * add `flux job info`, `eventlog`, `wait-event`, `attach`
   (#2071, #2085, #2098, #2112, #2114, #2115, #2137, #2142, #2269, #2084)
 * add `flux job drain` (#2092)
 * add flux-shell, the flux job shell (#2211, #2240, #2246, #2244, #2278)
 * add `flux srun` (#2179, #2227)

### Improvements:
 
 * libsubprocess updates (#2158, #2152, #2167, #2174, #2230, #2254, #2262,
    #2265)
 * job-manager: add exec and scheduler interfaces, add job state machine:
    (#2025, #2031, #2067, #2068, #2077, #2146, #2198, #2231)
 * job-manager: add state transition events (#2109)
 * job-manager: other improvements (#2047, #2062)
 * replace resource-hwloc module (#1968)
 * standardize parsing of duration in most tools (#2095, #2216)
 * add guest support to barrier module (#2215)
 * add broker `rundir` attribute (#2121) 
 * kvs: remove namespace prefix support (#1943)
 * kvs: support namespace symlinks (#1949)
 * kvs: new kvs namespace command (#1985)
 * python: add futures support (#2023)
 * improve signature of `flux_respond` and `flux_respond_error` (#2120)

### Fixes

 * misc build and test system fixes (#1912, #1914, #1915, #1925, #1941,
    #2004, #2014, #2019, #2022, #2028, #2034, #2037, #2058, #2104, #2133,
    #2124, #2177, #2221, #2128, #2229)
 * misc flux-broker fixes (#2172, #2178, #2175, #2181, #2194, #2197, 
 * misc kvs fixes (#1907, #1936, #1945, #1946, #1966, #1949, #1965, #1969,
    #1971, #1977, #2011, #2016, #2018, #2056, #2059, #2064, #2126, #2130,
    #2136, #2138)
 * remove kvs classic library (#2017)
 * misc python fixes (#1934, #1962, #2046, #2218)
 * misc libflux-core fixes (#1939, #1942, #1956, #1982, #2091, #2097, #2099,
    #2153, #2164)
 * do not version libpmi*.so (#1992)
 * ensure system python path is not pushed to front of PYTHONPATH (#2144)
 * flux-exec fixes: (#1997, #2005, #2248)
 * libpmi fixes (#2185)
 * libidset fixes (#1928, #1975, #1978, #2060)
 * jobspec fixes and updates (#1996, #2081, #2096)
 * other fixes (#1989, #2090, #2151, #2280, #2282)

 
flux-core version 0.11.0 - 2019-01-03
-------------------------------------

### Fixes
 * flux-module: increase width of size field in list output (#1883)
 * kvs: return errors to callers on asynchronous load/store failures (#1836)
 * flux-start: dispatch orphan brokers, fully clean up temp directories (#1835)
 * flux-exec: ensure stdin is restored to blocking mode on exit (#1814)
 * broker: don't connect to enclosing instance (#1798)
 * flux (command): handle inaccessible build directory, fix PATH issue (#1683)
 * wreck: fix incorrect error handling in job module (#1617)
 * libflux: improve efficiency of asynchronous futures (#1840)
 * libflux: fix composite future implementation (#1791)
 * libflux: improve lookup efficiency of RPC message handlers (#1807)
 * libflux: give all aux set/get interfaces uniform semantics (#1797)
 * update to libev 4.25, ensure valgrind runs clean on i686 (#1898)

### New Features
 * license: re-publish project under LGPLv3 (#1829, #1788, #1901)
 * wreck: use direct stdio transport, unless -okz option (#1875, #1896, #1900)
 * wreck: add new -J, --name=JOBNAME option to flux-wreckrun and submit (#1893)
 * libflux: support queue of future fulfillments (#1610)
 * libflux: support dynamic service registration (#1753, #1856)
 * kvs: replace inefficient KVS watch implementation and outdated API (#1891,
   #1890, #1882, #1878, #1879, #1873, #1870, #1868, #1863,
   #1861, #1859, #1850, #1848, #1820, #1643, #1622)
 * job: add job-ingest, job-manager modules, and API (experimental)
   (#1867, #1774, #1734, #1626)
 * libidset: expand API to replace internal nodeset class (#1862)
 * libflux: add KVS copy and move composite functions (#1828)
 * libflux: access broker, library, command versions (#1817)
 * kvs: restart with existing content sqlite, root reference (#1800, #1812)
 * python: add job & mrpc bindings (#1757, #1892)
 * python: add flux python command to run configured python (#1766)
 * python: add flux-security bindings (#1716)
 * python: Python3 compatibility (#1673)
 * kvs: add RFC 18 eventlog support (#1671)
 * libsubprocess: cleanup and redesign
   (#1713, #1664, #1659, #1658, #1654, #1645, #1636, #1629)
 * libflux/buffer: Add trimmed peek/read line variants (#1639)
 * build: add library versioning support (#1874)
 * build: add support for asciidoctor as manpage generator (#1650, #1676)
 * travis-ci: run tests under docker (#1688, #1684, #1670)

### Cleanup
 * libflux: drop broker zeromq security functions from public API (#1846)
 * libflux: clean up interface for broker attributes (#1845)
 * libflux: drop reduction code from public API (#1844)
 * libutil: switch from munge to libsodium base64 implementation (#1786)
 * python: python binding is no longer optional (#1772)
 * python: add "black" format check, and reformat existing code (#1802)
 * python/lua: avoid deprecated kvs functions (#1748)
 * kvs: misc cleanup, refactoring, and fixes
   (#1805, #1813, #1773, #1764, #1712, #1696, #1694)
 * broker: drop epgm event distribution (and munge dependency) (#1746)
 * content-sqlite: switch from lzo to lz4 (#1740)
 * libpmi: drop PMIx client support (#1663)
 * libpmi: avoid synchronous RPCs in simple-server kvs (#1615)
 * modules/cron: misc cleanup (#1657)
 * RFC 7: fix various style violations (#1705, #1717, #1706, #1611)
 * gcc8: fix output truncation (#1642)
 * sanitizer: fix memory leaks (#1737, #1736, #1739, #1737, #1735, #1733)
 * build: misc. cleanup and fixes (#1886, #1795, #1824, #1827, #1701, #1678)
 * test: misc. cleanup and fixes (#1644, #1704, #1691, 1640)


flux-core version 0.10.0 - 2018-07-26
-------------------------------------

### Fixes
 * fix python kz binding errors (#1537)
 * fix default socket path and config file parsing for flux-broker (#1577)
 * Lua 5.2 compatibility and other Lua fixes (#1586, #1594)
 * flux PMI server response before closing (#1528)

### New Features
 * support cpu affinity for wreck jobs (#1533, #1603)
 * support for GPU device discovery through hwloc (#1561)
 * set CUDA_VISIBLE_DEVICES for jobs with GPUs (#1599)
 * add ability to bootstrap Flux using pmix (#1580)
 * add `flux wreck sched-params` cmd to tune scheduler at runtime (#1579)
 * support `-o mpi=spectrum` for spectrum MPI launch (#1578, #1588)
 * allow generic JSON values in aggregator (#1535)
 * new --wrap=arg0,arg1 option to flux-start (#1542)
 * allow arbitrary error strings in RPC responses (#1538)
 * support for composite flux_future_t types (#1553)
 * add buffered I/O support to Flux API (#1518, #1547, #1548)
 * remove extra line breaks in Flux log messages (#1530)
 * add Flux Locally Unique ID (FLUID) implementation (#1541)

### Cleanup
 * remove json-c (#1522, #1524, #1525, #1527, #1529)
 * libidset internal cleanup (#1521)
 * libsubprocess cleanup (#1549)
 * drop PMIx heuristic in libpmi (#1575)
 * add missing `#!/bin/bash` to all rc1 scripts (#1597)

flux-core version 0.9.0 - 2018-05-10
------------------------------------

### Fixes
 * numerous memory leak fixes (#1494, #1322)
 * better support for C++ code (#1225, #1223, #1204)
 * massive scalability improvement for libkz readers (#1411, #1424)
 * increase job submission throughput (#1472, #1389)
 * reduce amount of information collected in resource-hwloc to
    enhance large instance startup (#1457)
 * i686 portability fixes (#1296)
 * fixes for `flux-kvs dir` and `ls` usage (#1444, #1452)
 * fix for clock_gettime workaround in Lua bindings (#1371)
 * update minimum libhwloc to 1.11.1 to avoid assertion failure (#1478)
 * fix incorrect output from option parsing when invalid short
    option is grouped with valid options in many commands (#1183)
 * fix thread cancellation in sqlite module (#1196)
 * fix segfault on 32bit systems in cron module (#1178)
 * log errors from event redistribution (#1457)
 * increase number of open files in `wrexecd` (#1450)
 * fix job hangs during final task IO output flush (#1450)
 * fixes for `flux-wreck purge` (#1357)
 * scalability fixes for `flux-wreck` subcommands (#1372)
 * general reduction in log messages at INFO level (#1450)
 * improve valgrind.h detection (#1502)
 * fix pkg-config pc name for liboptparse (#1506)
 * fix flux executable run-from-build-tree auto-detection (#1515)

### New Features
 * support config file boot method for broker (#1320)
 * new `flux-kvs ls` command (#1172, #1444)
 * new kvs transaction API (#1346, #1348, #1351)
 * support for KVS namespaces (#1286, #1299, #1316, #1323, #1320, #1327,
    #1336, #1390, #1423, #1432, #1436)
 * support for node inclusion,exclusion via flux-wreck command (#1418)
 * initial parser for jobspec (#1201, #1293, #1306)
 * store child instance URI in enclosing instance (#1429)
 * new `flux-wreck uri` command to fetch child instance URIs (#1429)
 * additional states from kvs module (#1310)
 * append support for KVS values (#1265)
 * support multiple blobrefs per valref in kvs (#1227, #1237)
 * add `flux_kvs_lookup_get_raw`(3) (#1218)
 * add `flux_kvs_lookup_get_key`(3) (#1414)
 * add `flux_event_publish`(3) to libflux API (#1512)
 * support for composite futures in libflux (#1188)
 * add `flux_future_reset`(3) to support multi-response RPCs (#1503)
 * new libflux-idset library (#1498)
 * support raw payloads in `flux-event` (#1488)
 * add raw encode/decode to `flux_event_*` API (#1486)
 * introduce `R_lite` format for job allocation description (#1399, #1485)
 * new `flux-hostlist` command for listing hostnames for jobs (#1499)
 * new `flux-wreck` environment manipulation commands (#1405)
 * `flux-wreck ls` returns active jobs first (#1481)
 * `flux-wreck` tools allow filtering on active,inactive jobs (#1481)
 * `flux-wreckrun` will now block until job is scheduled by default, use the
    new --immediate flag to get old behavior (#1399)
 * add `flux-wreck cancel` command to cancel pending job (#1365, #1367, #1385)
 * add `flux-wreck dumplog` command to dump error log for jobs (#1450)
 * add new `KZ_FLAGS_NOFOLLOW` flag to avoid blocking when no data in a kz
    file (#1450)
 * add `-n, --no-follow` option to `flux-wreck attach` (#1450)
 * propagate gpu and cores information for `flux-wreckrun` and `submit`
   (#1399, #1480)
 * use cmb.exec service to launch `wrexecd`, not direct exec (#1508)
 * new `completing` state for jobs (#1513)
 * support job epilog pre-complete and post-complete scripts (#1513)
 * support output to stderr with `flux_log` functions (#1192)
 

### Cleanup
 * kvs: major cleanup (#1154, #1177, #1182, #1190, #1214, #1213, #1233,
    #1235, #1242, #1243, #1244, #1246, #1248, #1253, #1257, #1262, #1263,
    #1268, #1269, #1273, #1274, #1276, #1279, #1300, #1301, #1304, #1308,
    #1309, #1301, #1314, #1315, #1321, #1329, #1339, #1342, #1343, #1347,
    #1349, #1353, #1383, #1402, #1404, #1440, #1458, #1466, #1477)
 * kvs: improved test coverage (#1291)
 * Add const to message payload accessor functions (#1212)
 * rename `flux_mrpcf`, `flux_mrpc_getf` to `flux_mrpc_pack`,`unpack` (#1338)
 * cleanup bulk message hanglers in libflux (#1277)
 * minor `flux_msg_handler` cleanup (#1171)
 * broker: cleanup to prepare for dynamic service registration (#1189)
 * broker: general cleanup (#1230, #1234, #1241)
 * Change key lwj to jobid in all jsc/wreck messages (#1409)
 * libjsc cleanup (#1374, #1395, #1509)
 * testsuite updates (#1167, #1175, #1313, #1464, #1266)
 * Internal libutil and libflux cleanup (#1319, #1283, #1229, #1231, #1166)
 * build system cleanup (#1163, #1354, #1184, #1200, #1275, #1252)
 * disable pylint by default (#1255, #1258)
 * partial migration from json-c to jansson (#1501, #1508) 
 * drop unused `ev_zlist` composite watcher (#1493)


flux-core version 0.8.0 - 2017-08-23
------------------------------------

#### Fixes
 * libflux: remove calls to functions that exit on error (#1060)
 * fix flux_reactor_run() to return active watcher count (#1085)
 * fix flux path detection when install path contains symlinks (#1122)
 * lua: fix refcount bug in kvs bindings (#1116)
 * kvs: oom() fixes (#1124, #1128)
 * kvs: Fix forced dirty bit clear error (#1133)
 * kvs: fix invalid memory read (#1065)
 * kvs: directory walk return error fixes (#1058)
 * kvs_classic: fix kvs(dir)_put_double (#1114)
 * fix memory leaks detected by valgrind (#1076)
 * avoid deadlock when unloading connector-local module (#1027)
 * fix several arm7l portability issues (#1023)
 * optparse: test and allow adjustment of posixly-correct behavior (#1049)
 * Small improvements for systemd unit file and install paths (#1037)
 * fix small leak in flux cmd driver (#1067)

#### New Features
 * add FLUX_MSGFLAG_PRIVATE and allow guests to content load/store (#1032)
 * allow guests to access hwloc topology (#1043)
 * libflux: new flux_future_t API (#1083)
 * libflux: implement RPCs in terms of futures (#1089)
 * kvs: implement transaction objects (#1107)
 * connector-local: Fix compiler warning (#1031)
 * add optional initial program timeout, for test scripts (#1129)
 * libutil: new dirwalk interface (#1072, #1061, #1059)
 * connector-local: add exponential backoff to connect retry count (#1148)
 * support tbon.endpoint and mcast.endpoint attributes (#1030)
 * content: allow hash type to be configured (#1051)

#### Cleanup
 * update many broker attribute names (#1042)
 * consolidate installed libraries and source tree cleanup (#1095)
 * convert broker from json-c to jansson (#1050)
 * libflux: rename jansson pack/unpack-based Flux API functions (#1104)
 * kvs: various code cleanup (#1057, #1073, #1079, #1099, #1119, #1123, #1153)
 * kvs: refactor kvs commit, lookup, and walk logic (#1066, #1105)
 * kvs: drop unused, legacy and deprecated functions (#1100, #1116)
 * kvs: switch from json-c to jansson (#1108, #1153)
 * Misc Cleanup/Minor Fixes from KVS TreeObject Work (#1152)
 * cron: avoid use of json-c and xzmalloc (#1143)
 * Change void * to void ** in flux_msg_get_payload (#1144)
 * python: make bindings compatible with newer versions of pylint (#1113)
 * barrier: cleanup (#1092)
 * tweak watcher structure, add external watcher construction interface
   (#1082)
 * drop coprocess programming model (#1081)
 * split flux_mrpc() out to its own class (#1080)
 * deprecate some libutil classes (#1047)
 * cleanup of flux_msg_copy(), flux_rpc_aux_set() etc. (#1056)

#### Testing
 * update sharness version to upstream 1.0.0 version (#1035)
 * cleanup kvs tests (#1149)
 * mitigate slow builds in Travis-CI (#1142)
 * fix --chain-lint option in sharness tests (#1125)
 * t2000-wreck.t: fix intermittent failures (#1102, #1109)
 * kvs: Add json_util unit tests (#1106)
 * run valgrind if available as part of make check (#1076, #1098)
 * add FLUX_PMI_SINGLETON env variable to avoid SLURMs libpmi in valgrind
   test (#1091)
 * other test improvements (#1087)
 * update soak test for recent flux changes (#1072)
 * test/security: Fix test corner case (#1029)

#### Documentation
 * add missing manpages, minor manpage fixes (#1045)
 * improve reactor documentation (#1086)
 * Code comments and documentation cleanup (#1138)

flux-core version 0.7.0 - 2017-04-01
------------------------------------

#### Fixes

 * Improve reliability of module unloading (#1017)
 * Update autotools for `make dist` to support newer arches (#1016)
 * Fix corner cases in resource-hwloc module (#1012)
 * Ensure destructors are called during broker shutdown (#1005)
 * `flux-logger(1)` and `flux_log(3)` can return error (#1000)
 * Fix balancing of Caliper hooks in RPC calls (#991)
 * Fix missed errors in subscribe/unsubscribe on local connector (#994)
 * sanitize log entries before they enter circular buffer (#959)
 * Do not send wreck.state.complete event before job archival (#955) 
 * Update embedded libev to 4.24 (#944)
 * Propagate argument quoting properly in `flux-start` and `flux-broker` (#931)
 * Fixes and improvements in liboptparse (#922, #927, #929)
 * Tighten up PMI implementation for OpenMPI (#926)

#### New Features

 * Allow user other than instance owner to connect to an instance (#980)
 * Systemd support, default run directory and URI for system instance
   (#992, #995)
 * New `--bootstrap` option to `flux-start` (#990)
 * New `KVS_NO_MERGE` flag in kvs commit and fence operations (#982)
 * Add `broker.pid` to broker attributes (#954)
 * `flux start` only execs broker if `--size` is not specified (#951)
 * Add pkg-config package for Flux PMI (#921)

#### Cleanup

 * Remove live module (#1003)
 * Remove flux-up and flux-topo (#960)
 * Transition away from deprecated czmq classes (#1013)
 * Re-architect and improve many internal and cmd rpc functions (#1002, #1009)
 * Other major and minor cleanup (#919, #928, #941, #940, #942, #954, #969,
    #976, #981, #978, #986, #990, #1001, #1008)
 * Remove `cmb.` prefix from broker services (#947)

#### Testing

 * Expand and improve unit and system tests for greater code coverage
   (#937, #942, #979, #985, #991, #1004, #1011, #1013, #1014)
 * Fix documentation spellcheck (#1015)
 * Add dependency on "all" to top-level `make check` (#970)
 * Add flake8/pylint checks (#816)

#### Documentation

 * Improve flux_reactor_create documentation (#970)
 * Update flux_msg_cmp(3) and flux_recv(3) to match flux_match changes (#946)
 * Update flux-submit(1) and flux-wreckrun(1) manpages (#945)


flux-core version 0.6.0 - 2016-11-29
------------------------------------

#### Fixes

 * Fixes for ATS testsuite compatibility (#914)
 * python: install kz bindings file (#895)
 * broker: adjust errno response to "upstream" request on rank 0 (#913)
 * Fix for possible unconstrained memory growth in modules/libjsc (#891)
 * Fix error message on flux-help failure (#887)
 * Issue fatal error in wrexecd for invalid tasks on node (#901)
 * Fix barrier protocol incompatibility with older jansson versions (#889)

#### New Features

 * Add a flux content service API (#903)
 * Enhance option parsing library for thread safety and new features
  (#908, #910, #911)
 * Add flux_rpcf_multi() (#909)
 * Add new "any" and "upstream" nodeset options (#909)
 * Add HostName key in resource-hwloc `by_rank` directory to allow
   easy resolution of rank to hostname in a flux session (#892)
 * Add `-d` option to `flux-kvs dir`, `dirat`, and `watchdir` to restrict
   output to key only. (#896)

#### Cleanup

 * `flux-ping` refactor and cleanup (#898, #904)
 * Check expected size of `json_int_t` during configure (#902)
 * Other various cleanup, refactoring and testing updates.


flux-core version 0.5.0 - 2016-10-27
------------------------------------

* barrier module cleanup and debug logging (#885)
* Various minor cleanup and documentation updates (#886)
* use jansson >= 2.6 and document JSON payload functions (#884)
* fix MANPATH for Ubuntu, and tidy up travis dep builder (#877)
* fixes for minor issues detected by Coverity (#876)
* build: add --disable-docs configure option (#871)
* kvs: allow get_double to be called on an int (#872)
* README.md: Update srun instructions (#867)
* misc minor fixes (#862)
* make flux_msg_t a bonafide type, add jansson payload accessors (#857)
* Fix --rank issues, add NODESET documentation, and minor cleanup (#860)
* Fix output errors with flux up --comma & --newline, add appropriate tests (#858)
* Add hierarchical lwj directory support in kvs (#811)
* doc/man1/flux-start.adoc: Fix example option usage (#852)
* add dlopen RTLD_DEEPBIND flag (#849)
* src/broker/broker.c: Fix typo (#851)
* doc/man1/flux-module.adoc: Fix environment variable error (#850)
* Pull in json-c, allowing internals to link against alternate json libraries. (#835)
* Add enhanced flux_rpc functions using libjansson json_pack/unpack functions
* Update flux_t * references in man pages (#844)
* Remove pointer from typedef flux_t (#841)
* Remove JSON typedef, just use json_object * (#832)
* module: Remove pointer from typedef flux_modlist_t (#831)
* security: Remove pointer from typedef flux_sec_t (#830)
* and related functions (#824)
* experimental aggregator module (#787)
* kvs: testing, fix use-after-free, streamline python bindings (#823)
* Fix #821: crash in kvs due to NULL arg in Jget_str() (#822)
* python: add a check for invalid handle types (#819)
* Python json and constant rework (#799)
* Python destructor refactoring and exception safety (#807)
* libutil/veb: quiet uninitialized variable warning in vebnew (#809)
* when tagpool is exhausted, grow up to RFC 6 limits (#806)
* add KVS blobref access functions (#801)
* Fix missing error checks in Lua bindings, flux-wreckrun, flux-wreck (#804)
* python: Several fixes for the bindings (#794)
* Switch lua scripts to use lua interpreter in PATH (#789)
* restructure kvs commit handling code for correctness (#788)
* broker/hello: fix leak/error detection in flux_rpc (#786)
* implement scalable reduction for wireup protocol (#781)
* wreck: minor enhancements for scale testing (#782)
* increase KVS commit window (#780)
* autogen.sh: run libtoolize before autoreconf (#771)
* clean up LOG_INFO output, log wireup, rc1, rc3 times, add pmi timing. (#769)
* optparse: remove requirement for option key on long-only options (and other fixes) (#768)

#### Testing

* add test to verify KVS int can be read as double (#878)
* travis-ci: minor updates (#865)
* jsc test: Add timed waits to avoid races (#859)
* t/t0005-exec.t: Fix corner case in test for file not found (#848)
* Fix make distcheck (#847)
* t/t2000-wreck.t: Fix invalid compare on per-task affinity test (#837)
* t/t2000-wreck.t: Fix invalid compare on 'wreckrun: --input=0 works' test (#836)
* travis.yml:  Fix ANCHOR definition (#767)

flux-core version 0.4.1 - 2016-08-12
------------------------------------

* python `kvs_watch()` fix (#759)

* include man7 in distribution (#762)


flux-core version 0.4.0 - 2016-08-11
------------------------------------

#### Scalability improvements

* don't store broken-down hwloc topology in the KVS (#716)

* route rank-addressed requests via TBON (#689)

* streamline matchtag handling (#687)

* keep active jobs in a separate KVS namespace from "archived" jobs (#609)

#### New features

* implement PMI-1 simple server in wrexecd (#706)

* add skeletal PMI-2 library (based on PMI-1) (#747)

* make libflux-optparse.so available externally (#702)

* asynchronous KVS fence and rewritten fence path in KVS module (#707, #729)

* `flux-cron`, a cron/at-like service (#626)

* `flux-proxy` and `ssh://` connector (#645)

#### Other changes

* Use RFC 5424 log format for internal logging, not ad hoc JSON (#691)

* Add wreck lua.d MPI personalities (#669, #743, #747)

* Improved command line for launching flux from slurm/flux (#658)

* Assorted code cleanup.

* Automatic github release upload on tags (#744)

#### Deprecations

* Sophia content backing store module (#727)

* mrpc KVS based multi-RPC interface (#689)

* ZPL config file (#674)

* Ring overlay network (#689)

#### Testing

* Print backtraces for any core files generated in travis-ci (#703)

* Add cppcheck target to travis-ci (#701)

* configure --enable-sanitizer for AddressSanitizer, ThreadSanitizer (#694)

* caliper based profiling (#741)

* coverage uploaded to CodeCof (#751)

* improved test coverage


flux-core version 0.3.0 - 2016-04-26
------------------------------------

* Add support for launching Intel MPI, OpenMPI using PMIv1.
  Use the broker circular log buffer for PMI tracing.

* Add flux wreck timing subcommand which prints time from
  - STARTING: reserved->starting
  - RUNNING:  starting->running
  - COMPLETE: running->complete
  - TOTAL:    starting->complete

* Add three "run levels" for Flux jobs:
  1. run rc1 script on rank 0 to load modules, etc.
  2. run the user's initial program
  3. run rc3 script on rank 0 to unload modules, etc.

* Add module status reporting via keepalive messages.
  `flux module list` now reports live module status:
  - I = initializing
  - S = sleeping
  - X = exited
  - R = running
  - F = finalizing

* Conform to RFC 3 change that requires all JSON payloads to use
  objects as the outermost JSON type (no bare arrays for example).

* Add `flux nodeset` utility so scripts can manipulate nodesets.

* Make `flux env` output suitable for use in bash/zsh eval.

* Drop `flux module --direct` option.  Module load/unload/list is
  now always direct between flux-module and broker(s).
  Drop the `modctl` module for distributed module control.

* When a module fails before entering its reactor loop, propagate
  the error back to `flux module load` so the user knows the
  load was not successful.

* Address memory leaks and adjust KVS usage to ameliorate increasing
  broker memory footprint and degrading job throughput when running
  many small jobs back to back.  Active jobs are now stored under
  `lwj-active` to avoid creating excessive versions of the larger `lwj`
  directory as job state is accumulated.

* Bug fixes to `live` (TBON self-healing) module.  The module is no
  longer loaded by default, pending additional work.  `flux up` will
  always report all ranks up when `live` is not loaded.

* Send keepalives on the ring network and log idle peers on TBON
  and ring at `LOG_CRIT` level, where "idle" means no messages in >= 3
  heartbeats.

* Compress large `content-sqlite` blobs with lzo to reduce disk
  utilization.

* KVS improvements:
  - `kvs_put()` follows intermediate symlinks
  - KVS operations bundled within one commit are applied in order
  - add `kvs_copy()` and `kvs_move()` utility functions.

* Configuration is loaded into broker attribute `config` namespace
  rather than KVS, and is no longer inherited from the enclosing instance.

* `flux` command driver usability improvements.

* Flux API improvements including dropping deprecated functions
  and fine tuning some function signatures (users should recompile).

* Build system allows `--with-tcmalloc`, `--with-jemalloc`, and tcmalloc
  heap profiling.

* Fine tuning of log levels and messages.

* Documentation improvements.

* Test suite improvements/fixes.


flux-core version 0.2.0 - 2016-02-16
------------------------------------

* Avoid putting the Flux libpmi.so in the system ld.so path on systems
  where Flux is installed to the default system prefix, as this could
  interfere with MPI runtimes under other resource managers.

* Enable the SQLite backing store for the KVS by default, which
  addresses unchecked memory growth in the rank 0 broker.

* Stability and usability improvements to the flux-hwloc subcommand,
  and resource-hwloc comms module.

* Added the flux-version subcommand.

* Build system fixes.

* Test suite fixes.

flux-core version 0.1.0 - 2016-01-27
------------------------------------

Initial release for build testing only.

