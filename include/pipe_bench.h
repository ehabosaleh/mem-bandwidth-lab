#ifndef PIPE_BENCH_H
#define PIPE_BENCH_H

#define _GNU_SOURCE
#include<errno.h>
#include<sys/wait.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<stddef.h>
#include<limits.h>
#include<math.h>

static inline double now_sec(void){
    struct timespec ts;
    if(clock_gettime(CLOCK_MONOTONIC,&ts)!=0){
        perror("Error: clock_gettime");
        exit(EXIT_FAILURE);
    }
    
    return (double)ts.tv_sec+(double)ts.tv_nsec*1e-9;
}

typedef enum{
    MODE_BOTH,
    MODE_LAT,
    MODE_BW
} bench_mode_t;

void write_all(int fd, const void*buff,size_t n);
void read_all(int fd,void*buff,size_t n);
void usage(const char *argv0);
size_t parse_size(const char* s);
void child_loop(int rfd, int wfd);
double measure_latency_one(size_t msg_bytes, int iters, int warmup, int p2c_w, int c2p_r, int p2c_r, int c2p_w);
#endif
