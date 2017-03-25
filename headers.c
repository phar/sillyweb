#include "headers.h"
#include "log.h"

/********************
 header functions
 ********************/
void new_headers(header_t **head){
	header_t *ptr,*newptr;
	
	if((newptr = calloc(1,sizeof(header_t)))){
		*head = newptr;
	}
}

void add_header(header_t **head, char * name, char * value){
header_t *ptr,*newptr;
header_t *inptr;
	
	inptr = *head;
	if((newptr = calloc(1,sizeof(header_t)))){
		newptr->header_name =  strdup(name);
		newptr->header_value = strdup(value);
		
		if(inptr != NULL){
			for(ptr=inptr;ptr->next_header != NULL;ptr=ptr->next_header);
			ptr->next_header = newptr;
		}else{
			*head = newptr;
		}
	}else{
		llog(NULL, LLOG_LOG_LEVEL_DEBUG,"MEMORY ALLOCATION ERROR add_header");
	}
}



char * get_header_value(header_t **head,char * defaultval, char * name){
	header_t *ptr, *inptr = *head;
	
	for(ptr=inptr;ptr;ptr=ptr->next_header){
		if(!strcmp(name,ptr->header_name)){
			return ptr->header_value;
		}
	}
	return defaultval;
}


void update_header_value(header_t **head, char * name, char * value){
	header_t *ptr,*inptr = *head;
	
	for(ptr=inptr;ptr;ptr=ptr->next_header){
		if(!strcmp(name,ptr->header_name)){
			ptr->header_value = realloc(ptr->header_value,strlen(value)+1);
			strcpy(ptr->header_value, value);
		}
	}
}


void del_headers(header_t **head){
	header_t *ptr,*nextptr;
	header_t *inptr;
	
	inptr = *head;
	for(ptr=inptr;ptr;ptr=nextptr){
		nextptr = ptr->next_header;
		free(ptr->header_name);
		free(ptr->header_value);
		free(ptr);
		ptr = 0;
	}
	*head = 0;
}
