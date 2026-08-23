#include"../include/dma_bench.h"

int main(int argc,char**argv){
    size_t min_bytes = 1;
    size_t max_bytes = 1024ULL*1024ULL;
    int iters = 1000;
    int warmup = 100;
    char*rdma_op="write";

    for(int i=1;i<argc;i++) {
        if(strncmp(argv[i], "--min-bytes=", 12) == 0) min_bytes = parse_size(argv[i] + 12);
        else if (strncmp(argv[i], "--max-bytes=", 12) == 0) max_bytes = parse_size(argv[i] + 12);
        else if (strncmp(argv[i], "--iters=", 8) == 0) iters = atoi(argv[i] + 8);
        else if (strncmp(argv[i], "--warmup=", 9) == 0) warmup = atoi(argv[i] + 9);
        else if (strncmp(argv[i], "--rdma-op=", 10) == 0) rdma_op = argv[i] + 10;
        else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
             usage(argv[0]);
        }else{
             fprintf(stderr, "Unknown arg: %s\n", argv[i]);
             dma_usage(argv[0]);
         }
    }
    for(size_t size=min_bytes;size<=max_bytes;size*=2){
        struct rdma_resource *res=rdma_resource_init(size);
        if(!res){
            fprintf(stderr,"Failed to initialize RDMA resource\n");
            return 1;
        }
        if(rdma_qp_to_initial(res,1)!=0){
            fprintf(stderr,"Failed to transition QP to INIT\n");
            rdma_resource_cleanup(res);
            return 1;
        }
        struct rdma_connection_info local_info;
        if(rdma_get_local_info(res,&local_info,1)!=0){
            fprintf(stderr,"Failed to get local connection info\n");
            rdma_resource_cleanup(res);
            return 1;
        }
        struct rdma_connection_info remote_info;
        socket_struct_t socket;
        if(rdma_exchange_info_client(res,&local_info,&remote_info,&socket)!=0){
            fprintf(stderr,"Failed to exchange connection info with server\n");
            rdma_resource_cleanup(res);
            return 1;
        }
        for(int i=0;i<warmup;i++){
            rdma_send_control_message(&socket,RDMA_MSG_READY);
            rdma_receive_control_message(&socket,RDMA_MSG_DONE);
        }
        
        for(int i=0;i<iters;i++){
           rdma_send_control_message(&socket,RDMA_MSG_READY);
           rdma_receive_control_message(&socket,RDMA_MSG_DONE);
        }
        rdma_resource_cleanup(res);
        socket_cleanup(&socket);
    }
    return 0;
}