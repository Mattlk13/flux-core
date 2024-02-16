.. flux-help-section: other

================
flux-hostlist(1)
================

SYNOPSIS
========

**flux** **hostlist** [*OPTIONS*] [*SOURCES*]

DESCRIPTION
===========

.. program:: flux hostlist

:program:`flux hostlist` takes zero or more *SOURCES* of host lists on the
command line and concatenates them by default into a single RFC 29 Hostlist.

*SOURCES* can optionally be combined by various set operations, for example
to find the intersection, difference, or to subtract hostlists.

SOURCES
=======

Valid *SOURCES* of hostlist information include:

instance
  hosts from the broker ``hostlist`` attribute

jobid
  hosts assigned to a job.

local
  *jobid* from ``FLUX_JOB_ID`` environment variable if set, otherwise
  *instance*

avail[able]
  *instance* hostlist minus those nodes down or drained

stdin or ``-``
  read a list of hosts on stdin

hosts
  a literal RFC 29 Hostlist

The default source is *local*.

OPTIONS
=======

.. option:: -e, --expand

  Expand hostlist result using the defined output delimiter. Default is
  space-delimited.

.. option:: -d, --delimiter=S

  Set the delimiter for :option:`--expand` to string *S*.

.. option:: -c, --count

  Emit the number of hosts in the result hostlist instead of the hostlist
  itself.

.. option:: -n, --nth=N

  Output only the host at index *N* (*-N* to index from the end). The command
  will fail if *N* is not a valid index.

.. option:: -L, --limit=N

  Output at most *N* hosts (*-N* to output the last *N* hosts).

.. option:: -S, --sort

  Display sorted result.

.. option:: -u, --union, --unique

  Return only unique hosts. This implies :option:`--sort`. Without any
  other manipulation options, this is equivalent to returning the set
  union of all provided hosts. (By default, all inputs are concatenated).

.. option:: -x, --exclude=HOSTS

  Exclude all occurrences of *HOSTS* form the result.

.. option:: -i, --intersect

  Return the set intersection of all hostlists.

.. option:: -m, --minus

  Subtract all hostlists from the first.

.. option:: -X, --xor

  Return the symmetric difference of all hostlists.

.. option:: -f, --fallback

  If an argument to :command:`flux-hostlist` is a single hostname, and the
  hostname can be interpreted as a valid Flux jobid (e.g. starts with ``f``
  and otherwise contains valid base58 characters like ``fuzzy`` or ``foo1``),
  then the command may fail with::

    flux-hostlist: ERROR: job foo1 not found

  With the :option:`--fallback` option arguments that appear to be jobids that
  are not found are treated as hostnames, e.g.::
 
    $ flux hostlist --fallback foo1 foo2
    foo[1-2]
  
.. option:: -q, --quiet

  Suppress output and exit with a nonzero exit code if the hostlist is empty.

EXAMPLES
========

Create host file for the current job or instance if running in an initial
program:

::

  $ flux hostlist -ed'\n' >hostfile

Launch an MPI program using :program:`mpiexec.hydra` from within a batch
script:

::

  #!/bin/sh
  mpiexec.hydra -launcher ssh -hosts "$(flux hostlist -e)" mpi_hello

List the hosts for one job: (Note: this is the same as
:command:`flux jobs -no {nodelist} JOBID`)

::

  $ flux hostlist JOBID
  host[1-2]

List the unordered, unique hosts for multiple jobs:

::

  $ flux hostlist -u JOBID1 JOBID2 JOBID3
  host[1-2,4]

Determine if any failed jobs shared common nodes:

::

  $ flux hostlist --intersect $(flux jobs -f failed -no {id})
  host4

Determine if a given host appeared the last submitted job:

::

  if flux hostlist -q -i $(flux job last) host1; then
      echo host1 was part of your last job
  fi


Count the number of currently available hosts:

::

  $ flux hostlist --count avail
  4

List all the hosts on which a job named 'myapp' ran:

::

  $ flux hostlist --union $(flux pgrep myapp)
  host[2,4-5]

List all hosts in the current instance which haven't had a job assigned
in the last 100 jobs:

::

  $ flux hostlist --minus instance $(flux jobs -c 100 -ano {id})
  host0

EXIT STATUS
===========

0
  Successful operation

1
  One or more *SOURCES* were invalid, an invalid index was specified to
  :option:`--nth`, or :option:`--quiet` was used and the result hostlist
  was empty.

2
  Invalid option specified or other command line error

RESOURCES
=========

.. include:: common/resources.rst
  
FLUX RFC
========

:doc:`rfc:spec_29`


SEE ALSO
========

:man1:`flux-getattr`, :man1:`flux-jobs`, :man7:`flux-broker-attributes`
