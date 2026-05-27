#ifndef SOCKET_BENCH_H
#define SOCKET_BENCH_H

#include"mem_bench.h"
#define PATH /tmp/socket

typedef struct{
	int type;
	int domain;
	const*path;
	int use_socketpair;
	struct sockaddr_un addr;

} socket_config_t;
typedef struct {
	int domain;
	int type;
	int listen_fd;
	int peer_fd;
	char*path[108];
	int is_server;
	int initialized;
}socket_struct_t;

extern socket_struct_t global_socket;

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


int server_init(const socket_config_t*cfg,const*path,const int type);
int client_init(const socket_config_t*cfg,const*path,const int type);

ssize_t read_all(int fd,char*buffer,size_t size);
ssize_t write_all(int fd, const char*buffer,size_t size);


void socket_cleanup(socket_config_t *cfg);

#endif
