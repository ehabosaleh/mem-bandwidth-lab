#include"../include/mq_bench.h"
mqd_t mq_p2c;
mqd_t mq_c2p;

void send_all(mqd_t mq, const void *buf, size_t size){
	struct mq_attr attr;
	mq_getattr(mq, &attr);
	size_t chunk=attr.mq_msgsize;
	for(size_t offset=0;offset<size;offset+=chunk){
		size_t n=(size-offset<chunk)?size-offset:chunk;
		if(mq_send(mq,buf+offset,n,0)==-1){
        		perror("Message Queue: Send");
        		exit(EXIT_FAILURE);
    		}
	}
}

void recv_all(mqd_t mq, void*buf,size_t size){
	struct mq_attr attr;
    	mq_getattr(mq, &attr);
	size_t chunk=attr.mq_msgsize;
	size_t received=0;
	ssize_t ret=0;
    	for(size_t received=0;received<size;received+=ret){
		
		ret=mq_receive(mq,buf+received,chunk,NULL);
    		if(ret<0){
        		perror("Message Queue: Recv");
        		exit(EXIT_FAILURE);
    		}
		
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
    	uint64_t size=msg_size;
   	unsigned char ack;
    	unsigned char*buf=malloc(size);
    	for(int i=0;i<warmup;i++){  
        	send_all(mq_p2c,&op,1);
        	send_all(mq_p2c,&size,sizeof(size));
        	recv_all(mq_c2p,&ack,1);

		if(buf==NULL){
			printf("%ld",size);
            		perror("Warm-up");
            		exit(EXIT_FAILURE);
        	}
	
        	send_all(mq_p2c,buf,size);
        	recv_all(mq_c2p,buf,size);
    	}

    	double t1=0.0;
	for(int i=0;i<iters;i++){
        	unsigned char*buf=malloc(size);
        	if(buf==NULL){
               		perror("Warm-up");
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

void mq_setup(size_t max_msg){

    struct mq_attr attr;
    attr.mq_flags=0;
    attr.mq_maxmsg=10;
    attr.mq_msgsize=max_msg;
    attr.mq_curmsgs=0;

    mq_unlink(P2C);
    mq_unlink(C2P);

    mq_p2c=mq_open(P2C,O_CREAT|O_RDWR, 0666, &attr);
    mq_c2p=mq_open(C2P,O_CREAT|O_RDWR, 0666, &attr);

    if (mq_p2c==(mqd_t)-1||mq_c2p==(mqd_t)-1){
        perror("Message Queue: mq_open");
        exit(EXIT_FAILURE);
    }

}

void mq_cleanup(void){
    mq_close(mq_p2c);
    mq_close(mq_c2p);

    mq_unlink(P2C);
    mq_unlink(C2P);
}

 void usage(const char *argv0) {
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
          else {
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
