#ifndef MQ_BENCH_H
#define MQ_BENCH_H

#include"mem_bench.h"

#define P2C "/mq_parent_to_child"
#define C2P "/mq_child_to_parent"

struct chunk_hdr {
    uint32_t total_size;
    uint32_t offset;
    uint32_t chunk_size;
};

extern mqd_t mq_p2c;
extern mqd_t mq_c2p;

void send_all(mqd_t mq, const void *buf, size_t size);
void recv_all(mqd_t mq, void *buf, size_t size);

void usage(const char *argv0);
size_t parse_size(const char* s);

void child_loop(void);
double measure_latency_one(size_t msg_size, int iters,int warmup);

void mq_setup(size_t max_msg);
void mq_cleanup(void);

#endif
