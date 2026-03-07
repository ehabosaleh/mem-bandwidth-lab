#include"../include/mq_bench.h"

void send_all(mqd_t mq, const void *buf, size_t size){
    if(mq_send(mq,buf,size)==-1){
        perror("Message Queue: Send");
        exit(EXIT_FAILURE);
    }
}

void recv_all(mqd_t mq, void*buff,size_t size){
    
    ssize_t ret=mq_receive(mq,buf,size,NULL);
    if(ret<0){
        perror("Message Queue: Recv");
        exit(EXIT_FAILURE);
    }

}
void child_loop(void){
    for(;;){
        unsigned char op;
        recv_all(mq_p2c,&op,1);
        if(op=='Q'){
            break;
        }

        uint64_t size;
        recv_all(mq_p2c, &size, sizeof(size));
        unsigned char ack=0xAC;
        send_all(mq_c2p,&ack,1);
        
        if(op=='L'){
            
            if(size){
                unsigned*buf=malloc(size);
                if(buf==NULL){
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                recv_all(mq_p2c, buf, size);
                send_all(mq_c2p, buf, size);
                free(buf);

            }

        }
    }
}

double measure_latency_one(size_t msg_size, int iters, int warmup){
        
    unsigned char op='L';
    uint64_t size=1;
    unsigned char ack;
    unsigned char*buf=malloc(1);
    
    for(int i=0;i<warmup;i++){  
        send_all(mq_p2c,&op,1);
        send_all(mq_p2c,&size,sizeof(size));
        recv_all(mq_c2p,&ack,1);

        if(buf==NULL){
            perror("Warm-up: malloc");
            exit(EXIT_FAILURE);
        }
        send_all(mq_p2c,buf,size);
        recv_all(mq_c2p,buf,size);
    }
    double t1=0.0;
    for(int i=0;i<iters;i++){
        unsigned char*buf=malloc(msg_size);
        if(buf==NULL){
               perror("Warm-up: malloc");
               exit(EXIT_FAILURE);
           }

        send_all(mq_p2c,&op,1);
        send_all(mq_p2c,&size,sizeof(size));
        recv_all(mq_c2p,&ack,1);

        double t0 = now_sec();
        send_all(mq_p2c,buf,size);
        recv_all(mq_c2p,buf,size);
        t1+=now_sec()-t0;
        free(buf);
    }
    double rtt=t1/iters;
    rtt=rtt/2.0;
    return rtt;
}
