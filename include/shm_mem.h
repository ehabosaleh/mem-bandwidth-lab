#include <sys/mman.h>
#include"mem_bench.h"

#define SHM_NAME "/shm_bench"

typedef struct{
	volatile int ready;
    	volatile int done;
    	size_t size;
    	char *buffer;
} shm_region;


void usage(const char *argv0);
size_t parse_size(const char* s);
void pin_cpu(int cpu);
