#ifndef _SERVER_H
#define _SERVER_H
#include "common.h"

#include "client.h"




typedef struct server_t{
	unsigned int port;
	int	server_sock;
	char  * bind_address;
	struct sockaddr_in addr;
	unsigned int	server_type;
	pthread_mutex_t client_list_mutex;
	int loglevel;
	int SSL;
	
	
	struct vhost_t *vhost_head;
	
	struct transport_t		*transport;	
	
	struct server_t *next_server;	
}server_t;



//#include "log.h"

void add_new_server_struct(server_t *newserver);
int get_server_count();
server_t *get_server_by_fd(int fd);


#endif
