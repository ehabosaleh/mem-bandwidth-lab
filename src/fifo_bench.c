#include"../include/fifo_bench.h"

void read_all(int fd,void*buf,size_t n){
    size_t offset=0;
    while(offset<n){
        ssize_t r=read(fd,(char*)buf+offset,n-offset);
        if(r<0){
            perror("Read");
            exit(EXIT_FAILURE);
        }
        offset+=r;
    }
}

void write_all(int fd,const void *buf,size_t n){
    size_t offset=0;
    while(offset<n){
        ssize_t w=write(fd,(char*)buf+offset,n-offset);
        if(w<0){
            perror("Write");
            exit(EXIT_FAILURE);
        }
        offset+=w;
    }
}
