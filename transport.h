
#ifndef _TRANSPORT_H
#define _TRANSPORT_H
#include "common.h"
#include "server.h"

											
int	tcp_transport_open( client_t *,const char *desc);
ssize_t tcp_transport_send( client_t *, void *buffer, size_t length);
ssize_t tcp_transport_recv( client_t *, void *buffer, size_t length);
int tcp_transport_close( client_t *);

int	ssl_transport_open( client_t *,const char *desc);
ssize_t ssl_transport_send( client_t *,  void *buffer, size_t length);
ssize_t ssl_transport_recv( client_t *, void *buffer, size_t length);
int ssl_transport_close( client_t *);



typedef struct transport_t{
	char * name;
	int	(*open)(struct client_t *client,const char *desc);
	ssize_t (*send) (struct  client_t *client, void *buffer, size_t length);
	ssize_t (*recv) (struct  client_t *client, void *buffer, size_t length);
	int (*close)(struct client_t *client);
}transport_t;


extern struct transport_t SERVER_TRANSPORT_METHODS[];

#endif
