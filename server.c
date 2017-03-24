#include "service.h"
#include "server.h"

void add_new_server_struct(server_t *newserver){
server_t *ptr;

	if(SERVICE_DATA.serverhead != NULL){
		for(ptr=SERVICE_DATA.serverhead; ptr->next_server; ptr=ptr->next_server);

		ptr->next_server = newserver;
	}else{
		SERVICE_DATA.serverhead = newserver;
	}
}



int get_server_count(){
server_t *ptr;
int i;

	for(ptr=SERVICE_DATA.serverhead,i=0;ptr;ptr=ptr->next_server){
		i++;
	}
	return i;
}

server_t *get_server_by_fd(int fd){
server_t *ptr;
int i;
	
	for(ptr=SERVICE_DATA.serverhead,i=0;ptr;ptr=ptr->next_server){
		if(ptr->server_sock == fd)
			return ptr;
	}
	return NULL;
}
