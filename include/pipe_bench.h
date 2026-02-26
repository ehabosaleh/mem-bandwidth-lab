#ifndef PIPE_H
#define PIPE_H

#define _GNU_SOURCE
#include<errno.h>
#include<sys/wait.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

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
} mode_t;

#endif
