#include"../include/shm_mem_bench.h"


void usage(const char *argv0){
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

void *shm_init(int is_writer, shm_region **shm_out) {
	int fd=shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    	if(fd<0){
		perror("shm_open");
		exit(1); 
	}
	size_t total_size = sizeof(shm_region)+MAX_SIZE;
	if(is_writer){
    		ftruncate(fd, total_size);
	}

    	void *addr=mmap(NULL,total_size,PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
	if(addr==MAP_FAILED){
		perror("mmap"); 
		exit(1); 
	}

    	*shm_out = (shm_region *)addr;
    	return addr;
}

void shm_cleanup(int is_writer, shm_region *shm, void *addr, size_t total_size) {
	munmap(addr, total_size);
    	if(is_writer)
		shm_unlink(SHM_NAME);
}

void shm_send_signal(shm_region *shm) {
    	shm->ready = 1;
}

void shm_wait_done(shm_region *shm) {
    	while(!shm->done);
    	shm->done = 0;
}

void shm_wait_ready(shm_region *shm) {
    	while(!shm->ready);
    	shm->ready = 0;
}

void shm_signal_done(shm_region *shm) {
	shm->done = 1;
}

void shm_memcpy_write(shm_region *shm, const void *src, size_t size){
	memcpy(shm->buffer,src,size);
}

void shm_memcpy_read(shm_region *shm, void *dst, size_t size){
        memcpy(dst,shm->buffer,size);

}

void run_writer(shm_region *shm, int core, int warmup, int iters){
	pin_cpu(core);
	char *src=aligned_alloc(64,MAX_SIZE);
	memset(src,0xAB,MAX_SIZE);
	printf("#Size\tLatency(us)\tBandwidth(GB/s)\n");
	
	for(size_t size=1;size<=MAX_SIZE;size*=2){
		shm->size=size;
		
		for(int i=0;i<warmup;i++){
            		shm_memcpy_write(shm,src, size);
			shm_send_signal(shm);
            		shm_wait_done(shm);
		}
		
		double start=now_sec();
        	for(int i=0;i<iters;i++) {
            		shm_memcpy_write(shm,src,size);
            		shm_send_signal(shm);
            		shm_wait_done(shm);
        	}
        	double end=now_sec();
		
		double t=(end-start)/iters;
        	double latency=t/2.0;
        	double bw=(double)size/latency/1e9;

        	printf("%zu\t%.3f\t%.3f\n",size,latency*1e6, bw);
	}
}

void run_reader(shm_region *shm,int core){
	pin_cpu(core);
	char *dst=aligned_alloc(64, MAX_SIZE);
    	
	while(1){
        	shm_wait_ready(shm);

        	size_t size = shm->size;
        	shm_memcpy_read(shm, dst, size);
        	shm_signal_done(shm);
    	}
}
