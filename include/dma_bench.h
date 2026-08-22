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

struct rdma_connection_info{
    uint32_t qp_num;
    uint32_t psn;
    uint32_t rkey;
    uint64_t buffer_addr;
    uint16_t lid;
    uint8_t  mtu;
};

struct rdma_resource* rdma_resource_init(size_t size); // reset state
int rdma_qp_to_initial(struct rdma_resource* res,uint8_t port_num);// init state
int rdma_qp_to_rtr(struct rdma_resource* res,const struct rdma_connection_info *local,const struct rdma_connection_info *remote,uint8_t port_num);// ready to receive state
int rdma_qp_to_rts(struct rdma_resource* res,uint32_t local_psn);// ready to send state
int rdma_get_local_info(struct rdma_resource* res, struct rdma_connection_info* info, uint8_t port_num);

int rdma_resource_cleanup(struct rdma_resource* res);


int rdma_check_completion(struct rdma_resource* res);

int rdma_exchange_info_server(struct rdma_resource*res, struct rdma_connection_info*local_info, struct rdma_connection_info*remote_info);
int rdma_exchange_info_client(struct rdma_resource* res, struct rdma_connection_info* local_info, struct rdma_connection_info* remote_info);

int rdma_write(struct rdma_resource* res, void* buf, size_t size, uint64_t remote_addr, uint32_t rkey);
int rdma_read(struct rdma_resource* res, void* buf, size_t size, uint64_t remote_addr, uint32_t rkey);

#endif