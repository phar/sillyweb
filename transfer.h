#ifndef _TRANSFER_H
#define _TRANSFER_H
#include "common.h"
#include "server.h"



typedef struct transfer_t{
	char * name;
	int	(*open)(struct client_t  *client,const char *desc);
	ssize_t (*send) (struct client_t  *client);
	ssize_t (*recv) (struct client_t  * client);
	int (*close)(struct client_t  *client);
}transfer_t;


int	plain_transfer_open( client_t *client,const char *desc);
ssize_t plain_transfer_send( client_t *client);
ssize_t plain_transfer_recv( client_t *client);
int plain_transfer_close( client_t *client);

int	chunked_transfer_open( client_t *client,const char *desc);
ssize_t chunked_transfer_send( client_t *client);
ssize_t chunked_transfer_recv( client_t *client);
int chunked_transfer_close( client_t *client);

int	gzip_transfer_open( client_t *client,const char *desc);
ssize_t gzip_transfer_send( client_t *client);
ssize_t gzip_transfer_recv( client_t *client);
int gzip_transfer_close( client_t *client);


extern struct transfer_t SERVER_TRANSFER_METHODS[];

#endif
