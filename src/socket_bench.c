#include"../include/socket_bench.h"

void usage(const char *argv0){
	fprintf(stderr,
           "Usage: %s [--min-bytes=N] [--max-bytes=N] [--iters=N] [--warmup=N] [--domain=AF_UNIX/AF_INET] [--type=SOCK_STREAM/SOCK_DGRAM]\n"
           "Examples:\n"
           "  %s --min-bytes=1 --max-bytes=4MiB --domain=AF_UNIX --type=SOCK_DGRAM\n"
           "  %s --min-bytes=1 --max-bytes=4MiB --domain=AF_UNIX --type=SOCK_STREAM \n",
           argv0, argv0, argv0);
	exit(EXIT_FAILURE);
}
size_t parse_size(const char* s){
	char*end=NULL;
      	errno=0;
      	unsigned long long v=strtoull(s,&end,10);
      	if (errno!=0 || end==s){
			fprintf(stderr, "Invalid size: %s\n", s);
           	exit(EXIT_FAILURE);
		}
      	unsigned long long mul=1;
      	if(*end){
		if(strcasecmp(end,"k")==0) mul=1000ULL;
          	else if(strcasecmp(end,"m")==0) mul=1000ULL*1000ULL;
          	else if(strcasecmp(end,"g")==0) mul=1000ULL*1000ULL*1000ULL;
          	else if(strcasecmp(end,"kib")==0) mul=1024ULL;
          	else if(strcasecmp(end,"mib")==0) mul=1024ULL*1024ULL;
          	else if(strcasecmp(end,"gib")==0) mul=1024ULL*1024ULL*1024ULL;
		else{
               		fprintf(stderr, "Unknown size suffix: %s\n", end);
               		exit(EXIT_FAILURE);
           	}
      }

      if(mul!=0&&v>(unsigned long)SIZE_MAX/mul){
              fprintf(stderr,"Passed size is too large\n");
              exit(EXIT_FAILURE);
      }

      return (size_t)v*mul;
  }

int parse_string_to_domain(const char* s){
	if(strcasecmp(s,"AF_UNIX")==0||strcasecmp(s,"AF_LOCAL")==0)
		return AF_UNIX;
	else if(strcasecmp(s,"AF_INET")==0)
		return AF_INET;
	else if(strcasecmp(s,"AF_NET16")==0)
		return AF_NET16;
	else{
		fprintf(stderr, "Unknown domain: %s\n", s);
		exit(EXIT_FAILURE);
	}
}
int parse_string_to_type(const char* s){
	if(strcasecmp(s,"SOCK_STREAM")==0)
		return SOCK_STREAM;
	else if(strcasecmp(s,"SOCK_DGRAM")==0)
		return SOCK_DGRAM;
	else{
		fprintf(stderr, "Unknown type: %s\n", s);
		exit(EXIT_FAILURE);
	}
}

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
int unix_server_init(socket_struct_t*s,const*char path,const int domain,const int type){
	struct sockaddr_un addr;
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
	memset(s,0,sizeof(*s));
	
	s->domain=domain;
	s->type=type;
	s->is_server=1;
	s->listen_fd=-1;
	s->fd=-1;

	if(strlen(path)>=sizeof(s->path)){
		errno=ENAMETOOLONG;
		return -1;
	}
	/*	
	if(init_unix_addr(path,&addr,type)!=0){
		return -1;
	}
	*/
	memset(&addr,0,sizeof(addr));
	addr.sun_family=domain;
	strncpy(addr.sun_path,path,sizeof(addr.sun_path)-1);
	unlink(path);

	if(type==SOCK_STREAM){
		s->listen_fd=socket(domain,SOCK_STREAM,0);
		if(s->listen_fd<0){
			perror("socket");
			return -1;
		}
		if(bind(s->listen_fd,(struct sockaddr*)&addr,sizeof(addr))!=0){
			perror("bind");
			close(s->listen_fd);
			return -1;
		}
		if(listen(s->listen_fd,1)!=0){
			perror("listen");
			close(s->listen_fd);
			unlink(path);
			return -1;
		
		}
		s->fd=accept(s->listen_fd,NULL,NULL);
		if(s->fd<0){
			perror("accept");
			close(s->listen_fd);
			unlink(path);
			return -1;
		}
	
	}
	else if(type==SOCK_DGRAM){
		s->fd=socket(domain,SOCK_DGRAM,0);
		if(s->fd<0){
			perror("socket");
			return -1;
		}
		if(bind(s->fd,(struct sockaddr*)&addr,sizeof(addr))!=0){
			perror("bind");
			close(s->fd);
			unlink(path);
			return -1;
		}
	
	}
	s->initialized=1;
	return 0;
}

int unix_client_init( socket_struct_t *s,const char*path,const int domain,const int type){
	struct sockaddr_un server_addr;
	if(!path||!s){
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
	memset(s,0,sizeof(*s));
	s->domain=domain;
	s->type=type;
	s->is_server=0;
	s->listen_fd=-1;
	s->fd=-1;	

	if(strlen(path)>=sizeof(s->path)){
		errno=ENAMETOOLONG;
		return -1;
	}
	strncpy(s->path,path,sizeof(s->path)-1);
	/*if(init_unix_addr(path,&addr,type)!=0){
		return -1;
	}*/
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sun_family=domain;
	strncpy(server_addr.sun_path,path,sizeof(server_addr.sun_path)-1);

	s->fd=socket(domain,type,0);
	if(s->fd<0){
		perror("socket");
		return -1;
	}
	if(type==SOCK_STREAM){
		if(connect(s->fd,(struct sockaddr*)&server_addr,sizeof(server_addr))!=0){
			perror("connect");
			close(s->fd);
			return -1;
		} 
	}else if(type==SOCK_DGRAM){
		if(connect(s->fd,(struct sockaddr*)&server_addr,sizeof(server_addr))!=0){
			perror("connect");
			close(s->fd);
			return -1;
		}	
	}
	s->initialized=1;
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
	return total_read;
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

