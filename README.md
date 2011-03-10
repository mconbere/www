Minimal Web Browser
===================

This project documents the creation of a minimal web browser, implementing something that resembles
a braindead HTML parser utilizing HTTP to fetch pages. It aims to implement this entirely in C,
using only standard POSIX libraries, in 128 lines of code or less.

Literate Programming
--------------------

The evolution of this project will be demonstrated over a series of 'chapters', expressed as commented
C files. The comments will follow a particular pattern that will allow the code to be turned into a
simple markdown-based web example:

* Any line that begins with `///` will be treated as a line of markdown. The `///` will be removed, as
  will an initial space, if present.
* All other lines of code will be treated as if they were code.

A simple python script, `preprocess.py`, exists to preprocess the example. If run on a C file of this
format, it will produce a file named .md containing the markdown text for the tutorial, as well as
.min.c, which contains the minimally stripped C code.

Chapter 1
---------

In `ch1.c`, the application skeleton is constructed.

Chapter 2
---------

In `ch2.c`, the application gains a very simple HTTP implementation.

More Information
----------------

For more information, contact Morgan Conbere at mconbere@gmail.com.
