Sandboxing libraries with RLBox
===============================

.. toctree::
   :maxdepth: 2
   :caption: Contents:

RLBox is a toolkit for sandboxing third-party libraries. The toolkit consists
of (1) a Wasm-based sandbox and (2) an API for retrofitting existing
application code to interface with a sandboxed library.  The Wasm-based sandbox
is documented in its `corresponding repository
<https://wasm-sandbox.programming.systems>`_.  This documentation focuses on
the API and the interface you will use when sandboxing code, independent of the
underlying sandboxing mechanism.

**Why do we need a sandboxing API?**
Sandboxing libraries without the RLBox API is both tedious and error-prone.
This is especially the case when retrofitting an existing codebase like Firefox
where libraries are trusted and thus the application-library boundary is
blurry.  To sandbox a library -- and thus to move to a world where the library
is no longer trusted -- we need to modify this application-library boundary. 
For example, we need to add security checks in Firefox to ensure that any value
from the sandboxed library is properly validated before it is used.  Otherwise,
the library (when compromised) may be able to abuse Firefox code to hijack its
control flow (see [RLBoxPaper]_ for details). The RLBox API is explicitly
designed to make retrofitting of existing application code simpler and less
error-prone.


**Sandboxing architecture overview** As shown in :numref:`arch-fig`, RLBox ensures that a
sandboxed library is *memory isolated* from the rest of the application -- the
library cannot directly access memory outside its designated region -- and that
all *boundary crossings are explicit*. This ensures that the library cannot,
for example, corrupt Firefox's address space. It also ensures that Firefox
cannot inadvertently expose sensitive data to the library (e.g., pointers that
would leak its ASLR).

.. _arch-fig:
.. figure:: architecture.png
   :scale: 50%
   :align: left
   :alt: RLBox architecture

   Sandboxed libraries are isolated from the application and all communication
   between the sandboxed library and application code is mediated. This ensures
   that the application code is robust and does not use untrusted values
   without checking them.

Memory isolation is enforced by the underlying sandboxing mechanism (from the
start, when you create the sandbox with :ref:`create_sandbox() <create_sandbox>`). Explicit
boundary crossings are enforced by RLBox (either at compile- or and run-time).
For example, with RLBox you can't call library functions directly; instead, you
must use the :ref:`sandbox_invoke() <sandbox_invoke>` method. Similarly, the library cannot
call arbitrary Firefox functions; instead, it can only call functions that you
expose with the :ref:`register_callback() <register_callback>` method. (To
simplify the sandboxing task, though, RLBox does expose a standard library as
described in :ref:`stdlib`.)

When calling a library function, RLBox copies simple values into the sandbox
memory before calling the function. For larger data types, such as structs and
arrays, you can't simply pass a pointer to the object. This would leak ASLR
and, more importantly, would not work: sandboxed code cannot access application
memory.  So, you must explicitly allocate memory in the sandbox via
:ref:`malloc_in_sandbox() <malloc_in_sandbox>` and copy application data to
this region of memory (e.g., via ``strncpy``).

RLBox similarly copies simple return values and callback arguments. Larger data
structures, however, must (again) be passed by *sandbox-reference*, i.e., via a
reference/pointer to sandbox memory.

Example
-------

.. note:: TODO

Core API
========

Creating (and destroying) sandboxes
-----------------------------------

RLBox encapsulates sandboxes with :ref:`rlbox_sandbox <rlbox_sandbox>` class.
For now, RLBox supports two sandboxes: a Wasm-based sandboxed and the *null*
sandbox. The null sandbox doesn't actually enforce any isolation, but is very
useful for migrating an existing codebase to use the RLBox API.  In fact, in
most cases you want to port the existing code to use RLBox when interfacing
with a particular library and only then switch over to the Wasm-based sandbox. 

.. _rlbox_sandbox:
.. doxygenclass:: rlbox::rlbox_sandbox

.. doxygenclass:: rlbox::rlbox_noop_sandbox

.. _create_sandbox:
.. doxygenfunction:: create_sandbox

Creating sandboxes is mostly straightforward.  For the null sandbox, however,
you need to add a ``#define`` at the top of your entry file, before you include
the RLBox headers::

  #define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol
  ...
  rlbox::rlbox_sandbox<rlbox_noop_sandbox> sandbox;
  sandbox.create_sandbox();

.. _destroy_sandbox:
.. doxygenfunction:: destroy_sandbox

It's important to destroy a sandbox after you are done with it. This ensures
that the memory footprint of sandboxing remains low. Once you destroy a sandbox
though, it is an error to use the sandbox object.

Calling sandboxed library functions
-----------------------------------

RLBox disallows code from calling sandboxed library functions directly.
Instead, application code must use the :ref:`sandbox_invoke() <sandbox_invoke>`
method.

.. _sandbox_invoke:
.. doxygendefine:: sandbox_invoke

Though this function is defined via macros, RLBox uses some template and macro
magic to make this look like a :ref:`sandbox <rlbox_sandbox>` method. So, in
general, you can call sandboxed library functions as::

  // call foo(4)
  auto result = sandbox.sandbox_invoke(foo, 4);

Exposing functions to sandboxed code
------------------------------------

Application code can expose :ref:`callback functions <callback>` to sandbox via
:ref:`register_callback() <register_callback>`.  These functions can be called
by the sandboxed code until they are :ref:`unregistered <unregister_callback>`.

.. _register_callback:
.. doxygenfunction:: register_callback(T_Ret (*)T_RL, T_Args...)

The type signatures of :ref:`register_callback() <register_callback>`
functions is a bit daunting. In short, the function takes a :ref:`callback
function <callback>` and returns a function pointer that can be passed to the
sandbox (e.g., via :ref:`sandbox_invoke() <sandbox_invoke>`).

.. _callback:

A *callback function* is a function that has a special type:

* The first argument of the function must be a reference a :ref:`sandbox
  <rlbox_sandbox>` object.
* The remaining arguments must be :ref:`tainted <tainted>`.
* The return value must be :ref:`tainted <tainted>` or ``void``. This ensures
  that the application cannot accidentally leak data to the sandbox.

Forcing arguments to be :ref:`tainted <tainted>` forces the application to
handled values coming from the sandbox with care. Dually, the return type
ensures that the application cannot accidentally leak data to the sandbox.

.. _unregister_callback:
.. doxygenfunction:: unregister_callback

Tainted values
--------------

Values that originate in the sandbox are *tainted*. We use a special tainted
type :ref:`tainted <tainted>` to encapsulate such values and prevent the
application from using tainted values unsafely.

.. _tainted:
.. doxygenclass:: rlbox::tainted

Unwrapping tainted values
^^^^^^^^^^^^^^^^^^^^^^^^^

To use tainted values, the application can copy
the value to application memory, verify the value, and unwrap it. RLBox
provides several functions to do this.

.. doxygenfunction:: copy_and_verify

For a given tainted type, the verifier should have the following signature:

+------------------------+-----------------------+----------------------------------+
| Tainted type kind      |  Example type         | Example verifier                 |
+========================+=======================+==================================+
| Simple type            |  ``int``              | ``T_Ret(*)(int)``                |
+------------------------+-----------------------+----------------------------------+
| Pointer to simple type |  ``int*``             | ``T_Ret(*)(unique_ptr<int>)``    |
+------------------------+-----------------------+----------------------------------+
| Pointer to class type  |  ``Foo*``             | ``T_Ret(*)(unique_ptr<Foo>)``    |
+------------------------+-----------------------+----------------------------------+
| Pointer to array       |  ``int[4]``           | ``T_Ret(*)(std::array<int, 4>)`` |
+------------------------+-----------------------+----------------------------------+
| Class type             |  ``Foo``              | ``T_Ret(*)(tainted<Foo>)``       |
+------------------------+-----------------------+----------------------------------+

In general, the return type of the verifier ``T_Ret`` is not constrained and can
be anything the caller chooses.

.. doxygenfunction:: copy_and_verify_range
.. doxygenfunction:: copy_and_verify_string
.. doxygenfunction:: copy_and_verify_address

In some cases it's useful to unwrap tainted values without verification.
Sometimes this is safe to do and RLBox provides a method for doing so:

.. doxygenfunction:: unverified_safe_because(const char *&&)

We however provide additional functions that are especially useful during
migration:

.. doxygenfunction:: rlbox::tainted_base_impl::UNSAFE_unverified()
.. doxygenfunction:: rlbox::sandbox_callback::UNSAFE_unverified()
.. doxygenfunction:: rlbox::tainted_base_impl::UNSAFE_sandboxed()
.. doxygenfunction:: rlbox::sandbox_callback::UNSAFE_sandboxed()

.. danger::  Unchecked unwrapped tainted values can be abused by a compromised
   or malicious library to potentially compromise the application.


Operating on tainted values
^^^^^^^^^^^^^^^^^^^^^^^^^^^
Unwrapping tainted values requires care -- getting a verifier wrong could lead
to a security vulnerability. It's also not cheap: we need to copy data to the
application memory to ensure that the sandboxed code cannot modify the data
we're tyring to verify. Lucikly, it's not always necessary to copy and verify:
sometimes we can compute on tainted values directly. To this end, RLBox defines
different kinds of operators on tainted values, which produce tainted values.
This allows you to perform some computations on tainted values, pass the values
back into the sandbox, and only later unwrap a tainted value when you need to.
operators like ``+`` and ``-`` on tainted values.

+-----------------------+-----------------------------------+
| Class of operator     |  Supported operators              |
+=======================+===================================+
| Arithmetic operators  |  ``+``,``-``,``*``,``/``,``%``    |
+-----------------------+-----------------------------------+
| Logical opators       |  ``^``,``&``,``|``,``<<``,``>>``  |
+-----------------------+-----------------------------------+
| Unary operators       |  ``-``,``~``                      |
+-----------------------+-----------------------------------+
| Pointer operators     |  ``[]``,``*``,``&``,``->``        |
+-----------------------+-----------------------------------+

When applying a binary operator like ``<<`` to a tainted value and an untainted
values the result is always tainted.

RLBox also defines several comparison operators on tainted values that sometime
unwrap the result:

* Operators ``==``,``!=`` on tainted pointers is allowed if the rhs is ``nullptr_t`` and return unwrapped ``bool``s.
* Operator ``!`` on tainted pointers retruns an unwrapped ``bool``.
* Operators ``==``,``!=``,``!`` on non-pointer tainted values return a ``tainted<bool>``
* Operators ``==``,``!=``,``!`` on `tainted_volatile <tainted_volatile>` values returns a :ref:`tainted_boolean_hint <tainted_boolean_hint>`

.. _tainted_volatile:
A *tainted_volatile* 

.. _tainted_boolean_hint:
A *tainted_boolean_hint*


Application-sandbox shared memory
---------------------------------

Since sandboxed code cannot access application memory, to share objects across
the boundary you need to explicitly allocate memory that both the application
and sandbox can access. To this end, :ref:`malloc_in_sandbox()
<malloc_in_sandbox>` allocates memory within the sandbox region and returns a
:ref:`tainted <tainted>` pointer that can be used by both the application and
sandbox (e.g., by passing the pointer as an argument to a function).

.. _malloc_in_sandbox:
.. doxygenfunction:: malloc_in_sandbox()
.. doxygenfunction:: malloc_in_sandbox(uint32_t)

.. _free_in_sandbox:
.. doxygenfunction:: free_in_sandbox

To distinguish between different pointer types, RLBox also provides some helper functions:

.. doxygenfunction:: is_pointer_in_app_memory
.. doxygenfunction:: is_pointer_in_sandbox_memory
.. doxygenfunction:: is_in_same_sandbox

.. _stdlib:

Standard library
----------------

.. note:: TODO

References
==========

.. [RLBoxPaper] *Retrofitting Fine Grain Isolation in the Firefox Renderer*. S. Narayan, C. Disselkoen, T. Garfinkel, S. Lerner, H. Shacham, D. Stefan. 


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
