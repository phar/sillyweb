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
	
	if((client->pump_fd = open(client->cleanlocalpath,O_RDWR))> 0){
		add_header(&client->outbound_headers, "Transfer-Encoding","plain");
		sz = lseek(client->pump_fd, 0L, SEEK_END);
		lseek(client->pump_fd,0L, SEEK_SET);
		snprintf(lenbuff,sizeof(lenbuff), "%d",sz);
		add_header(&client->outbound_headers, "Content-Length",lenbuff);
		client->transfer_in_progress = 1;
		client->transfer_completed = 0;
	}else{
		client->pump_fd = 0;
		client->transfer_in_progress = 0;
		client->transfer_completed = 0;
	}
	
	return client->pump_fd;
}



ssize_t plain_transfer_send(struct client_t * client){
int l,s;
char pump_buff[NETWORK_BLOCK_SIZE];

	if((client->pump_fd >0) & client->transfer_in_progress){
		if((s = NETWORK_BLOCK_SIZE -  client->outputbuffer->len)){
			if((l =  read(client->pump_fd,&pump_buff,s))){
				append_buffer(client->outputbuffer, pump_buff, l);
			}else{
				client->transfer_completed = 1;
			}
		}
	}

	if(client->outputbuffer->len){
		if(client->outputbuffer->len >= NETWORK_BLOCK_SIZE){
			s = NETWORK_BLOCK_SIZE;
		}else{
			s = client->outputbuffer->len;
		}
		
		if((l =  client->server->transport->send(client,client->outputbuffer->buffer,s))){
			ltrim_buffer(client->outputbuffer, l);
		}else{
			client->transfer_completed = 1;
		}
	}
	return l;
}


ssize_t plain_transfer_recv(struct client_t *client){
int s,l;
char buffer[NETWORK_BLOCK_SIZE];

	if((l = client->server->transport->recv(client,&buffer, NETWORK_BLOCK_SIZE))){
		append_buffer(client->inputbuffer, (char *)&buffer, l);
	}else{
		client->transfer_completed = 1;
	}
	
	if((client->pump_fd>0) & client->transfer_in_progress){
		if(client->inputbuffer->len < NETWORK_BLOCK_SIZE){
			s = client->inputbuffer->len;
		}else{
			s = NETWORK_BLOCK_SIZE;
		}
		if((l =  write(client->pump_fd,client->inputbuffer,s))){
			ltrim_buffer(client->outputbuffer, l);
		}else{
			client->transfer_completed = 1;
		}
	}
	return l;
	
}

int plain_transfer_close(struct client_t *client){
int ret;
	
	if(client->pump_fd){
		ret = close(client->pump_fd);
	}else{
		ret = 0;
	}
	client->pump_fd = 0;
	client->transfer_in_progress = 0;
	client->transfer_completed = 0;

	return 0;

}



int	chunked_transfer_open(struct client_t *client,const char *desc){
	
	add_header(&client->outbound_headers, "Transfer-Encoding","chunked");
	
	if((client->pump_fd = open(client->cleanlocalpath,'r'))){
		client->pump_fd = 1;
		client->transfer_in_progress = 1;

	}
	
	return client->pump_fd;

}

ssize_t chunked_transfer_send(struct client_t * client){
int l;
short s;
char buffer[NETWORK_BLOCK_SIZE];

	
	if(client->outputbuffer->len >= NETWORK_BLOCK_SIZE){
		s = NETWORK_BLOCK_SIZE;
	}else{
		s = client->outputbuffer->len;
	}


	//fixme
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
	
	if(client->pump_fd){
		ret = close(client->pump_fd);
	}else{
		ret = 0;
	}
	client->pump_fd = 0;
	client->transfer_in_progress = 1;
	return 0;
}



int	gzip_transfer_open(struct client_t *client,const char *desc){
	
	add_header(&client->outbound_headers, "Transfer-Encoding","gzip");
	
	if((client->pump_fd = open(client->cleanlocalpath,'r'))){
		client->transfer_in_progress = 1;
	}
	
	return client->pump_fd;

}

ssize_t gzip_transfer_send(struct client_t *client){
	return 0;

}

ssize_t gzip_transfer_recv (struct client_t *client){
	return 0;

}

int gzip_transfer_close(struct client_t *client){
int ret;
	
	if(client->pump_fd){
		ret = close(client->pump_fd);
	}else{
		ret = 0;
	}
	client->pump_fd = 0;
	client->transfer_in_progress = 1;
	return 0;
}



