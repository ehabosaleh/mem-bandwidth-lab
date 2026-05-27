#ifndef SOCKET_BENCH_H
#define SOCKET_BENCH_H

#include"mem_bench.h"

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
	int fd;
	int listen_fd;
	int peer_fd;
	int sv[2];
	char*path[108];
	int is_server;
	int initialized;
	int use_socketpair;
}socket_struct_t;

extern socket_struct_t global_socket;

int socketpair_init(int type);
int server_init(const socket_config_t*cfg);
int client_init(const socket_config_T*cfg);

void read_all(int fd,char*buffer,size_t size);
void write_all(int fd, const char*buffer,size_t size);


void socket_cleanup(void);

#endif
