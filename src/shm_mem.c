#include"../include/shm_mem.h"


void usage(const char *argv0) 
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
		else{
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

void pin_cpu(int cpu){
	cpu_set_t set;
    	CPU_ZERO(&set);
    	CPU_SET(cpu, &set);
    	sched_setaffinity(0, sizeof(set), &set);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        
	return 1;
    }

    int is_writer = strcmp(argv[1], "writer") == 0;

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); exit(1); }

    size_t total_size = sizeof(shm_region) + MAX_SIZE;
    ftruncate(fd, total_size);

    shm_region *shm = mmap(NULL, total_size,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED, fd, 0);

    if (shm == MAP_FAILED) { perror("mmap"); exit(1); }

    if (is_writer) {
        pin_cpu(0);
        printf("#Size(Bytes)\tLatency(us)\tBandwidth(GB/s)\n");

        for (size_t size = 1; size <= MAX_SIZE; size <<= 1) {

            shm->size = size;

            // warmup
            for (int i = 0; i < WARMUP; i++) {
                shm->ready = 1;
                while (!shm->done);
                shm->done = 0;
            }

            double start = now_sec();

            for (int i = 0; i < ITERS; i++) {
                shm->ready = 1;
                while (!shm->done);
                shm->done = 0;
            }

            double end = now_sec();

            double total_time = end - start;
            double latency = (total_time / ITERS) / 2.0; // RTT/2
            double bandwidth = (double)size / latency / 1e9;

            printf("%zu\t%.3f\t%.3f\n",
                   size,
                   latency * 1e6,
                   bandwidth);
        }

        shm_unlink(SHM_NAME);
    } else {
        pin_cpu(1);

        while (1) {
            while (!shm->ready);

            size_t size = shm->size;

            // simulate read access
            volatile char sink = 0;
            for (size_t i = 0; i < size; i += 64)
                sink ^= shm->buffer[i];

            shm->ready = 0;
            shm->done = 1;
        }
    }

    return 0;
}

