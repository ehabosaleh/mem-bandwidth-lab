# POSIX Pipe IPC Microbenchmark

This benchmark measures **one-way latency** and **effective bandwidth** of POSIX pipes on a single host.

It uses a parent–child process model with blocking `read()`/`write()` and a ping-pong protocol to obtain round-trip timing.

---

## Overview

The benchmark:

- Creates two unidirectional pipes:
  - Parent → Child
  - Child → Parent
- Forks a child process
- Uses a request/ack handshake
- Measures round-trip time (RTT)
- Reports:
  - One-way latency (µs)
  - Effective bandwidth (MiB/s)

---

## Measurement Model

For each message size:

1. Parent sends control header:
   - Operation (`'L'`)
   - Message size
2. Child acknowledges
3. Parent performs ping-pong iterations:
   - Parent writes payload
   - Child echoes payload
4. RTT is measured over multiple iterations
5. One-way latency is computed as: lat_us = (t_end - t_start) / iterations / 2
6. Effective bandwidth (MiB/s) is measured as: BW (msg / (lat_us * 1e-6)) / (1024 * 1024)
