#ifndef FIFO_BENCH_H
#define FIFO_BENCH_H
#include"mem_bench.h"

#define SERVER_FIFO "./bin/server_fifo"
#define CLIENT_FIFO "./bin/client_fifo"

void read_all(int fd,void*buf,size_t n);
void write_all(int fd,const void *buf,size_t n);



#endif
