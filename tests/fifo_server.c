#include"../include/fifo_bench.h"

int main(){
    /*
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    printf("CWD = %s\n", cwd);
    printf("Creating: %s\n", SERVER_FIFO);
    */
    
    if(mkfifo(SERVER_FIFO,0666)<0 && errno!=EEXIST){
        perror("Server: mkfifo()");
        exit(EXIT_FAILURE);
    }
    if(mkfifo(CLIENT_FIFO,0666)<0 && errno!=EEXIST){
         perror("Server: mkfifo()");
         exit(EXIT_FAILURE);
     }
    
    int rfd=open(SERVER_FIFO,O_RDONLY);
    int wfd=open(CLIENT_FIFO,O_WRONLY);
    if(rfd<0||wfd<0){
        perror("FIFO:Open");
        exit(EXIT_FAILURE);
    }

    for(;;){
        uint64_t size;
        read_all(rfd,&size,sizeof(size));
        if(size==0)
            break;
        char*buf=malloc(size);
        read_all(rfd,buf,size);

        write_all(wfd,&size,sizeof(size));
        write_all(wfd,buf,size);

        free(buf);

    }

    close(rfd);
    close(wfd);
    unlink(SERVER_FIFO);
    unlink(CLIENT_FIFO);

}
