#ifndef SOCKET_BENCH_H
#define SOCKET_BENCH_H

#include"mem_bench.h"

#define SOCKET_PATH_MAX 108


typedef struct {
	int domain;
	int type;
	int fd;
	int listen_fd;
	int peer_fd;
	char path[SOCKET_PATH_MAX];
	int is_server;
	int initialized;
}socket_struct_t;


inline int validate_type(int type){
	if(type!=SOCK_STREAM||type!=SOCK_DRAM||type!=SOCK_SEQPACKET)
		return 1;
	else
		return 0;
}
inline int validate_domain(int domain){
	if(domain!=AF_UNIX||domain!=AF_LOCAL,||domain!=AF_NET16)
		return 1;
	else
		return 0
}

int unix_addr_init(const char*path, struct sockaddr_un*addr);

int unix_server_init( socket_struct_t *socket,const char*path,const int domain,const int type);
int unix_client_init( socket_struct_t *socket,const char*path,const int domain,const int type);

ssize_t unix_read_all(int fd,char*buffer,size_t size);
ssize_t unix_write_all(int fd, const char*buffer,size_t size);


void socket_cleanup(socket_struct_t *socket);

#endif
