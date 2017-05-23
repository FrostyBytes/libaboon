# libaboon

A co-operative multitasking library for C that replaces libc.

Libaboon is a replacement for libc that supports multiple threads
through cooperative multitasking. It uses assembler blocks in C source
code to provide a start symbol and system calls (only 64-bit Linux is
supported). Cooperative multitasking is done by copying the current
stack to and from non-contiguous buffer pages. This reduces the
requirement for continuous stack pages (except for the initial program
stack that will always hold the currently executing thread).
