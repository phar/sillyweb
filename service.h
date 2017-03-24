#ifndef _SERVICE_H
#define _SERVICE_H
#include "common.h"
#include "server.h"
#include "client.h"
#include "vhost.h"


typedef struct service_data_t{
	SSL_CTX			 * sslctx;
	struct server_t * serverhead;
	struct client_t * clienthead;
	struct  vhost_t * vhost_head;
	int servicesetuid;
	int servicesetgid;
}service_data_t;


extern service_data_t SERVICE_DATA;

#endif

