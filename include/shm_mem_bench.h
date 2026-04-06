#ifndef SHM_MEM_BENCH_H
#define SHM_MEM_BENCH_H

#include <sys/mman.h>
#include"mem_bench.h"

#define SHM_NAME "/shm_bench"
#define MAX_SIZE (1UL << 30)

typedef struct{
	volatile int ready;
    	volatile int done;
    	size_t size;
    	char *buffer;
} shm_region;


void usage(const char *argv0);
size_t parse_size(const char* s);
void pin_cpu(int cpu);

void *shm_init(int is_writer, shm_region **shm_out);
void shm_cleanup(int is_writer, shm_region *shm, void *addr, size_t total_size);

void shm_send_signal(shm_region *shm);
void shm_wait_done(shm_region *shm);
void shm_wait_ready(shm_region *shm);
void shm_signal_done(shm_region *shm);

void shm_memcpy_write(shm_region *shm, const void *src, size_t size);
void shm_memcpy_read(shm_region *shm, void *dst, size_t size);

void run_writer(shm_region *shm,int core, int warmup, int iters);
void run_reader(shm_region *shm,int core);

#endif
