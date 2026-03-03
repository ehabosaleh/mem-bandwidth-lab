#include"../include/fifo_bench.h"

int main(int argc,char**argv){
    size_t min_bytes = 1;
    size_t max_bytes = 1024ULL*1024ULL;
    int iters = 1000;
    int warmup = 100;

    for (int i=1;i<argc;i++) {
         if (strncmp(argv[i], "--min-bytes=", 12) == 0) min_bytes = parse_size(argv[i] + 12);
         else if (strncmp(argv[i], "--max-bytes=", 12) == 0) max_bytes = parse_size(argv[i] + 12);
         else if (strncmp(argv[i], "--iters=", 8) == 0) iters = atoi(argv[i] + 8);
         else if (strncmp(argv[i], "--warmup=", 9) == 0) warmup = atoi(argv[i] + 9);
         else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
             usage(argv[0]);
         } else {
             fprintf(stderr, "Unknown arg: %s\n", argv[i]);
             usage(argv[0]);
         }
     }

    int wfd=open(SERVER_FIFO,O_WRONLY);
    int rfd=open(CLIENT_FIFO,O_RDONLY);
    if(wfd<0||rfd<0){
        perror("Client-FIFO:Open");
        exit(EXIT_FAILURE);
    }
    size_t temp_wsize=1;
    size_t temp_rsize=1;
    char*temp_buf=(char*)malloc(temp_wsize);

    for (int i=0;i<warmup;i++) {
        write_all(wfd,&temp_wsize,sizeof(temp_wsize));
        write_all(wfd,temp_buf,temp_wsize);
        read_all(rfd,&temp_rsize,sizeof(temp_rsize));
        read_all(rfd,&temp_buf,temp_rsize);
    }


    printf("%-12s%-12s%-12s \n","Bytes","Latency(us)","Bandwidth(MB/s)");
    for(size_t size=min_bytes;size<max_bytes;size*=2){
        char*buf=malloc(size);
        for(int i=0;i<size;i++)
            *(buf+i)=(char)i;
        
        double t0=now_sec();
        write_all(wfd,&size,sizeof(size));
        write_all(wfd,buf,size);

        read_all(rfd,&size,sizeof(size));
        read_all(rfd,&buf,size);
        double t1=now_sec();

        double rtt=t1-t0;
        double one_way=rtt/2.0;
        
        printf("%-12zu %-12.3f %-12.3f\n",size, one_way*1e6, (size/one_way)/(1024.0*1024.0));
        free(buf);
    }
    close(rfd);
    close(wfd);

    return 0;
}
