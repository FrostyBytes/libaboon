# libaboon

A co-operative multitasking library for C that replaces libc.

Libaboon is a replacement for libc that supports multiple threads
through cooperative multitasking. It uses assembler blocks in C source
code to provide a start symbol and system calls (only 64-bit Linux is
supported). Cooperative multitasking is done by copying the current
stack to and from non-contiguous buffer pages. This reduces the
requirement for continuous stack pages (except for the initial program
stack that will always hold the currently executing thread).

Libaboon is intended to be directly included into a client program so
that an executable program is produced in a single compilation
step. With gcc, the option set `-nostartfiles -nostdlib
-nodefaultlibs` suppresses the inclusion of default libraries.

Libaboon is currently a bit fragile and does not work older versions
of `gcc` still present in some Linux distributions. It is known to
work with gcc 5.4.0.
