#include "transport.h"
#include "client.h"
#include "service.h"


transport_t SERVER_TRANSPORT_METHODS[] = {
	{"tcp",tcp_transport_open,tcp_transport_send,tcp_transport_recv,tcp_transport_close},
	{"ssl",ssl_transport_open,ssl_transport_send,ssl_transport_recv,ssl_transport_close}
};

int	tcp_transport_open(struct client_t *client,const char *desc){
	return 1;
}

ssize_t tcp_transport_send(struct client_t *client,  void *buffer, size_t length){
	return send(client->sock, buffer, length,0);
}

ssize_t tcp_transport_recv(struct client_t *client, void *buffer, size_t length){
	return  recv(client->sock, buffer, length,0);
}

int tcp_transport_close(struct client_t * client){
	return close(client->sock);
}





int	ssl_transport_open(struct client_t * client,const char *desc){
	client->sslctx = SSL_new(client->service->sslctx);
	SSL_set_fd(client->sslctx, client->sock);
	return 0;
}

ssize_t ssl_transport_send(struct client_t * client,  void *buffer, size_t length){
	return SSL_write(client->sslctx, buffer, length);
}

ssize_t ssl_transport_recv(struct client_t * client, void *buffer, size_t length){
	return SSL_read(client->sslctx, buffer, length);
}

int ssl_transport_close(struct client_t * client){
	SSL_free(client->sslctx);
	return close(client->sock);

}



