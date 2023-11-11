.. flux-help-include: true

=============
flux-start(1)
=============


SYNOPSIS
========

**flux** **start** [*OPTIONS*] [initial-program [args...]]

DESCRIPTION
===========

.. program:: flux start

:program:`flux start` assists with launching a new Flux instance, which
consists of one or more :man1:`flux-broker` processes functioning as a
distributed system.  It is primarily useful in environments that don't run
Flux natively, or when a standalone Flux instance is required for test,
development, or post-mortem debugging of another Flux instance.

When already running under Flux, single-user Flux instances can be more
conveniently started with :man1:`flux-batch` and :man1:`flux-alloc`.
The `Flux Administration Guide
<https://flux-framework.readthedocs.io/en/latest/guides/admin-guide.html>`_
covers setting up a multi-user Flux "system instance", where Flux natively
manages a cluster's resources and those commands work ab initio for its users.

:program:`flux start` operates in two modes.  In `NORMAL MODE`_, it does not
launch broker processes; it *becomes* a single broker which joins an externally
bootstrapped parallel program.  In `TEST MODE`_, it starts one or more brokers
locally, provides their bootstrap environment, and then cleans up when the
instance terminates.

NORMAL MODE
===========

Normal mode is used when an external launcher like Slurm or Hydra starts
the broker processes and provides the bootstrap environment.  It is selected
when the :option:`--test-size` option is *not* specified.

In normal mode, :program:`flux start` replaces itself with a broker process
by calling :linux:man2:`execvp`.  The brokers bootstrap as a parallel program
and establish overlay network connections.  The usual bootstrap method is
some variant of the Process Management Interface (PMI) provided by the
launcher.

For example, Hydra provides a simple PMI server.  The following command
starts brokers on the hosts listed in a file called ``hosts``.  The
instance's initial program prints a URI that can be used with
:man1:`flux-proxy` and then sleeps forever::

  mpiexec.hydra -f hosts -launcher ssh \
    flux start "flux uri --remote \$FLUX_URI; sleep inf"

Slurm has a PMI-2 server plugin with backwards compatibility to the simple
PMI-1 wire protocol that Flux prefers.  The following command starts a two
node Flux instance in a Slurm allocation, with an interactive shell as the
initial program (the default if none is specified)::

  srun -N2 --pty --mpi=pmi2 flux start

When Flux is started by a launcher that is not Flux, resources are probed
using `HWLOC <https://www.open-mpi.org/projects/hwloc/>`_.  If all goes well,
when Slurm launches Flux :option:`flux resource info` in Flux should show all
the nodes, cores, and GPUs that Slurm allocated to the job.

TEST MODE
=========

Test mode, selected by specifying the :option:`--test-size` option, launches
a single node Flux instance that is independent of any configured resource
management on the node.  In test mode, :program:`flux start` provides the
bootstrap environment and launches the broker process(es).  It remains running
as long as the Flux instance is running.  It covers the following use cases:

- Start an interactive Flux instance on one node such as a developer system
  ::

    flux start --test-size=1

  Jobs can be submitted from the interactive shell started as the initial
  program, similar to the experience of running on a one node cluster.

- Mock a multi-node (multi-broker) Flux instance on one node
  ::

    flux start --test-size=64

  When the test size is greater than one, the actual resource inventory is
  multiplied by the test size, since each broker thinks it
  is running on a different node and re-discovers the same resources.

- Start a Flux instance to run a continuous integration test.  A test
  that runs jobs in Flux can be structured as::

    flux start --test-size=1 test.sh

  where ``test.sh`` (the initial program) runs work under Flux.  The exit
  status of :program:`flux start` reflects the exit status of ``test.sh``.
  This is how many of Flux's own tests work.

- Start a Flux instance to access job data from an inactive batch job that
  was configured to leave a dump file::

   flux start --test-size=1 --recovery=dump.tar

- Start a Flux instance to repair the on-disk state of a crashed system
  instance (experts only)::

   sudo -u flux flux start --test-size=1 --recovery

- Run the broker under :linux:man1:`gdb` from the source tree::

   ${top_builddir}/src/cmd/flux start --test-size=1 \
      --wrap=libtool,e,gdb


OPTIONS
=======

.. option:: -o, --broker-opts=OPTIONS

   Add options to the message broker daemon, separated by commas.

.. option:: -v, --verbose=[LEVEL]

   This option may be specified multiple times, or with a value, to
   set a verbosity level.  See `VERBOSITY LEVELS`_ below.

.. option:: -X, --noexec

   Don't execute anything. This option is most useful with -v.

.. option:: --caliper-profile=PROFILE

   Run brokers with Caliper profiling enabled, using a Caliper
   configuration profile named *PROFILE*. Requires a version of Flux
   built with :option:`--enable-caliper`. Unless :envvar:`CALI_LOG_VERBOSITY`
   is already set in the environment, it will default to 0 for all brokers.

.. option:: --rundir=DIR

   (only with :option:`--test-size`) Set the directory that will be
   used as the rundir directory for the instance. If the directory
   does not exist then it will be created during instance startup.
   If a DIR is not set with this option, a unique temporary directory
   will be created. Unless DIR was pre-existing, it will be removed
   when the instance is destroyed.

.. option:: --wrap=ARGS

   Wrap broker execution in a comma-separated list of arguments. This is
   useful for running flux-broker directly under debuggers or valgrind.

.. option:: -s, --test-size=N

   Launch an instance of size *N* on the local host.

.. option:: --test-hosts=HOSTLIST

   Set :envvar:`FLUX_FAKE_HOSTNAME` in the environment of each broker so that
   the broker can bootstrap from a config file instead of PMI.  HOSTLIST is
   assumed to be in rank order.  The broker will use the fake hostname to
   find its entry in the configured bootstrap host array.

.. option:: --test-exit-timeout=FSD

   After a broker exits, kill the other brokers after a timeout (default 20s).

.. option:: --test-exit-mode=MODE

   Set the mode for the exit timeout.  If set to ``leader``, the exit timeout
   is only triggered upon exit of the leader broker, and the
   :program:`flux start` exit code is that of the leader broker.  If set to
   ``any``, the exit timeout is triggered upon exit of any broker, and the
   :program:`flux start` exit code is the highest exit code of all brokers.
   Default: ``any``.

.. option:: --test-start-mode=MODE

   Set the start mode.  If set to ``all``, all brokers are started immediately.
   If set to ``leader``, only the leader is started.  Hint: in ``leader`` mode,
   use :option:`--setattr=broker.quorum=1` to let the initial program start
   before the other brokers are online.  Default: ``all``.

.. option:: --test-rundir=PATH

   Set the directory to be used as the broker rundir instead of creating a
   temporary one.  The directory must exist, and is not cleaned up unless
   :option:`--test-rundir-cleanup` is also specified.

.. option:: --test-rundir-cleanup

   Recursively remove the directory specified with :option:`--test-rundir` upon
   completion of :program:`flux start`.

.. option:: --test-pmi-clique=MODE

   Set the pmi clique mode, which determines how ``PMI_process_mapping`` is set
   in the PMI server used to bootstrap the brokers.  If ``none``, the mapping
   is not created.  If ``single``, all brokers are placed in one clique. If
   ``per-broker``, each broker is placed in its own clique.
   Default: ``single``.

.. option:: -r, --recovery=[TARGET]

   Start the rank 0 broker of an instance in recovery mode.  If *TARGET*
   is a directory, treat it as a *statedir* from a previous instance.
   If *TARGET* is a file, treat it as an archive file from :man1:`flux-dump`.
   If *TARGET* is unspecified, assume the system instance is to be recovered.
   In recovery mode, any rc1 errors are ignored, broker peers are not allowed
   to connect, and resources are offline.

.. option:: --sysconfig

   Run the broker with :option:`--config-path` set to the default system
   instance configuration directory.  This option is unnecessary if
   :option:`--recovery` is specified without its optional argument.  It may
   be required if recovering a dump from a system instance.

VERBOSITY LEVELS
================

level 1 and above
   Display commands before executing them.

level 2 and above
   Trace PMI server requests (test mode only).


EXAMPLES
========

Launch an 8-way local Flux instance with an interactive shell as the
initial program and all logs output to stderr:

::

   flux start -s8 -o,--setattr=log-stderr-level=7

Launch an 8-way Flux instance within a slurm job, with an interactive
shell as the initial program:

::

   srun --pty -N8 flux start

Start the system instance rank 0 broker in recovery mode:

::

   sudo -u flux flux start --recovery

Start a non-system instance in recovery mode:

::

   flux start --recovery=/tmp/statedir


RESOURCES
=========

Flux: http://flux-framework.org


SEE ALSO
========

:man1:`flux-broker`
