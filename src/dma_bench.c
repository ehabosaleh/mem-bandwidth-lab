#include"../include/dma_bench.h"
#include"../include/socket_bench.h"

void dma_usage(const char* prog_name){
    fprintf(stderr,
           "Usage: %s [--min-bytes=N] [--max-bytes=N] [--iters=N] [--warmup=N] [--rdma-op=write/read]\n"
           "Examples:\n"
           "  %s --min-bytes=1 --max-bytes=4MiB --iters=1000 --warmup=100 --rdma-op=write\n"
           "  %s --min-bytes=1 --max-bytes=4MiB --iters=1000 --warmup=100 --rdma-op=read\n",
           prog_name, prog_name, prog_name);
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

struct rdma_resource *rdma_resource_init(size_t size){
    struct rdma_resource *res=NULL;
    struct ibv_device **device_list=NULL;
    int num_devices=0;

    if(size==0){
        perror("Invalid size for RDMA resource");
        return NULL;
    }

    res=calloc(1,sizeof(*res));
    if (!res){
        perror("calloc rdma_resource");
        return NULL;
    }

    device_list=ibv_get_device_list(&num_devices);
    if (!device_list){
        perror("ibv_get_device_list");
        return NULL;
    }

    if (num_devices==0){
        fprintf(stderr, "No RDMA devices were found\n");
        return NULL;
    }

    res->ctx = ibv_open_device(device_list[0]);
    if(!res->ctx){
        perror("ibv_open_device");
        return NULL;
    }

    ibv_free_device_list(device_list);
    device_list = NULL;

    res->pd = ibv_alloc_pd(res->ctx);
    if(!res->pd){
        perror("ibv_alloc_pd");
       return NULL;
    }
    res->buffer=calloc(1, size);
    if(!res->buffer){
        perror("calloc RDMA buffer");
        return NULL;
    }

    res->buffer_size=size;
    res->mr=ibv_reg_mr(res->pd,res->buffer,size,IBV_ACCESS_LOCAL_WRITE|IBV_ACCESS_REMOTE_READ|IBV_ACCESS_REMOTE_WRITE);

    if(!res->mr){
        perror("ibv_reg_mr");
        return NULL;
    }

    res->cq=ibv_create_cq(res->ctx,1,NULL,NULL,0);

    if (!res->cq){
        perror("ibv_create_cq");
        return NULL;
    }

    struct ibv_qp_init_attr qp_attr = {0};

    qp_attr.qp_type=IBV_QPT_RC;
    qp_attr.send_cq=res->cq;
    qp_attr.recv_cq=res->cq;

    qp_attr.cap.max_send_wr=1;
    qp_attr.cap.max_recv_wr=1;
    qp_attr.cap.max_send_sge=1;
    qp_attr.cap.max_recv_sge=1;

    qp_attr.sq_sig_all=0;

    res->qp = ibv_create_qp(res->pd, &qp_attr);
    if (!res->qp){
        perror("ibv_create_qp");
        return NULL;
    }

    return res;

}

int rdma_qp_to_initial(struct rdma_resource* res,uint8_t port_num){
    struct ibv_qp_attr attr = {0};
    int flags=0;
    int ret=1;

    attr.qp_state = IBV_QPS_INIT;
    attr.port_num = port_num;
    attr.pkey_index = 0;
    attr.qp_access_flags = IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE;

    flags = IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS;

    ret = ibv_modify_qp(res->qp, &attr, flags);
    if(ret) {
        perror("ibv_modify_qp to INIT");
        return -1;
    }
    return 0;
}

int rdma_qp_to_rtr(struct rdma_resource* res,const struct rdma_connection_info *local,const struct rdma_connection_info *remote,uint8_t port_num){
    struct ibv_qp_attr attr = {0};
    int flags=0;
    int ret=1;

    attr.qp_state = IBV_QPS_RTR;
    attr.path_mtu = (enum ibv_mtu)(local->mtu<remote->mtu?local->mtu:remote->mtu);
    attr.dest_qp_num = remote->qp_num;
    attr.rq_psn = remote->psn;
    attr.max_dest_rd_atomic = 1;
    attr.min_rnr_timer = 12;
    attr.ah_attr.is_global = 0;
    attr.ah_attr.dlid = remote->lid;
    attr.ah_attr.sl = 0;
    attr.ah_attr.src_path_bits = 0;
    attr.ah_attr.port_num = port_num;

    flags = IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN |
            IBV_QP_RQ_PSN | IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER;

    ret = ibv_modify_qp(res->qp, &attr, flags);
    if(ret) {
        perror("ibv_modify_qp to RTR");
        return -1;
    }
    return 0;
}
int rdma_qp_to_rts(struct rdma_resource* res,uint32_t local_psn){
    struct ibv_qp_attr attr = {0};
    int flags=0;
    int ret=1;

    attr.qp_state = IBV_QPS_RTS;
    attr.timeout = 14;
    attr.retry_cnt = 7;
    attr.rnr_retry = 7;
    attr.sq_psn=local_psn;
    attr.max_rd_atomic = 1;

    flags = IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
            IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN | IBV_QP_MAX_QP_RD_ATOMIC;

    ret = ibv_modify_qp(res->qp, &attr, flags);
    if(ret) {
        perror("ibv_modify_qp to RTS");
        return -1;
    }
    return 0;
}
int rdma_get_local_info(struct rdma_resource* res, struct rdma_connection_info* info, uint8_t port_num){
    if(!res || !info){
        errno=EINVAL;
        return -1;
    }
    struct ibv_port_attr port_attr;
    if(ibv_query_port(res->ctx, port_num, &port_attr)){
        perror("ibv_query_port");
        return -1;
    }
    info->qp_num = res->qp->qp_num;
    info->psn = 0;
    info->rkey = res->mr->rkey;
    info->buffer_addr = (uint64_t)(uintptr_t)res->mr->addr;
    info->lid = port_attr.lid;
    info->mtu = port_attr.active_mtu;

    return 0;
}

int rdma_exchange_info_server(struct rdma_resource*res, struct rdma_connection_info*local_info, struct rdma_connection_info*remote_info){
    if(!res||!local_info||!remote_info){
        errno=EINVAL;
        return -1;
    }
    socket_struct_t socket;
    if(inet_server_init(&socket,NULL,12345,AF_INET,SOCK_STREAM)!=0){
        fprintf(stderr,"Failed to initialize server socket\n");
        return -1;
    }
    if(write_all(&socket,(char*)local_info,sizeof(*local_info))!=sizeof(*local_info)){
        fprintf(stderr,"Failed to send local connection info\n");
        socket_cleanup(&socket);
        return -1;
    }
   
    if(read_all(&socket,(char*)remote_info,sizeof(*remote_info))!=sizeof(*remote_info)){
        fprintf(stderr,"Failed to receive remote connection info\n");
        socket_cleanup(&socket);
        return -1;
    }
    socket_cleanup(&socket);
    if(rdma_qp_to_rtr(res,local_info,remote_info,1)!=0){
        fprintf(stderr,"Failed to transition QP to RTR\n");
        return -1;
    }
    if(rdma_qp_to_rts(res,local_info->psn)!=0){
        fprintf(stderr,"Failed to transition QP to RTS\n");
        return -1;
    }
    return 0;
}

int rdma_exchange_info_client(struct rdma_resource* res, struct rdma_connection_info* local_info, struct rdma_connection_info* remote_info){
    if(!res||!local_info||!remote_info){
        errno=EINVAL;
        return -1;
    }
    socket_struct_t socket;
    if(inet_client_init(&socket,NULL,12345,AF_INET,SOCK_STREAM)!=0){
        fprintf(stderr,"Failed to initialize client socket\n");
        return -1;
    }
    if(read_all(&socket,(char*)remote_info,sizeof(*remote_info))!=sizeof(*remote_info)){
        fprintf(stderr,"Failed to receive remote connection info\n");
        socket_cleanup(&socket);
        return -1;
    }
    if(write_all(&socket,(char*)local_info,sizeof(*local_info))!=sizeof(*local_info)){
        fprintf(stderr,"Failed to send local connection info\n");
        socket_cleanup(&socket);
        return -1;
    }
    socket_cleanup(&socket);
    if(rdma_qp_to_rtr(res,local_info,remote_info,1)!=0){
        fprintf(stderr,"Failed to transition QP to RTR\n");
        return -1;
    }
    if(rdma_qp_to_rts(res,local_info->psn)!=0){
        fprintf(stderr,"Failed to transition QP to RTS\n");
        return -1;
    }
    return 0;
}


int rdma_write(struct rdma_resource* res, void* buf, size_t size, uint64_t remote_addr, uint32_t rkey) {
    struct ibv_sge sge = {0};
    sge.addr = (uintptr_t)buf;
    sge.length = size;
    sge.lkey = res->mr->lkey;

    struct ibv_send_wr wr = {0};
    wr.wr_id = 1;
    wr.opcode = IBV_WR_RDMA_WRITE;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr.rdma.remote_addr = remote_addr;
    wr.wr.rdma.rkey = rkey;

    struct ibv_send_wr *bad_wr;
    if (ibv_post_send(res->qp, &wr, &bad_wr)) {
        perror("ibv_post_send");
        return -1;
    }
    return 0;
}
int rdma_read(struct rdma_resource* res, void* buf, size_t size, uint64_t remote_addr, uint32_t rkey) {
    struct ibv_sge sge = {0};
    sge.addr = (uintptr_t)buf;
    sge.length = size;
    sge.lkey = res->mr->lkey;

    struct ibv_send_wr wr = {0};
    wr.wr_id = 1;
    wr.opcode = IBV_WR_RDMA_READ;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;
    wr.wr.rdma.remote_addr = remote_addr;
    wr.wr.rdma.rkey = rkey;

    struct ibv_send_wr *bad_wr;
    if (ibv_post_send(res->qp, &wr, &bad_wr)) {
        perror("ibv_post_send");
        return -1;
    }

    return 0;
}

int rdma_check_completion(struct rdma_resource* res) {
    struct ibv_wc wc;
    int num_completions = ibv_poll_cq(res->cq, 1, &wc);
    if (num_completions < 0) {
        perror("ibv_poll_cq");
        return -1;
    } else if (num_completions == 0) {
        return 0; 
    } else {
        if (wc.status != IBV_WC_SUCCESS) {
            fprintf(stderr, "RDMA operation failed: %s\n", ibv_wc_status_str(wc.status));
            return -1;
        }
        return 1;
    }
}

int rdma_resource_cleanup(struct rdma_resource* res){
    if(!res)
        return -1;
    if(res->qp)
        ibv_destroy_qp(res->qp);
    if(res->cq)
        ibv_destroy_cq(res->cq);
    if(res->mr)
        ibv_dereg_mr(res->mr);
    if(res->pd)
        ibv_dealloc_pd(res->pd);
    if(res->ctx)
        ibv_close_device(res->ctx);
    free(res);
    return 0;
}