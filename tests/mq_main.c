#include "../include/mq_bench.h"



int main(int argc,char**argv){
    size_t min_bytes = 1;
    size_t max_bytes = 1024ULL*1024ULL;
    int iters = 1000;
    int warmup = 100;

    for(int i=1;i<argc;i++) {
        if(strncmp(argv[i], "--min-bytes=", 12) == 0) min_bytes = parse_size(argv[i] + 12);
        else if (strncmp(argv[i], "--max-bytes=", 12) == 0) max_bytes = parse_size(argv[i] + 12);
        else if (strncmp(argv[i], "--iters=", 8) == 0) iters = atoi(argv[i] + 8);
        else if (strncmp(argv[i], "--warmup=", 9) == 0) warmup = atoi(argv[i] + 9);
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
             usage(argv[0]);
        }else{
             fprintf(stderr, "Unknown arg: %s\n", argv[i]);
             usage(argv[0]);
         }
     }
    mq_setup(max_bytes);

    pid_t pid=fork();
    if(pid<0){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if(pid==0) {
        child_loop();
        exit(EXIT_SUCCESS);
    }
    printf("%-20s %-20s %-20s\n","Bytes","Latency(us)","Bandwidth(MiB/s)");

    for (size_t size=min_bytes;size< max_bytes; size*=2){

        double lat=measure_latency_one(size,iters,warmup);
        double lat_us=lat*1e6;
        double bw=(size/lat)/(1024.0*1024.0);

        printf("%-20zu %-20.3f %-20.3f\n",size, lat_us, bw);
    }

    unsigned char q ='Q';
    send_all(mq_p2c,&q,1);

    mq_cleanup();

    return 0;
}
