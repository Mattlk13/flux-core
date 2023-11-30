==============
flux-module(1)
==============


SYNOPSIS
========

| **flux** **module** **load** [*--name*] *module* [*args...*]
| **flux** **module** **reload** [*--name*] [*--force*] *module* [*args...*]
| **flux** **module** **remove** [*--force*] *name*
| **flux** **module** **list** [*-l*]
| **flux** **module** **stats** [*-R*] [*--clear*] *name*
| **flux** **module** **debug** [*--setbit=VAL*] [*--clearbit=VAL*] [*--set=MASK*] [*--clear=MASK*] *name*



DESCRIPTION
===========

.. program:: flux module

:program:`flux module` manages dynamically loadable :man1:`flux-broker` modules.


COMMANDS
========

load
----

.. program:: flux module load

Load *module*, which may be either the path to a shared object file, including
the ``.so`` suffix, or the basename of a shared object file on the
:envvar:`FLUX_MODULE_PATH`, without the suffix.

When :program:`flux module load` completes successfully, the new module has
entered the running state (see LIST OUTPUT below).

.. option:: -n, --name=NAME

  Override the default module name.  A single shared object file may be
  loaded multiple times under different names.

reload
------

.. program:: flux module reload

Reload module *name*. This is equivalent to running
:program:`flux module remove` followed by :program:`flux module load`.

.. option:: -f, --force

  Suppress failure if *module* is not loaded and proceed with loading.

.. option:: -n, --name=NAME

  Override the default module name.

remove
------

.. program:: flux module reload

Remove module *name*.

.. option:: -f, --force

  Suppress failure if module *name* is not loaded.

list
----

.. program:: flux module list

List the loaded modules.

.. option:: -l, --long

  Include the full DSO path for each module.

stats
-----

.. program:: flux module stats

Request statistics from module *name*. A JSON object containing a set of
counters for each type of Flux message is returned by default, however
the object may be customized on a module basis.

.. option:: -p, --parse=OBJNAME

  *OBJNAME* is a period delimited list of field names that should be walked
  to obtain a specific value or object in the returned JSON.

.. option:: -t, --type=int|double

  Force the returned value to be converted to int or double.

.. option:: -s, --scale=N

  Multiply the returned (int or double) value by the specified
  floating point value.

.. option:: -R, --rusage

  Return a JSON object representing an *rusage* structure
  returned by :linux:man2:`getrusage`.

.. option:: -c, --clear

  Send a request message to clear statistics in the target module.

.. option:: -C, --clear-all

  Broadcast an event message to clear statistics in the target module
  on all ranks.

debug
-----

.. program:: flux module debug

Manipulate debug flags in module *name*. The interpretation of debug
flag bits is private to the module and its test drivers.

.. option:: -C, --clear

  Set all debug flags to 0.

.. option:: -S, --set=MASK

  Set debug flags to *MASK*.

.. option:: -s, --setbit=VAL

  Set one debug flag *VAL* to 1.

.. option:: -c, --clearbit=VAL

  Set one debug flag *VAL* to 0.

DEBUG OPTIONS
=============

.. program:: flux module debug

.. option:: -c, --clear

   Set debug flags to zero.

.. option:: -S, --set=MASK

   Set debug flags to MASK.
   The value may be prefixed with 0x to indicate hexadecimal or 0
   to indicate octal, otherwise the value is interpreted as decimal.

.. option:: -c, --clearbit=MASK

   Clear the debug bits specified in MASK without disturbing other bits.
   The value is interpreted as above.

.. option:: -s, --setbit=MASK

   Set the debug bits specified in MASK without disturbing other bits.
   The value is interpreted as above.


LIST OUTPUT
===========

.. program:: flux module list

The *list* command displays one line for each unique (as determined by
SHA1 hash) loaded module.

**Module**
   The value of the **mod_name** symbol for this module.

**Idle**
   Idle times are defined as the number of seconds since the module last sent
   a request or response message.

**State**
   The state of the module is shown as a single character: *I* initializing,
   *R* running, *F* finalizing, *E* exited.  A module automatically enters
   running state when it calls :man3:`flux_reactor_run`.  It can transition
   earlier by calling `flux_module_set_running()`.

**Service**
   If the module has registered additional services, the service names are
   displayed in a comma-separated list.

**Path**
   The full path to the broker module shared object file (only shown with
   the **-l, --long** option).


MODULE SYMBOLS
==============

All Flux modules define the following global symbols:

**const char \*mod_name;**
   A null-terminated string defining the module name.

**int mod_main (void \*context, int argc, char \**argv);**
   An entry function.


RESOURCES
=========

.. include:: common/resources.rst


FLUX RFC
========

:doc:`rfc:spec_5`


SEE ALSO
========

:linux:man3:`syslog`
