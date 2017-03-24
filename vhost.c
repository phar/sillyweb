#include "vhost.h"

void add_new_vhost_struct( vhost_t *newvhost){
 vhost_t *ptr;
	
	if(SERVICE_DATA.vhost_head != NULL){
		for(ptr=SERVICE_DATA.vhost_head; ptr->next_vhost; ptr=ptr->next_vhost);
		ptr->next_vhost = newvhost;
	}else{
		SERVICE_DATA.vhost_head = newvhost;
	}
}

vhost_t * get_default_vhost(){
	return SERVICE_DATA.vhost_head;
}


vhost_t *  get_vhost(char * vhost){
	
	
}
