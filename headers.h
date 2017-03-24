#ifndef _HEADERS_H
#define _HEADERS_H

#include "common.h"

typedef struct header_t{
	char * header_name;
	char * header_value;
	struct header_t * next_header;
}header_t;

void new_headers(header_t ** header);
void add_header(header_t **head, char * name, char * value);
void del_header(header_t *head, char * name);
void del_headers(header_t **head);
char * get_header_value(header_t **head,char * defaultval, char * name);
void update_header_value(header_t **head, char * name, char * value);

#endif
