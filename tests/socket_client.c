#include"../include/socket_bench.h"

#define UNIX_SOCKET_PATH "/tmp/unix_socket"

int main(int argc, char **argv){
    size_t min_bytes = 1;
    size_t max_bytes = 1024ULL*1024ULL;
    int iters = 1000;
    int warmup = 100;
    int domain=AF_UNIX;
    int type=SOCK_STREAM;


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
    if(unix_client_init(&socket,UNIX_SOCKET_PATH,domain,type)!=0){
        fprintf(stderr,"Failed to initialize client socket\n"); 
        return 1;
    }
    char *buffer=malloc(max_bytes);
    if(!buffer){
        perror("malloc");
        socket_cleanup(&socket);
        return 1;
    }
    
    for(size_t size=min_bytes;size<=max_bytes;size*=2){
        
        for(int i=0;i<warmup;i++){
            if(unix_write_all(&socket,buffer,size)!=size){
                fprintf(stderr,"Failed to write data to server\n");
                free(buffer);
                socket_cleanup(&socket);
                return 1;
            }
            if(unix_read_all(&socket,buffer,size)!=size){
                fprintf(stderr,"Failed to read data from server\n");
                free(buffer);                
                socket_cleanup(&socket);
                return 1;
            }
        }
        for(int i=0;i<iters;i++){
            if(unix_write_all(&socket,buffer,size)!=size){
                fprintf(stderr,"Failed to write data to server\n");
                free(buffer);
                socket_cleanup(&socket);
                return 1;
            }
            if(unix_read_all(&socket,buffer,size)!=size){
                fprintf(stderr,"Failed to read data from server\n");
                free(buffer);                
                socket_cleanup(&socket);
                return 1;
            }
        }
    }
    free(buffer);
    socket_cleanup(&socket);
    return 0;
 
}
ssize_t unix_read_all(socket_struct_t*socket,char*buffer,size_t size){       
