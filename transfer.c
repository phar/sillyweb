#include "common.h"
#include "client.h"
#include "transfer.h"
#include "buffer.h"
#include "server.h"
#include "headers.h"


transfer_t SERVER_TRANSFER_METHODS[] = {
	{"plain",plain_transfer_open,plain_transfer_send,plain_transfer_recv,plain_transfer_close},
	{"chunked",chunked_transfer_open,chunked_transfer_send,gzip_transfer_recv,gzip_transfer_close},
	{"gzip",gzip_transfer_open,gzip_transfer_send,gzip_transfer_recv,gzip_transfer_close},
};

int	plain_transfer_open(struct client_t *client,const char *desc){
	long sz;
	char lenbuff[30];
	
	add_header(&client->outbound_headers, "Transfer-Encoding","plain");
	
	if((client->pump_fd = open(client->uriclean,'r'))){
		sz = lseek(client->pump_fd, 0L, SEEK_END);
		lseek(client->pump_fd,0L, SEEK_SET);
		sprintf(lenbuff, "%d",sz);
		add_header(&client->outbound_headers, "Content-Length",lenbuff);
		client->has_pump_fd = 1;
		client->transfer_in_progress = 1;
		client->transfer_complete = 0;

	}
	
	return client->pump_fd;
}



ssize_t plain_transfer_send(struct client_t * client,  void *buffer, size_t length){
int s,l;
	
	
	if(length){
		append_buffer(client->outputbuffer, (char *)&buffer, length);
	}else{
		
		plain_transfer_close(client);
	}
	return l;
}


ssize_t plain_transfer_recv(struct client_t *client){
int s,l;
char buffer[NETWORK_BLOCK_SIZE];

	l = client->server->transport->recv(client,&buffer, NETWORK_BLOCK_SIZE);
	if(l){
		append_buffer(client->inputbuffer, (char *)&buffer, l);
	}else{
		
		plain_transfer_close(client);
	}
	return l;
	
}

int plain_transfer_close(struct client_t *client){
int ret;
	
	if(client->has_pump_fd){
		ret = close(client->pump_fd);
	}else{
		ret = 0;
	}
	client->has_pump_fd = 0;
	client->transfer_in_progress = 1;
	client->transfer_complete = 1;
	return 0;

}



int	chunked_transfer_open(struct client_t *client,const char *desc){
	
	add_header(&client->outbound_headers, "Transfer-Encoding","chunked");
	
	if((client->pump_fd = open(client->uriclean,'r'))){
		client->has_pump_fd = 1;
		client->transfer_in_progress = 1;
		client->transfer_complete = 0;

	}
	
	return client->pump_fd;

}

ssize_t chunked_transfer_send(struct client_t * client,  void *buffer, size_t length){
int l;
short s;
	//FIXME
	if(s){
		l = sprintf(buffer,"%04x\r\n",s);
		if(client->transfer->send(client, buffer, l) <= 0){
			chunked_transfer_close(client);
		}
		
		if(client->transfer->send(client,client->outputbuffer->buffer, length) > 0){
			ltrim_buffer(client->outputbuffer, length);
		}else{
			chunked_transfer_close(client);
		}
		
		if(client->transfer->send(client,"\r\n", 2) <= 0){
			chunked_transfer_close(client);
		}
	}
	return l;
}


ssize_t chunked_transfer_recv(struct client_t *client){
int s,l;
char buffer[NETWORK_BLOCK_SIZE];
	
	l = client->server->transport->recv(client, &buffer, NETWORK_BLOCK_SIZE);
	if(l > 0){
		append_buffer(client->inputbuffer, (char *)&buffer, l);
	}else{
		chunked_transfer_close(client);
	}
	return l;
}


int chunked_transfer_close(struct client_t *client){
int ret;
	
	if(client->has_pump_fd){
		ret = close(client->pump_fd);
	}else{
		ret = 0;
	}
	client->has_pump_fd = 0;
	client->transfer_in_progress = 1;
	client->transfer_complete = 1;
	return 0;
}



int	gzip_transfer_open(struct client_t *client,const char *desc){
	
	add_header(&client->outbound_headers, "Transfer-Encoding","gzip");
	
	if((client->pump_fd = open(client->uriclean,'r'))){
		client->has_pump_fd = 1;
		client->transfer_in_progress = 1;
		client->transfer_complete = 0;
	}
	
	return client->pump_fd;

}

ssize_t gzip_transfer_send(struct client_t *client,  void *buffer, size_t length){
	return 0;

}

ssize_t gzip_transfer_recv (struct client_t *client){
	return 0;

}

int gzip_transfer_close(struct client_t *client){
int ret;
	
	if(client->has_pump_fd){
		ret = close(client->pump_fd);
	}else{
		ret = 0;
	}
	client->has_pump_fd = 0;
	client->transfer_in_progress = 1;
	client->transfer_complete = 1;
	return 0;
}



