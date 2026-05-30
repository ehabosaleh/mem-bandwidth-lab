#include"../include/socket_bench.h"


int main(int argc, char **argv){

    size_t min_bytes = 1;
    size_t max_bytes = 1024ULL*1024ULL;
    int iters = 1000;
    int warmup = 100;
	int domain=AF_UNIX;
    int type=SOCK_STREAM;
    char *path=UNIX_SOCKET_PATH;

    for (int i=1;i<argc;i++) {
        if(strncmp(argv[i], "--min-bytes=", 12) == 0) min_bytes = parse_size(argv[i] + 12);
        else if(strncmp(argv[i], "--max-bytes=", 12) == 0) max_bytes = parse_size(argv[i] + 12);
        else if(strncmp(argv[i], "--iters=", 8) == 0) iters = atoi(argv[i]+8);
        else if(strncmp(argv[i], "--warmup=", 9) == 0) warmup = atoi(argv[i]+9);
		else if(strncmp(argv[i],"--domain=", 9)==0) domain=parse_string_to_domain(argv[i]+9);
		else if(strncmp(argv[i],"--type=", 7)==0) type=parse_string_to_type(argv[i]+7);
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
        }else{
            	fprintf(stderr, "Unknown arg: %s\n", argv[i]);
            	usage(argv[0]);
        	}
    	
    }
    socket_struct_t socket;
    if(domain==AF_UNIX){
        if(unix_server_init(&socket,path,domain,type)!=0){
            fprintf(stderr,"Failed to initialize server socket\n"); 
            return 1;
        }
    }
    else if(domain==AF_INET){
        uint16_t port=12345;
        if(inet_server_init(&socket,NULL,port,domain,type)!=0){
            fprintf(stderr,"Failed to initialize server socket\n"); 
            return 1;
        }
    }
    char *buffer=malloc(max_bytes);
    if(!buffer){
        perror("malloc");
        socket_cleanup(&socket);
        return 1;
    }
    
    printf("%-20s %-20s %-20s\n","Bytes","Latency(us)","Bandwidth(MiB/s)");    
    for(size_t size=min_bytes;size<=max_bytes;size*=2){
        
        for(int i=0;i<warmup;i++){
            if(read_all(&socket,buffer,size)!=size){
                fprintf(stderr,"Failed to read data from client\n");
                free(buffer);
                socket_cleanup(&socket);
                return 1;
            }
            if(write_all(&socket,buffer,size)!=size){
                fprintf(stderr,"Failed to write data to client\n");
                free(buffer);
                socket_cleanup(&socket);
                return 1;
            }
        }
        
        double total_time=0;
        double start_time=0,end_time=0;

        for(int i=0;i<iters;i++){
            start_time=now_sec();
            if(read_all(&socket,buffer,size)!=size){
                fprintf(stderr,"Failed to read data from client\n");
                free(buffer);
                socket_cleanup(&socket);
                return 1;
            }
            if(write_all(&socket,buffer,size)!=size){
                fprintf(stderr,"Failed to write data to client\n");
                free(buffer);
                socket_cleanup(&socket);
                return 1;
            }
            end_time=now_sec();
            total_time+=(end_time-start_time)*1e6;
        }
        double avg_latency=(double)(total_time/iters)/2.0;
        double bandwidth=(double)size/(1024*1024)/(avg_latency/1e6);
        printf("%-20zu %-20.2f %-20.2f\n",size,avg_latency,bandwidth);
    }

    return 0;
}

