#ifndef DMA_BENCH_H
#define DMA_BENCH_H
#include"mem_bench.h"

struct ibv_context;
struct ibv_pd;
struct ibv_mr;
struct ibv_cq;
struct ibv_qp;  

struct rdma_resource{
    struct ibv_context *ctx;
    struct ibv_pd *pd;
    struct ibv_mr *mr;
    struct ibv_cq *cq;
    struct ibv_qp *qp;
    void *buffer;
    size_t buffer_size;
};

struct rdma_resource* rdma_resource_init(size_t size);
int rdma_resource_cleanup(struct rdma_resource* res);
int rdma_write(struct rdma_resource* res, void* buf, size_t size, uint64_t remote_addr, uint32_t rkey);
int rdma_read(struct rdma_resource* res, void* buf, size_t size, uint64_t remote_addr, uint32_t rkey);

int rdma_check_completion(struct rdma_resource* res);

int rdma_exchange_info(struct rdma_resource* res, uint64_t* remote_addr, uint32_t* rkey);

#endif