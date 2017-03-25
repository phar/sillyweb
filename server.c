#include "service.h"
#include "server.h"

void add_new_server_struct(server_t *newserver){
server_t *ptr;

	if(SERVICE_DATA.serverhead != NULL){
		pthread_mutex_lock(&SERVICE_DATA.server_list_mutex);
		for(ptr=SERVICE_DATA.serverhead; ptr->next_server; ptr=ptr->next_server);
		ptr->next_server = newserver;
	}else{
		pthread_mutex_lock(&SERVICE_DATA.server_list_mutex);
		SERVICE_DATA.serverhead = newserver;
		pthread_mutex_unlock(&SERVICE_DATA.server_list_mutex);
	}
}



int get_server_count(){
server_t *ptr;
int i;

	pthread_mutex_lock(&SERVICE_DATA.server_list_mutex);
	for(ptr=SERVICE_DATA.serverhead,i=0;ptr;ptr=ptr->next_server){
		i++;
	}
	pthread_mutex_unlock(&SERVICE_DATA.server_list_mutex);
	return i;
}

server_t *get_server_by_fd(int fd){
server_t *ptr;
int i;

	pthread_mutex_lock(&SERVICE_DATA.server_list_mutex);
	for(ptr=SERVICE_DATA.serverhead,i=0;ptr;ptr=ptr->next_server){
		if(ptr->server_sock == fd){
			pthread_mutex_unlock(&SERVICE_DATA.server_list_mutex);
			return ptr;
		}
	}
	pthread_mutex_unlock(&SERVICE_DATA.server_list_mutex);
	return NULL;
}
