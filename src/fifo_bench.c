#include"../include/fifo_bench.h"

void usage(const char *argv0) {
      fprintf(stderr,
          "Usage: %s [--min-bytes=N] [--max-bytes=N] [--iters=N] [--warmup=N]\n"
          "Examples:\n"
          "  %s --max-bytes=64KiB \n"
          "  %s --min-bytes=0 --max-bytes=1GiB \n",
          argv0, argv0, argv0);
      exit(EXIT_FAILURE);
  }

size_t parse_size(const char* s){
     char*end=NULL;
     errno=0;
     unsigned long long v=strtoull(s,&end,10);
     if (errno!=0 || end==s){
          fprintf(stderr, "Invalid size: %s\n", s);
          exit(EXIT_FAILURE);
      }
     unsigned long long mul=1;
     if(*end){
         if(strcasecmp(end,"k")==0) mul=1000ULL;
         else if(strcasecmp(end,"m")==0) mul=1000ULL*1000ULL;
         else if(strcasecmp(end,"g")==0) mul=1000ULL*1000ULL*1000ULL;
         else if(strcasecmp(end,"kib")==0) mul=1024ULL;
         else if(strcasecmp(end,"mib")==0) mul=1024ULL*1024ULL;
         else if(strcasecmp(end,"gib")==0) mul=1024ULL*1024ULL*1024ULL;
         else {
              fprintf(stderr, "Unknown size suffix: %s\n", end);
              exit(EXIT_FAILURE);
          }
     }
     if(mul!=0&&v>(unsigned long)SIZE_MAX/mul){
             fprintf(stderr,"Passed size is too large\n");
             exit(EXIT_FAILURE);
     }

     return (size_t)v*mul;



 }

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
