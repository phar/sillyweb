
#include <netdb.h>
#include "client.h"
#include "buffer.h"
#include "log.h"
#include "headers.h"
#include "transfer.h"
#include "transport.h"
#include "service.h"

/*****************************
 
	client request functions
 
 ******************************/

void add_new_client_rep_struct( struct client_reputation *newrep){
struct client_reputation *ptr;
	
	if(SERVICE_DATA.rep_head != NULL){
		for(ptr=SERVICE_DATA.rep_head; ptr->next_rep; ptr=ptr->next_rep);
		ptr->next_rep = newrep;
	}else{
		SERVICE_DATA.rep_head = newrep;
	}
}


void inc_client_rep_resource_counter(client_t *client){
	
}

void dec_client_rep_resource_counter(client_t *client){
	
}



void close_connect(client_t *client){
	client->active = 0;
	
	//	client->transfer_close(client);
	
}

void delete_client(client_t *client){
	
	llog(client, LLOG_LOG_LEVEL_DEBUG,"delete client");
	client->active = 0;

	client->server->transport->close(client);
	
	clear_client(client);
	
	del_client_struct(client);
}



void clear_client(client_t *client){
	
	if (client->uri){
		free(client->uri);
		client->uri = NULL;
	}
	
	if (client->uriclean){
		free(client->uriclean);
		client->uriclean = NULL;
	}
	
	if (client->query){
		free(client->query);
		client->query = NULL;
	}
	
	if(client->pump_fd)
		close(client->pump_fd);
	
	client->pump_fd = 0;
	free_buffer(client->outputbuffer);
	
	free_buffer(client->inputbuffer);
	
	del_headers(&client->inbound_headers);
	
	del_headers(&client->outbound_headers);
	
	client->request_complete = 0;
	client->request_parsed = 0;
	client->response_complete = 0;
	
	client->client_connected = time(NULL);
	
	client->flags = 0;
}

int create_new_client(int s, struct sockaddr_in * addr, struct server_t *server){
	client_t *newclient;
	char ip_addr_buffer[INET6_ADDRSTRLEN+1];
	struct hostent *he;
	
	llog((client_t *)NULL, LLOG_LOG_LEVEL_DEBUG,"connect!");
	
	if((newclient = calloc(1,sizeof(client_t)))){
		newclient->active = 1;
		newclient->sock = s;
		newclient->server = server;
		newclient->thread_pool_id = rand() % NUM_CPUS;
		
		
		newclient->inputbuffer = new_buffer();
		newclient->outputbuffer = new_buffer();

		 switch(addr->sin_family) {
			case AF_INET:
				inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), &ip_addr_buffer, INET_ADDRSTRLEN);
				break;
		 
			case AF_INET6:
				inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)addr)->sin6_addr), &ip_addr_buffer, INET6_ADDRSTRLEN);
				break;
		 
		 }
 
		newclient->ipaddrstr = strdup(ip_addr_buffer);
		
		newclient->transfer = &SERVER_TRANSFER_METHODS[0]; //FIXME
		newclient->vhost = SERVICE_DATA.vhost_head;

		newclient->client_connected  = time(NULL);
		newclient->srcport  = ntohs(addr->sin_port);
 
		add_new_client_struct(newclient);
		inc_client_rep_resource_counter(newclient);
 
		return 1;
	}else{
		llog((client_t *)NULL, LLOG_LOG_LEVEL_DEBUG,"MEMORY ALLOCATION FAILURE");
		return 0;
	}
}


void add_new_client_struct(client_t *newclient){
client_t *ptr;
	
	pthread_mutex_lock(&SERVICE_DATA.client_list_mutex);
	if(SERVICE_DATA.clienthead != NULL){
		for(ptr=SERVICE_DATA.clienthead; ptr->next_client; ptr=ptr->next_client);
		ptr->next_client = newclient;
	}else{
		SERVICE_DATA.clienthead = newclient;
	}
	pthread_mutex_unlock(&SERVICE_DATA.client_list_mutex);
}


void del_client_struct(client_t *client){
	client_t *ptr,*prevptr;
	
	if(SERVICE_DATA.clienthead != NULL){
		if(client == SERVICE_DATA.clienthead){
			SERVICE_DATA.clienthead = SERVICE_DATA.clienthead->next_client;
		}else{
			pthread_mutex_lock(&SERVICE_DATA.client_list_mutex);
			
			for(ptr=SERVICE_DATA.clienthead; ptr->next_client; ptr=ptr->next_client){
				if(ptr == client){
					dec_client_rep_resource_counter(ptr);
					prevptr->next_client = ptr->next_client;
					pthread_mutex_unlock(&SERVICE_DATA.client_list_mutex);
					return;
				}
				prevptr = ptr;
			}
			
			pthread_mutex_unlock(&SERVICE_DATA.client_list_mutex);
		}
	}
}
