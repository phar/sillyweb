#ifndef _BUFFER_H
#define _BUFFER_H
#include "common.h"

typedef struct buffer_t{
	char		* buffer;
	unsigned int len;
	int			fd;
}buffer_t;

void clear_buffer(buffer_t * buffer);
int append_buffer(buffer_t * buffer, char * appenddata, size_t appenddatalen);
void free_buffer(buffer_t * buffer);
void ltrim_buffer(buffer_t * buffer, size_t trimlen);
buffer_t * new_buffer();

#endif
