#include"../include/dma_bench.h"

int main(int argc, char** argv) {
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
             dma_usage(argv[0]);
        }else{
             fprintf(stderr, "Unknown arg: %s\n", argv[i]);
             dma_usage(argv[0]);
         }
    }

    socket_struct_t socket;
    struct rdma_resource *res=rdma_resource_init(max_bytes);
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
        
    if(rdma_exchange_info_server(res,&local_info,&remote_info,&socket)!=0){
            fprintf(stderr,"Failed to exchange connection info with client\n");
            rdma_resource_cleanup(res);
            return 1;
    }
    printf("Running DMA-%s benchmark \n",rdma_op);
    printf("%-20s %-20s %-20s\n","Bytes","Latency(us)","Bandwidth(MiB/s)");
    for(size_t size=min_bytes;size<=max_bytes;size*=2){
        
        
        for(int i=0;i<warmup;i++){
            rdma_receive_control_message(&socket,RDMA_MSG_READY);
            if(strcmp(rdma_op,"write")==0){
                
                if(rdma_write(res,res->buffer,size,remote_info.buffer_addr,remote_info.rkey)!=0){
                    fprintf(stderr,"Failed to perform RDMA write\n");
                    rdma_resource_cleanup(res);
                    return 1;
                }
            }else if(strcmp(rdma_op,"read")==0){
                if(rdma_read(res,res->buffer,size,remote_info.buffer_addr,remote_info.rkey)!=0){
                    fprintf(stderr,"Failed to perform RDMA read\n");
                    rdma_resource_cleanup(res);
                    return 1;
                }
            }

            if(rdma_wait_for_completion(res)!=0){
                fprintf(stderr, "RDMA operation failed\n");
                rdma_resource_cleanup(res);
                return 1;
            }
            rdma_send_control_message(&socket,RDMA_MSG_FIN);
        }

        double total_time=0.0;
       
        for(int i=0;i<iters;i++){
            rdma_receive_control_message(&socket,RDMA_MSG_READY);
             double start_time=now_sec();
            if(strcmp(rdma_op,"write")==0){
                if(rdma_write(res,res->buffer,size,remote_info.buffer_addr,remote_info.rkey)!=0){
                    fprintf(stderr,"Failed to perform RDMA write\n");
                    rdma_resource_cleanup(res);
                    return 1;
                }
            }else if(strcmp(rdma_op,"read")==0){
                if(rdma_read(res,res->buffer,size,remote_info.buffer_addr,remote_info.rkey)!=0){
                    fprintf(stderr,"Failed to perform RDMA read\n");
                    rdma_resource_cleanup(res);
                    return 1;
                }
            }
            if(rdma_wait_for_completion(res)!=0){
                fprintf(stderr, "RDMA operation failed\n");
                rdma_resource_cleanup(res);
                return 1;
            }
            total_time+=now_sec()-start_time;
            rdma_send_control_message(&socket,RDMA_MSG_FIN);
        }
        double latency=total_time/iters;
        double latency_us=latency*1e6;
        double bandwidth=(size/latency)/(1024.0*1024.0);
        printf("%-20zu %-20.3f %-20.3f\n",size, latency_us, bandwidth);
        
    }
    rdma_resource_cleanup(res);
    socket_cleanup(&socket);
    return 0;
    
   
    
}

