#include"../include/socket_bench.h"

int init_unix_addr(const char*path, struct sockaddr_un*addr,int type){
	if(!path||!addr){
		errno=EINVAL;
		return -1;
	}
	memset(addr,0,sizeof(*addr));
	addr->su_family=type;
	if(strlen(path)>=sizeof(addr->sun_path)){
		errno=ENAMETOOLONG;
		return -1;
	}
	strcnp(addr->sun_path,path,sizeof(addr->sun_path)-1)
	return 0;
}
int unix_server_init(socket_struct_t*socket,const*path,const int domain,const int type){
	struct socketaddr_un addr;
	if(!path||!socket){
		errno=EINVAL;
		return -1;
	}
	if(validate(type)!=0){
		errno=EINVAL;
		return -1;
	}
	if(validate(domain)!=0){
		errno=EINVAL;
		return -1;
	}
	memset(socket,0,sizeof(*socket));
	
	socket->domain=domain;
	socket->type=type;
	socket->is_server=1;
	socket->listen_fd=-1;
	socket->fd=-1;

	if(strlen(socket->path)<=strlen(path)){
		errno=ENAMETOOLONG;
		return -1;
	}
	strncpy(s->path,path,sizeof(s->path)-1);
	/*	
	if(init_unix_addr(path,&addr,type)!=0){
		return -1;
	}
	*/
	memset(&addr,0,sizeof(socketaddr_un));
	addr->su_family=type;
	unlink(path);

	if(type==SOCK_STREAM){
		socket->listen_fd=socket(domain,SOCK_STREAM,0);
		if(socket->listen_fd<0){
			perror("socket");
			return -1;
		}
		if(bind(socket->listen_fd,(struct sockaddr*)&addr,sizeof(addr))!=0){
			perror("bind");
			close(socket->fd);
			return -1;
		}
		if(listen(socket->listen_fd<0,1)){
			perror("listen");
			close(socket->listen_fd);
			unlink(path);
		
		}
		socket->fd=accept(socket->listen_fd,NULL,NULL);
		if(socket->fd<=0){
			perror("accept");
			close(socket->listen_fd);
			unlink(path);
			return -1;
		}
	
	}
	else if(type==SOCK_DGRAM){
		socket->fd=socket(domain,SOCK_DGRAM,0);
		if(socket->fd<0){
			perror("socket");
			return -1;
		}
		if(bind(socket->fd,(struct sockaddr*)&addr,sizeof(addr))!=0){
			perror("bind");
			close(socket->fd);
			unlink(path);
			retrurn -1;
		}
	
	}
	socket->initialize=1;
	return 0;
}

