===================================================
  `cscgi` --- Streaming SCGI request parser for C
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
.. _`netstrings`: http://cr.yp.to/proto/netstrings.txt
.. _`SCGI`: http://en.wikipedia.org/wiki/Simple_Common_Gateway_Interface
