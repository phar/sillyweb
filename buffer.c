
#include "buffer.h"
#include "log.h"

/******************
 basic buffer type and manipulators and encoders
 *******************/

buffer_t * new_buffer(){
	buffer_t * buffer;
	
	if((buffer = calloc(1,sizeof(buffer_t)))){
		buffer->buffer = NULL;
		buffer->len = 0;
	}else{
		llog(NULL,LLOG_LOG_LEVEL_DEBUG,"MEMORY ALLOCATION ERROR new_buffer");
	}
	return buffer;
}

int append_buffer(buffer_t * buffer, char * appenddata, size_t appenddatalen){
	if((buffer->buffer = realloc(buffer->buffer, buffer->len + appenddatalen))){
		memcpy(buffer->buffer + buffer->len, appenddata, appenddatalen);
		buffer->len += appenddatalen;
		return 1;
	}
	return 0;
}


void ltrim_buffer(buffer_t * buffer, size_t trimlen){
	
	if(trimlen <= buffer->len){
		buffer->len -= trimlen;
		memmove(buffer->buffer, buffer->buffer+trimlen, buffer->len);
		buffer->buffer = realloc(buffer->buffer, buffer->len);
	}else{
		clear_buffer(buffer);
	}
	
}

void clear_buffer(buffer_t * buffer){
	
	if (buffer->buffer){
		free(buffer->buffer);
		buffer->buffer = NULL;
	}
	buffer->len = 0;
}

void free_buffer(buffer_t * buffer){
	clear_buffer(buffer);
}

int percent_decode_buffer(buffer_t * inbuffer, buffer_t * outbuffer){
	int ll;
	int i,o,x;
	char t;
	int ret = 0;
	
	for(i=0,outbuffer->len=0;i<inbuffer->len;i++){
		if(inbuffer->buffer[i] == '%'){
			if((inbuffer->len >= i+2) && (sscanf(&inbuffer->buffer[i+1],"%02x", (int *)&t))){
				outbuffer->buffer[outbuffer->len++] = t;
				i+=2;
			}else{//bad character, sink it and move on
				ret += 1;
			}
		}else
			outbuffer->buffer[outbuffer->len++] = inbuffer->buffer[i];
	}
	return 1;
}


int percent_encode_buffer(buffer_t * inbuffer, buffer_t * outbuffer){
	int i;
	char buff[10];
	
	buff[4] = 0;
	for(i=0;i<inbuffer->len;i++){
		sprintf(buff,"%%%02x",inbuffer->buffer[i]);
		strcat(outbuffer->buffer,buff);
	}
	return 0;
}
