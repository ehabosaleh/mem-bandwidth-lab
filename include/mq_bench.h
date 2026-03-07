#ifndef MQ_BENCH_H
#define MQ_BENCH_H

#include"mem_bench.h"

#define p2c "mq_parent_to_child"
#define c2p "mq_child_to_parent"
mqd_t mq_p2c;
mqd_t mq_c2p;

void send_all(mqd_t mq, const void *buf, size_t size);
void recv_all(mqd_t mq, void *buf, size_t size);
void usage(const char *argv0);
size_t parse_size(const char* s);
void child_loop(void);
double measure_latency_one(size_t msg_size, int iters,int warmup);
void mq_setup(mqd_t *mq,size_t max_msg);
void mq_cleanup(mqd_t *mq);


#endif
