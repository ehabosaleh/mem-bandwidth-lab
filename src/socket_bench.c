#include"../include/socket_bench.h"

int init_unix_addr(const char*path, struct sockaddr_un*addr){
	if(!path||!addr){
		perror("Invalid addr");
		return 1;
	}
	memset(addr,0,sizeof(*addr));
	addr->su_family=AF_UNIX;
	if(strlen(path)>=sizeof(addr->sun_path)){
		perror("path length");
		return 1;
	}
	strcnp(addr->sun_path,path,sizeof(addr->sun_path)-1)
	return 0;
}

