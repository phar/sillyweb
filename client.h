#ifndef _CLIENT_H
#define _CLIENT_H
#include "common.h"
//static struct client_t * clienthead = NULL;


typedef struct client_t{
	int active;
	int thread_pool_id;
	pthread_mutex_t mutex;
	
	int sock;
	int flags;
	
	struct buffer_t  * inputbuffer;
	struct buffer_t * outputbuffer;
	
	struct header_t * inbound_headers;
	struct header_t * outbound_headers;
	
	int method_id;
	
	char * uri;
	char * uriclean;
	char * query;
	char * ipaddrstr;
	unsigned short srcport;
	char * hostnamestr;
	char * filename;
	char * path;
	
	int pump_fd;
	int has_pump_fd;
	
	int client_id;
	
	int request_complete;
	
	int response_complete;
	int response_in_progress;
	int request_parsed;
	int transfer_complete;
	int transfer_in_progress;
	int	result_code;
	//int transfer_chunked_encoding;
	
	struct vhost_t  *vhost;
	

	struct server_t		*server;
	struct client_t		*next_client;
	struct transfer_t		*transfer;
	struct service_data_t	*service;
	
	SSL		*sslctx;
	
	time_t client_connected;
	
}client_t;
#include "transport.h"
#include "transfer.h"


void clear_client(client_t *client);
int create_new_client(int s, struct sockaddr_in * addr, struct server_t *server);
void delete_client(client_t *client);
void add_new_client_struct(client_t *newclient);
void del_client_struct(client_t *newclient);


#endif
