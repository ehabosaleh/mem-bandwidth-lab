#include "../include/shm_mem_bench.h"

int main(int argc, char **argv){
	size_t min_bytes = 1;
    	size_t max_bytes = 1024ULL*1024ULL;
    	int iters = 1000;
    	int warmup = 100;
	int writer_core=0,reader_core=1;
	bool is_writer=1;

    	for (int i=1;i<argc;i++) {
        	if(strncmp(argv[i], "--min-bytes=", 12) == 0) min_bytes = parse_size(argv[i] + 12);
        	else if(strncmp(argv[i], "--max-bytes=", 12) == 0) max_bytes = parse_size(argv[i] + 12);
        	else if(strncmp(argv[i], "--iters=", 8) == 0) iters = atoi(argv[i]+8);
        	else if(strncmp(argv[i], "--warmup=", 9) == 0) warmup = atoi(argv[i]+9);
		else if(strncmp(argv[i],"--reader_core=", 14)==0) reader_core=atoi(argv[i]+14);
		else if(strncmp(argv[i],"--writer_core=", 14)==0) writer_core=atoi(argv[i]+14);
        	else if(strncmp(argv[i],"--is_writer=",12 )==0) is_writer=(atoi(argv[i]+12)!=0);
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            		usage(argv[0]);
        	}else{
            		fprintf(stderr, "Unknown arg: %s\n", argv[i]);
            		usage(argv[0]);
        	}
    	}
	
	if(!is_writer){
		shm_region *shm;
		void *addr=shm_init(is_writer, &shm);
		run_reader(shm,reader_core);
		shm_cleanup(is_writer,shm, addr,sizeof(shm_region)+MAX_SIZE);
        	return 0;
	}
	
    	shm_region *shm;
    	void *addr = shm_init(is_writer, &shm);
	pid_t pid;
	char reader_core_arg[32];
	snprintf(reader_core_arg,sizeof(reader_core_arg), "--reader_core=%d",reader_core);

	pid=fork();
	if(pid<0){
		perror("fork():");
		exit(1);
	}
	if(pid==0){
		execl("./shm_mem","shm_mem_bench","--is_writer=0",reader_core_arg,NULL);
		perror("child–execl");
		exit(1);
	}
	
        run_writer(shm,writer_core,warmup,iters,min_bytes,max_bytes);
    	shm_cleanup(is_writer,shm,addr,sizeof(shm_region)+MAX_SIZE);
    	return 0;
}

