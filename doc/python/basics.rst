Flux Python Basics
==================


Importing the ``flux`` Python package
-------------------------------------

.. note:: The ``flux`` package which is used to interact with Flux *cannot* be
   installed into a virtual environment with pip or conda.

Flux's Python bindings are available with any installation of Flux.
When running in a Flux instance, Flux will export the
`PYTHONPATH <https://docs.python.org/3/using/cmdline.html#envvar-PYTHONPATH>`_
environment variable so that Python processes can import the flux package
by the usual import mechanism (``import flux``).

If you want to import the package from outside of a Flux instance,
running ``/path/to/flux env | grep PYTHONPATH`` in your shell will show you
what PYTHONPATH would be set to if you were running in a Flux instance
built/installed at ``/path/to/flux``.

Note however that if you import the ``flux`` package from outside a Flux
instance, you will need to specify a Flux instance to communicate with,
or most of the package's operations will fail.


Additional Examples
-------------------

In addition to the examples given in these pages, there is
a set of detailed examples maintained on
`the workflow examples page <https://flux-framework.readthedocs.io/projects/flux-workflow-examples/en/latest/>`_.


The Flux class
--------------

Almost all of the functionality of the ``flux`` package revolves around
``flux.Flux`` objects, often called "Flux handles", which represent a
connection to a Flux instance.
It is possible to simultaneously have multiple connections open
to the same Flux instance, or to multiple
Flux instances.

.. note:: Flux handles are not thread-safe and should
	not be shared between threads.

In addition to the methods defined on ``flux.Flux`` objects, the ``flux``
package also provides a number of functions which accept
``flux.Flux`` objects as arguments, such as the functions described
:ref:`here <python_job_submission>`.

.. autoclass:: flux.core.handle.Flux
	:members:


The Flux reactor
----------------
Content coming soon.
