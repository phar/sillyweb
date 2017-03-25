#ifndef _SERVICE_H
#define _SERVICE_H
#include "common.h"
#include "server.h"
#include "client.h"
#include "vhost.h"


typedef struct service_data_t{
	int		active;
	SSL_CTX			 * sslctx;
	struct client_reputation *rep_head;
	struct server_t * serverhead;
	struct client_t * clienthead;
	struct  vhost_t * vhost_head;
	int servicesetuid;
	int servicesetgid;
	pthread_mutex_t client_list_mutex;
	pthread_mutex_t server_list_mutex;
}service_data_t;


extern service_data_t SERVICE_DATA;

#endif

