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

int unix_client_init( socket_struct_t *socket,const*path,const int domain,const int type){
	struct socketaddr_un server_addr;
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
	socket->is_server=0;
	socket->listen_fd=-1;
	socket->fd=-1;	

	if(strlen(socket->path)<=strlen(path)){
		errno=ENAMETOOLONG;
		return -1;
	}
	strncpy(socket->path,path,sizeof(socket->path)-1);
	/*if(init_unix_addr(path,&addr,type)!=0){
		return -1;
	}*/
	memset(&server_addr,0,sizeof(socketaddr_un));
	server_addr->su_family=type;
	
	socket->fd=socket(domain,type,0);
	if(socket->fd<0){
		perror("socket");
		return -1;
	}
	if(type==SOCK_STREAM){
		if(connect(socket->fd,(struct sockaddr*)&server_addr,sizeof(server_addr))!=0){
			perror("connect");
			close(socket->fd);
			return -1;
		} 
	}else if(type==SOCK_DGRAM){
		if(connect(socket->fd,(struct sockaddr*)&server_addr,sizeof(server_addr))!=0){
			perror("connect");
			close(socket->fd);
			return -1;
		}	
	}
	socket->initialized=1;
	return 0;
}

ssize_t unix_read_all(socket_struct_t*socket,char*buffer,size_t size){
	size_t total_read=0;

	if(!socket||!buffer||size==0||!socket->initialized){
		errno=EINVAL;
		return -1;
	}
	if(socket->type==SOCK_STREAM){
		while(total_read<size){
			ssize_t bytes_read=read(socket->fd,buffer+total_read,size-total_read);
			if(bytes_read<0){
				if(errno==EINTR)
					continue;
				else
					return -1;
			}
			else if(bytes_read==0){
				break;
			}
			total_read+=bytes_read;
		}
	}
	else if(socket->type==SOCK_DGRAM){
		size_t bytes_read=0;
		do{
			bytes_read=recv(socket->fd,buffer,size,0);
		}while(bytes_read<0&&errno==EINTR);
		if(bytes_read<0){
			perror("recv");
			return -1;
		}
		total_read=bytes_read;
		
	}
	
}
ssize_t unix_write_all(socket_struct_t*socket,const char*buffer,size_t size){
	size_t total_written=0;
	if(!socket||!buffer||size==0||!socket->initialized){
		errno=EINVAL;
		return -1;
	}
	if(socket->type==SOCK_STREAM){
		while(total_written<size){
			ssize_t bytes_written=write(socket->fd,buffer+total_written,size-total_written);
			if(bytes_written<0){
				if(errno==EINTR)
						continue;
				else
					return -1;
			}
			total_written+=bytes_written;
		}
	}
	else if(socket->type==SOCK_DGRAM){
		ssize_t bytes_written=0;
		do{
			bytes_written=send(socket->fd,buffer,size,0);
		}while(bytes_written<0&&errno==EINTR);
		if(bytes_written<0){
			perror("send");
			return -1;
		}
		total_written=bytes_written;
	}
	return total_written;
}
void socket_cleanup(socket_struct_t*socket){
	if(!socket||!socket->initialized)
		return;
	if(socket->is_server&&socket->listen_fd>=0){
		close(socket->listen_fd);
		unlink(socket->path);
	}
	if(socket->fd>=0){
		close(socket->fd);
	}
	socket->initialized=0;
}
