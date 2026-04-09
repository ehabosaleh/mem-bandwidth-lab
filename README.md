# POSIX IPC Microbenchmarks

This project provides microbenchmarks to measure **one-way latency** and **effective bandwidth** of POSIX inter-process communication (IPC) mechanisms on a single host.

The following IPC mechanisms are evaluated:

1. POSIX Pipes (pipe())
2. POSIX Named Pipes (FIFOs)
3. POSIX Message Queues
5. POSIX Shared Memeory
6. TODO: TCP/IP Sockets

All benchmarks use a ping-pong methodology to derive latency and bandwidth.
