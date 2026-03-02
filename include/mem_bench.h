#pragma once
#ifndef MEM_BENCH_H
#define MEM_BENCH_H

#include<errno.h>
#include<time.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<stdint.h>
#include<stddef.h>
#include<limits.h>
#include<math.h>
#include<fcntl.h> //provide control over file descriptors
#include "tlpi_hdr.h"
#include <sys/types.h>
#include <sys/stat.h>

__attribute__((always_inline)) static inline double now_sec(void){
     struct timespec ts;
     if(clock_gettime(CLOCK_MONOTONIC,&ts)!=0){
         perror("Error: clock_gettime");
         exit(EXIT_FAILURE);
     }

     return (double)ts.tv_sec+(double)ts.tv_nsec*1e-9;
 }

#endif
