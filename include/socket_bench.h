#ifndef SOCKET_BENCH_H
#define SOCKET_BENCH_H

#include"mem_bench.h"

#define SOCKET_PATH_MAX 108

#define UNIX_SOCKET_PATH "/tmp/unix_socket"//for both server and client when using the STREAM domain 

#define UNIX_SERVER_PATH "/tmp/unix_server" //For server socket unsing the DGRAM domain
#define UNIX_CLIENT_PATH "/tmp/unix_client"//For client socket using the DGRAM domain

#define DGRAM_CHUNK_SIZE 8192


typedef struct {
	int domain;
	int type;
	int fd;
	int listen_fd;
	int peer_fd;
	char path[SOCKET_PATH_MAX];
	struct sockaddr_un peer_addr;
	struct sockaddr_in peer_addr_in;
    socklen_t peer_len;
    int has_peer;
	int is_server;
	int initialized;
}socket_struct_t;

typedef struct{
	uint32_t msg_id;
	uint32_t chunk_id;
    uint32_t total_chunks;
    uint32_t payload_size;
}dgram_header_t;

static inline int validate_type(int type){
	if(type!=SOCK_STREAM&&type!=SOCK_DGRAM&&type!=SOCK_SEQPACKET)
		return 1;
	else
		return 0;
}
static inline int validate_domain(int domain){
	if(domain!=AF_UNIX&&domain!=AF_INET&&domain!=AF_INET6)
		return 1;
	else
		return 0;
}

int unix_addr_init(const char*path, struct sockaddr_un*addr);
int unix_server_init( socket_struct_t *socket,const char*path,const int domain,const int type);
int unix_client_init( socket_struct_t *socket,const char*path,const int domain,const int type);

int inet_addr_init(struct sockaddr_in *addr, const char*ip, const uint16_t port);
int inet_server_init(socket_struct_t *socket,const char*ip, const uint16_t port,const int type);
int inet_client_init(socket_struct_t *socket,const char*ip, const uint16_t port,const int type);

ssize_t read_all(socket_struct_t*s,char*buffer,size_t size);
ssize_t write_all(socket_struct_t*s, const char*buffer,size_t size);

void socket_cleanup(socket_struct_t *socket);

int parse_string_to_type(const char* s);
int parse_string_to_domain(const char* s);
#endif
