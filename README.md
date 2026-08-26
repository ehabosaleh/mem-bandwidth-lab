# POSIX IPC Microbenchmarks

This project provides microbenchmarks to measure **one-way latency** and **effective bandwidth** of POSIX inter-process communication (IPC) mechanisms on a single host.

The following IPC mechanisms are evaluated:

1. POSIX Pipes (pipe())
2. POSIX Named Pipes (FIFOs)
3. POSIX Message Queues
5. POSIX Shared Memory
6. UNIX Domain Sockets
    - STREAM
    - DGRAM
7. Internet Domains Sockets:
    - STREAM (TCP/IP)
    - DGRAM (UDP)
9. libibverbs (RDMA)

All benchmarks use a ping-pong methodology to derive latency and bandwidth.
