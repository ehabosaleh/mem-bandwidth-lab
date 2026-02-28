#include"../include/pipe_bench.h"

size_t print_buf_size(int fd){
#if defined(__linux__)
        return (size_t)fcntl(fd,F_GETPIPE_SZ);
#else
        return 0;
#endif

}
void set_buf_size(int fd,size_t size){
 #if defined(__linux__)
           fcntl(fd,F_SETPIPE_SZ,size);
#else
           fprintf(stderr,"Not linux environment\n");
#endif

}
void write_all(int fd, const void*buff,size_t n){
    unsigned const char*p=(const unsigned char*) buff;
    while(n){
        ssize_t rc=write(fd,p,n);
        if(rc<0){
            if(errno==EINTR)
                continue;
            perror("Error: write");
            exit(EXIT_FAILURE);
        }
        p+=rc;
        n-=rc;
    }
}

void read_all(int fd,void*buff,size_t n){
    unsigned  char*p=(unsigned char*) buff;
    while(n){

        ssize_t rc=read(fd,p,n);
        if(rc<0){
            if(errno==EINTR)
                continue;
            perror("Error: read");
            exit(EXIT_FAILURE);
        }
        if(rc==0){
            fprintf(stderr,"unexpected EOF\n");
            exit(EXIT_FAILURE);
        }
        p+=rc;
        n-=rc;
    }


}


void usage(const char *argv0) {
     fprintf(stderr,
         "Usage: %s [--min-bytes=N] [--max-bytes=N] [--iters=N] [--warmup=N]\n"
         "Examples:\n"
         "  %s --max-bytes=64KiB \n"
         "  %s --min-bytes=0 --max-bytes=1GiB --steps=31 --mode=bw \n",
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

void child_loop(int rfd, int wfd){
    for(;;){
        
        unsigned char op;
        read_all(rfd,&op,1);
        if(op=='Q')
            break;
        
        if(op=='L'){
            uint64_t sz=0;
            read_all(rfd,&sz,sizeof(sz));
            unsigned char ack=0xAC;
            write_all(wfd,&ack,1);
            
            if(sz){
                unsigned char *buf=(unsigned char*)malloc((size_t)sz);
                if(!buf){
                    perror("malloc"); exit(EXIT_FAILURE); 
                }
                read_all(rfd, buf, (size_t)sz);
                write_all(wfd, buf, (size_t)sz);
                free(buf);
            }

        }

        else {
             fprintf(stderr, "child: unknown op %c\n", op);
             exit(EXIT_FAILURE);
        }

    }



}

double measure_latency_one(size_t msg_bytes, int iters, int warmup, int p2c_w, int c2p_r, int p2c_r, int c2p_w) {
     
    for (int i=0;i<warmup;i++) {
         unsigned char op= 'L';
         write_all(p2c_w,&op,1);
         
         uint64_t sz64=(uint64_t)msg_bytes;
         write_all(p2c_w,&sz64,sizeof(sz64));
         
         unsigned char ack;
         read_all(c2p_r,&ack,1);
         unsigned char *buf=NULL;
         if(msg_bytes){
             buf=(unsigned char*)malloc(sizeof(char)*msg_bytes);
             if (!buf) { perror("malloc"); exit(EXIT_FAILURE); }
             for (size_t i=0;i<msg_bytes;i++)
                 buf[i]=(unsigned char)(i*131u+7u);
         }

         if(msg_bytes){
             write_all(p2c_w,buf,msg_bytes);
             read_all(c2p_r,buf,msg_bytes);
         }
     }

     double t1 = 0;
     for (int i=0;i<iters;i++) {
         unsigned char op= 'L';
         write_all(p2c_w,&op,1);
         uint64_t sz64=(uint64_t)msg_bytes;
         write_all(p2c_w,&sz64,sizeof(sz64));
        
         unsigned char ack;
         read_all(c2p_r,&ack,1);
         unsigned char *buf=NULL;
         if(msg_bytes){
                buf=(unsigned char*)malloc(sizeof(char)*msg_bytes);
                if (!buf) { perror("malloc"); exit(EXIT_FAILURE); }
                for (size_t i=0;i<msg_bytes;i++)
                    buf[i]=(unsigned char)(i*131u+7u);
            }
         if(msg_bytes){  
             double t0=now_sec();
             write_all(p2c_w,buf,msg_bytes);
             read_all(c2p_r, buf, msg_bytes);
             t1+=now_sec()-t0;
         }
         if(buf)
             free(buf);
     }
    

    double rtt=(t1)/(double)iters;
    return rtt/2.0;



}


