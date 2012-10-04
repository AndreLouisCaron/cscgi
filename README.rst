===================================================
  ``cscgi``: Streaming SCGI request parser for C.
===================================================
:authors:
   André Caron
:contact: andre.l.caron@gmail.com


Description
===========

This library provides a parser for `SCGI`_ requests.  The parser is implemented
as a finite state machine (FSM) for use in streaming applications (i.e. data
arrives at an unpredictable rate and the parser must be interruptible).  As
such, the parser itself does not buffer any received data.  It just forwards it
to registered callbacks.  It requires little overhead and is well suited for
being used in an object-oriented wrapper.


Dependencies
============

The project relies on `cnetstrings`_ to parse SCGI headers.

.. _`cnetstrings`: https://github.com/AndreLouisCaron/cnetstring


Getting started
===============

The library is currently only distributed in source form.  However, it has a
single external dependency and will compile as-is with almost any C compiler.

The following presents the *supported* way to get up and running with
``cscgi``.  Feel free to experiment with your toolchain of choice.

Requirements
------------

Dependencies are:

#. Git_
#. CMake_
#. Doxygen_
#. A C++ compiler toolchain:

   * Microsoft Visual Studio
   * ``g++`` and ``make``

.. _Git: http://git-scm.com/
.. _CMake: http://www.cmake.org/
.. _Doxygen: http://www.stack.nl/~dimitri/doxygen/

Standalone build
----------------

#. Get the source code.

   ::

      > git clone git://github.com/AndreLouisCaron/cscgi.git
      > cd cscgi

   Feel free to check out a specific version

   ::

      > git tag
      v0.1

      > git checkout v0.1

#. Generate the build scripts.

   ::

      > mkdir work
      > cd work
      > cmake -G "NMake Makefiles" ..

#. Build the source code.

   ::

      > nmake

#. Run the test suite.

   ::

      > nmake /A test

#. Build the API documentation.

   ::

      > nmake help

   Open the HTML documentation in ``help/html/index.html``.

Embedded build
--------------

#. Register as a Git sub-module.

   ::

      > cd myproject
      > git submodule add git://github.com/AndreLouisCaron/cscgi.git libs/cscgi

   Feel free to check out a specific version.

   ::

      > cd libs/cscgi
      > git tag
      v0.1

      > git checkout v0.1
      > cd ../..
      > git add libs/cscgi

#. Add ``cscgi`` targets to your CMake project.

   ::

      set(cscgi_DIR
        ${CMAKE_SOURCE_DIR}/libs/cscgi
      )

#. Make sure your CMake project can ``#include <scgi.h>``.

   ::

      include_directories(
        ${cscgi_include_dirs}
      )


#. Link against the ``scgi`` library.

   ::

      target_link_libraries(my-application ${cscgi_libraries})
