#include"../include/pipe.h"

static void write_all(int fd, const void*buff,size_t n){
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

static void read_all(int fd,void*buff,size_t n){
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


 static void usage(const char *argv0) {
     fprintf(stderr,
         "Usage: %s [--min-bytes=N] [--max-bytes=N] [--steps=K] [--iters=N] [--warmup=N]\n"
         "          [--mode=both|lat|bw] [--csv] [--linear]\n"
         "\n"
         "Examples:\n"
         "  %s --max-bytes=64KiB --mode=both\n"
         "  %s --min-bytes=0 --max-bytes=1GiB --steps=31 --mode=bw --csv\n",
         argv0, argv0, argv0);
     exit(EXIT_FAILURE);
 }

static size_t parse_size(const char* s){
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
        if(v>(unsigned long)SIZE_MAX/mul){
            fprintf(stderr,"Passed size is too large\n");
            exit(EXIT_FAILURE);
        }

        return v*mul;
        

    }



}

static void child_loop(int rfd, int wfd){

    for(;;){
        unsigned char op;
        read_all(rfd,&op,1);
        if(op=='Q')
            break;
        
        uint64_t sz=0;
        read_all(rfd,&sz,sizeof(sz));
        
        unsigned char ack=0xAC;
        write_all(rfd,&ack,1);
        
        if(op=='L'){
            if(sz){
                unsigned char *buf=(unsigned char*)malloc((size_t)sz);
                if(!buf){
                    perror("malloc"); exit(EXIT_FAILURE); }
                    read_all(rfd, buf, (size_t)sz);
                    write_all(wfd, buf, (size_t)sz);
                    free(buf);
            }

        }

        if(op=='B'){
            if(sz){
                unsigned char *buf = (unsigned char*)malloc((size_t)sz);
                if(!buf){
                    perror("malloc"); exit(EXIT_FAILURE);
                }
                read_all(rfd, buf, (size_t)sz);
                free(buf);
            }
            unsigned char fin=0xDD;
            write_all(rfd,&fin,1);
        }
        else {
             fprintf(stderr, "child: unknown op %c\n", op);
             exit(EXIT_FAILURE);
        }

    }



}

static double measure_latency_one(size_t msg_bytes, int iters, int warmup, int p2c_w, int c2p_r, int p2c_r, int c2p_w) {
    unsigned char op= 'L';
    uint64_t sz64=(uint64_t)msg_bytes;
    write_all(p2c_w,&op,1);
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
     for (int i=0;i<warmup;i++) {
         if(msg_bytes)
             write_all(p2c_w,buf,msg_bytes);
         if(msg_bytes)
             read_all(c2p_r,buf,msg_bytes);
         else{
             unsigned char z=0;
             write_all(p2c_w,&z,1);
             read_all(c2p_r,&z,1);
         }
     }

     double t0 = now_sec();
     for (int i=0;i<iters;i++) {
         if(msg_bytes)
             write_all(p2c_w,buf,msg_bytes);
         if(msg_bytes)
             read_all(c2p_r, buf, msg_bytes);
         else{
             unsigned char z=0;
             write_all(p2c_w, &z, 1);
             read_all(c2p_r, &z, 1);
         }
     }
     double t1=now_sec();

     if (buf)
         free(buf);

     double rtt=(t1-t0)/(double)iters;
     return rtt/2.0;



}


