.. flux-help-include: true

===========
flux-job(1)
===========


SYNOPSIS
========

**flux** **job** **cancel** *id* [*message...*]

**flux** **job** **cancelall** [*OPTIONS*] [*message...*]

**flux** **job** **kill** [*--signal=SIG*] *id*

**flux** **job** **killall** [*OPTIONS*]

**flux** **job** **raise** [*OPTIONS*] *id* [*message...*]

**flux** **job** **raiseall** [*OPTIONS*] *type* [*message...*]

DESCRIPTION
===========

flux-job(1) performs various job related housekeeping functions.

CANCEL
======

A single job may be canceled with ``flux job cancel``.

Jobs may be canceled in bulk with ``flux job cancelall``.  Target jobs are
selected with:

**-u, --user=USER**
   Set target user.  The instance owner may specify *all* for all users.

**-S, --states=STATES**
   Set target job states (default: ACTIVE).

**-f, --force**
   Confirm the command

**-q, --quiet**
   Suppress output if no jobs match

SIGNAL
======

Running jobs may be signaled with ``flux job kill``.

**-s, --signal=SIG**
   Send signal SIG (default: SIGTERM).

Running jobs may be signaled in bulk with ``flux job killall``.  In addition
to the option above, target jobs are selected with:

**-u, --user=USER**
   Set target user.  The instance owner may specify *all* for all users.

**-f, --force**
   Confirm the command.

EXCEPTION
=========

An exception may raised on a single job with ``flux job raise``.

**-s, --severity=N**
   Set exception severity.  The severity may range from 0=fatal to
   7=least severe (default: 0).

**-t, --type=TYPE**
   Set exception type (default: cancel).

Exceptions may be raised in bulk with ``flux job raiseall``, which requires a
type (positional argument) and accepts the following options:

**-s, --severity=N**
   Set exception severity.  The severity may range from 0=fatal to
   7=least severe (default: 7).

**-u, --user=USER**
   Set target user.  The instance owner may specify *all* for all users.

**-S, --states=STATES**
   Set target job states (default: ACTIVE)

**-f, --force**
   Confirm the command.

RESOURCES
=========

Flux: http://flux-framework.org

