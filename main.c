#include "common.h"
#include "buffer.h"
#include "client.h"
#include "log.h"
#include "server.h"
#include "service.h"
#include "mime.h"
#include "config.h"
#include "headers.h"
#include "util.h"
#include "http.h"
#include "percent.h"

#define MAX_OUTPUT_BUFFER_PUMP_LENGTH 4096


#define SERVER_TYPE_LOCAL	1
#define SERVER_TYPE_HTTP	0
#define SERVER_TYPE_HTTPS	2

#define DIR_SEP_CHR "/"


int null_request_handler(client_t *client);
int get_request_handler(client_t *client);
int is_client_request_complete(buffer_t *buffer);

int dispatch_request(client_t *client);
int parse_request(client_t *client);
int service_clients(int pool_id);
void close_connect(client_t *client);

void client_send_response_code(client_t *client);
void client_send_headers(client_t *client);
int config_load_config(char * config);
void server_listening_thread(void * something);
int config_add_vhost(char * vhostfile, void * loadctx,void * varctx);

void cleanup_dead_clients();
int pump_write_client_fd(client_t *client);
int pump_read_client_fd(client_t *client);
void service_clients_thread(void * thread_pool_id);
int pump_client_fd(client_t *client);
int config_add_server_port(char * portfile, void * loadctx,void * varctx);

int pump_fd_to_buffer(int fd, buffer_t * buffer);
int pump_buffer_to_client(client_t * client);
void generate_dir_listing(client_t * client,char * dir);


struct HTTP_Method METHOD_HANDLERS[] = {
	{"GET",get_request_handler},
	{"PUT",null_request_handler},
	{"POST",null_request_handler},
	{"PUT",null_request_handler},
	{"DELETE",null_request_handler},
	{"TRACE",null_request_handler},
	{"CONNECT",null_request_handler},
	{"HEAD",null_request_handler},
	{"OPTIONS",null_request_handler},
	{NULL,NULL}
};


void cleanup_openssl(){
	EVP_cleanup();
}


int bind_server(struct server_t * ctx){
int sockfd;
int clientfd;

	if (pthread_mutex_init(&ctx->client_list_mutex, NULL) != 0){
		return -1;
	}
	
	ctx->server_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (ctx->server_sock  < 0 ){
		return -2;
	}
	
	if (setsockopt(ctx->server_sock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
		return -5;
	
	bzero(&ctx->addr, sizeof(ctx->addr));
	
	if(inet_pton(AF_INET, ctx->bind_address, &ctx->addr.sin_addr.s_addr)){
		ctx->addr.sin_family = AF_INET;
		
	}else if(inet_pton(AF_INET6, ctx->bind_address, &ctx->addr.sin_addr.s_addr)){
		ctx->addr.sin_family = AF_INET6;
		return -5; //unrecoverable
		
	}
	
	ctx->addr.sin_port = htons((short)ctx->port);
	
	if ( bind(ctx->server_sock, (struct sockaddr*)&ctx->addr, sizeof(ctx->addr)) != 0 ){
		return -3;
	}
	
	if ( listen(ctx->server_sock, 20) != 0 ){
		return -4;
	}

	return ctx->server_sock;
}


int do_server_port_bindings(){
struct server_t *ptr;
	
	for(ptr=SERVICE_DATA.serverhead; ptr; ptr=ptr->next_server){
		if(bind_server(ptr)<0){
			llog((client_t *)ptr, LLOG_LOG_LEVEL_DEBUG,"server port bind failed");
			return -1;
		}
	}
	return 0;
}



int main(int argc, char *argv[]){
int pv;
int i;
int clientfd;
pthread_t listening_thread, client_service_threads[NUM_CPUS];
pthread_attr_t attr;
int s;
	
	memset(&SERVICE_DATA,0,sizeof(SERVICE_DATA));
	pthread_mutex_init(&SERVICE_DATA.client_list_mutex, NULL);
	pthread_mutex_init(&SERVICE_DATA.server_list_mutex, NULL);
	
	
	SSL_load_error_strings();
	ERR_load_crypto_strings();
	
	OpenSSL_add_all_algorithms();
	SSL_library_init();
	
	s = pthread_attr_init(&attr);

	if(!config_load_config("config/server.cfg")){
		llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"server configuration is invalid, bailing");
		exit(-1);
	}
		
	if(do_server_port_bindings() < 0){
		llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"server was unable to bind all required ports");
		exit(-1);
	}
	
	if (getuid() == 0) {
		if (setgid(SERVICE_DATA.servicesetgid) != 0){
			llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"setgid: Unable to drop group privileges: %s", strerror(errno));
			exit(-3);
		}
		if (setuid(SERVICE_DATA.servicesetuid) != 0){
			llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"setuid: Unable to drop user privileges: %s", strerror(errno));
			exit(-3);
		}
	}
	
	pthread_create(&listening_thread, &attr,(void *) &server_listening_thread, NULL);

	for(i=0;i<NUM_CPUS;i++){
		pthread_create(&client_service_threads[i], &attr,(void *) &service_clients_thread, (void *)i);
	}

	SERVICE_DATA.active = 1;
	
	while (SERVICE_DATA.active){
		sleep(5);
		llog_heartbeat();
	}

	for(i=0;i<NUM_CPUS;i++){
		pthread_join(&client_service_threads[i],NULL);
	}
	
	return 0;
}


void service_clients_thread(void * thread_pool_id){
	while(1){
		service_clients((int)thread_pool_id);
	}
}

void server_listening_thread(void * something){
struct pollfd *ufds;//[20];//fixme
int pv,i,e;
int clientfd;
struct sockaddr_in client_addr;
int addrlen,cnt;
struct server_t *ptr;

	llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"listening thread started");
	while (1){
		cnt = get_server_count();
		ufds = calloc(cnt, sizeof(struct pollfd));
		
		for(ptr=SERVICE_DATA.serverhead,i=0;ptr;ptr=ptr->next_server,i++){
			ufds[i].fd = ptr->server_sock;
			ufds[i].events = POLLIN | POLLPRI;
		}
		
		if((pv = poll(ufds,cnt, 1000))){
			for(e=0;e<cnt;e++){
				if(ufds[e].revents == POLLIN){
					if((ptr = get_server_by_fd(ufds[e].fd))){
						addrlen=sizeof(client_addr);
						if((clientfd = accept(ptr->server_sock, (struct sockaddr*)&client_addr, (socklen_t *)&addrlen)) != -1){
							if(!create_new_client(clientfd,&client_addr,ptr)){
								llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"error allocating new client");
							}else{
								llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"accept");
							}
							break;
						}
					}else{
						llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"this should never happen, server fd not found in server list");
					}
				}
			}
		}else{
			cleanup_dead_clients();
		}
		free(ufds);
	}
}




static int sni_callback_handler(SSL *ssl, int *ad, void *arg){
	
	if (ssl == NULL)
		return SSL_TLSEXT_ERR_NOACK;
	
	const char* servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
	
	printf("ServerName: %s\n", servername);
}





int post_request_handler(client_t *client){
	return 0;
}


int get_request_handler(client_t *client){
	
	return 1;
}



int null_request_handler(client_t *client){
	
	return 1;
}




int dispatch_request(client_t *client){
char *tmppath;
char *abspath;
char * subdir;
char **argp;
int i;
int forkfd[2];
int pathlen;
char buff[1024];
int ret;
	
	
	llog((client_t *)client,LLOG_LOG_LEVEL_DEBUG,"dispatch");
	
	if(client->result_code == HTTP_RESULT_OK){
		if((strstr(client->cleanlocalpath,client->vhost->document_root) != NULL) & (!client->is_cgi_request)){ //its a file, not a cgi
			if(is_file(client->cleanlocalpath)){
				if(client->transfer->open(client, client->cleanlocalpath)){
					add_header(&client->outbound_headers, "Content-Type", get_mime_from_filename(client->filename));
				
				}else{
					client->result_code = HTTP_RESULT_INTERNAL_SERVER_ERROR;
				}
			}else{
				if(is_dir(client->cleanlocalpath)){
					client->result_code = HTTP_RESULT_OK;
					generate_dir_listing(client,client->cleanlocalpath);
				}else{
					client->result_code = HTTP_RESULT_NOT_FOUND;
				}
			}
		}else if(client->is_cgi_request){//is cgi?
			llog((client_t *)client,LLOG_LOG_LEVEL_DEBUG,"i am here3");
			
			argp = malloc(128 * sizeof(char *)); //FIXME
			
	//		DOCUMENT_ROOT	The root directory of your server
			snprintf(buff,sizeof(buff),"DOCUMENT_ROOT=%s",client->vhost->document_root);
			argp[i++] = strdup(buff);
			
	//		HTTP_COOKIE	The visitor's cookie, if one is set
			snprintf(buff,sizeof(buff),"HTTP_COOKIE=%1023s",get_header_value(&client->inbound_headers,"None","Cookie"));
			argp[i++] = strdup(buff);
			
	//		HTTP_HOST	The hostname of the page being attempted
			snprintf(buff,sizeof(buff),"HTTP_HOST=%s",client->vhost->hostname);
			argp[i++] = strdup(buff);
			
	//		HTTP_REFERER	The URL of the page that called your program
			snprintf(buff,sizeof(buff),"HTTP_REFERER=%s",get_header_value(&client->inbound_headers,"None","Referer"));
			argp[i++] = strdup(buff);
			
	//		HTTP_USER_AGENT	The browser type of the visitor
			snprintf(buff,sizeof(buff),"HTTP_USER_AGENT=%s",get_header_value(&client->inbound_headers,"None","User-Agent"));
			argp[i++] = strdup(buff);
			
	//		HTTPS	"on" if the program is being called through a secure server
			if(client->server->SSL)
				snprintf(buff,sizeof(buff),"HTTPS=%s","on");
			else
				snprintf(buff,sizeof(buff),"HTTPS=%s","off");
			argp[i++] = strdup(buff);
			
	//		PATH	The system path your server is running under
			snprintf(buff,sizeof(buff),"PATH=%s",client->vhost->document_root);
			argp[i++] = strdup(buff);
			
	//		QUERY_STRING	The query string (see GET, below)
			snprintf(buff,sizeof(buff),"QUERY_STRING=%s",client->query);
			argp[i++] = strdup(buff);
			
	//		REMOTE_ADDR	The IP address of the visitor
			snprintf(buff,sizeof(buff),"REMOTE_ADDR=%s",client->ipaddrstr);
			argp[i++] = strdup(buff);
			
	//		REMOTE_HOST	The hostname of the visitor (if your server has reverse-name-lookups on; otherwise this is the IP address again)
	//		snprintf(buff,sizeof(buff),"REMOTE_HOST=%s",);
	//		argp[i++] = strdup(buff);
			
	//		REMOTE_PORT	The port the visitor is connected to on the web server
			snprintf(buff,sizeof(buff),"REMOTE_PORT=%d",client->srcport);
			argp[i++] = strdup(buff);
			
	//		REMOTE_USER	The visitor's username (for .htaccess-protected pages)
			snprintf(buff,sizeof(buff),"REMOTE_USER=%s","nobody");
			argp[i++] = strdup(buff);
			
	//		REQUEST_METHOD	GET or POST
	//		snprintf(buff,sizeof(buff),"REQUEST_METHOD=%s",);
	//		argp[i++] = strdup(buff);
			
	//		REQUEST_URI	The interpreted pathname of the requested document or CGI (relative to the document root)
	//		snprintf(buff,sizeof(buff),"REQUEST_URI=%s",client->uriclean);//no
	//		argp[i++] = strdup(buff);
			
	//		SCRIPT_FILENAME	The full pathname of the current CGI
			snprintf(buff,sizeof(buff),"SCRIPT_FILENAME=%s",client->filename);
			argp[i++] = strdup(buff);
			
	//		SCRIPT_NAME	The interpreted pathname of the current CGI (relative to the document root)
			snprintf(buff,sizeof(buff),"SCRIPT_NAME=%s",client->filename);
			argp[i++] = strdup(buff);
			
	//		SERVER_ADMIN	The email address for your server's webmaster
			snprintf(buff,sizeof(buff),"SERVER_ADMIN=%s",client->vhost->admin_email);
			argp[i++] = strdup(buff);
			
	//		SERVER_NAME	Your server's fully qualified domain name (e.g. www.cgi101.com)
			snprintf(buff,sizeof(buff),"SERVER_NAME=%s",client->vhost->hostname);
			argp[i++] = strdup(buff);
			
	//		SERVER_PORT	The port number your server is listening on
			snprintf(buff,sizeof(buff),"SERVER_PORT=%d",ntohs(client->server->addr.sin_port));
			argp[i++] = strdup(buff);
			
	//		SERVER_SOFTWARE	The server software you're using (e.g. Apache 1.3)
			snprintf(buff,sizeof(buff),"SERVER_SOFTWARE=%s",SERVER_VERSION_STRING);
			argp[i++] = strdup(buff);
			argp[i] = 0;
			
			free(buff);
			
			pipe(forkfd);
			
			client->pump_fd = forkfd[1];
			
			if(!fork()){
				//drop privs again
				//fixme stdin, and stdout, stdin needs to pump data
				dup2(forkfd[0], STDOUT_FILENO);
				close(forkfd[1]);
				close(forkfd[0]);
				execle(client->cleanlocalpath,client->cleanlocalpath,client->filename,(char*) NULL,argp);
			}
		}else { //forbidden
			client->result_code = HTTP_RESULT_FORBIDDEN;
		}
		
		ret = client->method->handler(client);
		client_send_response_code(client);
		client_send_headers(client);
		
		llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"%s [%s] %d - %s", client->method->method, client->uri,client->result_code, client->cleanlocalpath);

		
		return ret;
	}else{
		add_header(&client->outbound_headers, "Content-type", "text/html");
		client_send_response_code(client);
		client_send_headers(client);
		append_buffer(client->outputbuffer,"<html>",6);
		
		if(client->flags & CONNECTION_FLAG_HTTP_1_1){
			clear_client(client);
		}else if(client->flags & CONNECTION_FLAG_HTTP_1_0){
			client->active = 0;
			client->server->transport->close(client);
		}
		
		llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"%s [%s] %d", client->method->method, client->uri, client->result_code);
		return 1;
	}
}



void client_send_headers(client_t *client){
char *buff;
header_t *ptr;
char *headerfmt = "%s: %s\r\n";
int l,hl;

	if(client->flags & CONNECTION_FLAG_CLOSE){
		add_header(&client->outbound_headers, "Connection","Close");
	}else{
		add_header(&client->outbound_headers, "Connection","Keep-Alive");
	}

	for(ptr=client->outbound_headers;ptr;ptr=ptr->next_header){
		hl = strlen(ptr->header_name) + strlen(ptr->header_value) +  strlen(headerfmt) + 1;
		if((buff = malloc(hl))){
			l = snprintf(buff,hl,headerfmt,  ptr->header_name,  ptr->header_value);
			client->server->transport->send(client,buff, l);
			free(buff);
		}
	}
	client->server->transport->send(client,"\r\n", 2);
}



void client_send_response_code(client_t *client){
char buff[256];
int l, i;
char * resptext = HTTP_ERRORS[0].result_string;

	if(client->flags & CONNECTION_FLAG_HTTP_1_0){
		l = snprintf(buff,sizeof(buff), "HTTP/%s ",HTTP_VERSION_1_0);
		client->server->transport->send(client,buff, l);
		
	}else if(client->flags & CONNECTION_FLAG_HTTP_1_1){
		l = snprintf(buff,sizeof(buff), "HTTP/%s ",HTTP_VERSION_1_1);
		client->server->transport->send(client,buff, l);
	}
	
	l = snprintf(buff,sizeof(buff), "%d ", client->result_code);
	client->server->transport->send(client,buff, l);
	
	l = snprintf(buff,sizeof(buff), "%s", get_http_result_text(client->result_code));
	client->server->transport->send(client, buff,l);
	
	l = snprintf(buff,sizeof(buff), "\r\n");
	client->server->transport->send(client,buff, l);
	
}



int parse_request(client_t *client){
char method[20+1],uri[1024+1],httpsp[20+1];
int l,i;
char * linesep;
char * headertoken,*headertokenstr = client->inputbuffer->buffer;
char * valuetoken,*nametoken,*valuetokenstr,*queryptr;
char * vhoststr,*keepalive;
vhost_t *vhptr;
char buffer[1024];
	
	add_header(&client->outbound_headers, "Server", SERVER_VERSION_STRING);
	add_header(&client->outbound_headers, "Date", sctime(NULL));

	if(client->flags & CONNECTION_FLAG_LINE_ENDING_USES_CR){
		linesep = "\r\n";
		headertoken = strsep(&headertokenstr, "\n");
		l = sscanf(headertoken,"%20s %1024s %20s", method, uri, httpsp);
	}else{
		linesep = "\n";
		headertoken = strsep(&headertokenstr, "\n");
		l = sscanf(headertoken,"%20s %1024s %20s", method, uri, httpsp);
	}

	if(l == 3){
		method[sizeof(method)-1] = 0;
		uri[sizeof(uri)-1] = 0;
		httpsp[sizeof(httpsp)-1] = 0;
		
		//default
		client->flags |= CONNECTION_FLAG_CLOSE;
		client->flags |= CONNECTION_FLAG_HTTP_1_0;
		
		if(!strncasecmp(httpsp, "HTTP/",5)){
			// setup basic http versions
			if(!strncmp(httpsp+5,"1.0",3)){
				client->flags ^= (CONNECTION_FLAG_HTTP_1_1);
				client->flags |= (CONNECTION_FLAG_HTTP_1_0 | CONNECTION_FLAG_CLOSE);
			}else if(!strncmp(httpsp+5,"1.1",3)){
				client->flags ^= (CONNECTION_FLAG_HTTP_1_0);
				client->flags |= CONNECTION_FLAG_HTTP_1_1;
				
			}else{
				client->result_code = HTTP_RESULT_HTTP_VERSION_NOT_SUPPORTED;
				client->flags |= CONNECTION_FLAG_CLOSE;
				return client->result_code;
			}
	
			
			//determine which HTTP method is in use
			for(i=0;METHOD_HANDLERS[i].method;i++){
				if(!strcasecmp(METHOD_HANDLERS[i].method,method)){
					client->method = &METHOD_HANDLERS[i];
				}
			}

			if(client->method){
				//headers
				do{
					headertoken = strsep(&headertokenstr, "\n");
					valuetokenstr = headertoken;
					nametoken = strsep(&valuetokenstr, ":");
					valuetoken = strsep(&valuetokenstr, ":");
					if(nametoken && valuetoken)
						add_header(&client->inbound_headers, nametoken, valuetoken);
				}while(headertoken != NULL);
				
				//query string
				queryptr = strchr(uri,'?');
				if(queryptr){
					client->query = strdup(queryptr);
					queryptr[0] = 0;
				}else{
					client->query = strdup("");
				}

				//declared uri
				if(!strcmp(uri, "/")){
					client->uri = strdup(DEFAULT_FILENAME);
				}else{
					i =  percent_decode(uri, uri);
					uri[i] = 0;
					client->uri = strdup(uri);
				}
				
				//keep-alive
				if ((client->flags & CONNECTION_FLAG_HTTP_1_1)){
					keepalive = get_header_value(&client->inbound_headers,"Close","Connection");
					if(!strcasecmp(keepalive,"Keep-Alive")){
						client->flags ^= CONNECTION_FLAG_CLOSE;
					}else if(!strcasecmp(keepalive,"Close")){
						client->flags |= CONNECTION_FLAG_CLOSE;
					}
				}
				
				
				//vhost
				vhoststr = get_header_value(&client->inbound_headers,NULL,"Host");
			
				if(vhoststr && (client->flags & CONNECTION_FLAG_HTTP_1_1)){
					if((vhptr = get_vhost(vhoststr))){
						client->vhost = vhptr;
					}else{
						client->result_code = HTTP_RESULT_NOT_FOUND;
					}
				}
				//local path from uri (unsanitized)
				snprintf(buffer,sizeof(buffer), "%s%s%s",client->vhost->document_root,DIR_SEP_CHR,client->uri);
				client->localpath = strdup(buffer);
				
				client->filename = basename(client->localpath);
				
				if((client->cleanlocalpath = realpath(client->localpath,NULL))){
					if(strstr(client->vhost->cgi_bin_root,client->cleanlocalpath) != NULL){
						client->is_cgi_request = 1;
					}
				}else{
					client->result_code = HTTP_RESULT_NOT_FOUND;
				}
				
				if(!client->result_code)
					client->result_code = HTTP_RESULT_OK;
				
				clear_buffer(client->inputbuffer);
				
				client->request_parsed = 1;
			}else{
				client->result_code = HTTP_RESULT_NOT_IMPLEMENTED;
			}
		}else{
			client->result_code = HTTP_RESULT_BAD_REQUEST;
			client->flags |= CONNECTION_FLAG_CLOSE;
		}
	}else{
		client->result_code = HTTP_RESULT_BAD_REQUEST;
		client->flags |= CONNECTION_FLAG_CLOSE;
	}
	
	return client->result_code;
}


void cleanup_dead_clients(){
client_t *ptr;
	
	for(ptr=SERVICE_DATA.clienthead;ptr;ptr=ptr->next_client){
		if(ptr->active == 0){
			delete_client(ptr);
		}
	}
}



int service_clients(int poolid){
int i,c;
client_t *ptr;
int clientcount;
struct pollfd *clientfds;
clientfds = NULL;
int clients_need_service = 0;
	
	/* build a list of clients */
	if(SERVICE_DATA.clienthead != NULL){
		for(ptr=SERVICE_DATA.clienthead,clientcount=0;ptr;ptr=ptr->next_client){
			if(ptr->active){
				if(ptr->transfer_in_progress  & ptr->transfer_completed){
					ptr->transfer->close(ptr);
					if((ptr->flags & CONNECTION_FLAG_HTTP_1_0) | (ptr->flags & CONNECTION_FLAG_CLOSE)){
						ptr->active = 0;
						ptr->server->transport->close(ptr);
					}else if((ptr->flags & CONNECTION_FLAG_HTTP_1_1)){
						clear_client(ptr);
					}
				}else if(poolid == ptr->thread_pool_id){
					// add them to the service list
					clientcount++;
					clientfds = realloc(clientfds,clientcount * sizeof(struct pollfd));
					clientfds[clientcount-1].fd = ptr->sock;
					clientfds[clientcount-1].events = POLLOUT | POLLIN | POLLERR | POLLHUP | POLLNVAL;
					
					if(is_client_request_complete(ptr->inputbuffer)){
						parse_request(ptr);
						dispatch_request(ptr);
					}
				}
			}
		}
	}
	
	if(clientcount){
		if((clients_need_service = poll(clientfds, clientcount, 1000)) > 0){
			for(i=0;i<clientcount;i++){
				for(ptr=SERVICE_DATA.clienthead,clientcount=0;ptr;ptr=ptr->next_client){
					if(ptr->sock == clientfds[i].fd){
						if((clientfds[i].revents  & (POLLERR | POLLHUP |POLLNVAL)) > 0){
							close_connect(ptr);
						}else if((clientfds[i].revents  & (POLLIN))>0){
							ptr->transfer->recv(ptr);

						}else if((clientfds[i].revents & POLLOUT)){
								ptr->transfer->send(ptr);
						}
						break;
					}
				}
			}
		}
	}else{
		sleep(1); //ech.. but keeps us from an tight loop if there are no clients to service
	}
	return clients_need_service;
}



int is_client_request_complete(buffer_t *buffer){

	if(buffer->len > 4){
		if((strstr(buffer->buffer, "\r\n\r\n") != NULL)){
			return 1;
		}else if(strstr(buffer->buffer, "\n\n") != NULL){
			return 1;
		}else{
			return 0;
		}
	}
	return 0;
}



int config_load_config(char * config){
	struct config_callback MAIN_CONFIG[] = {
		{"PORT", config_add_server_port,NULL},
		{"VHOST", config_add_vhost,NULL},
		{"UID", config_set_uid,&SERVICE_DATA.servicesetuid},
		{"GID", config_set_gid,&SERVICE_DATA.servicesetgid},
		{NULL,NULL,NULL}};
	
	llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"parsing main config file \"%s\"",config);
	
	return config_load_callbackable_config(config,MAIN_CONFIG, NULL);
}


int config_add_server_port(char * portfile, void * loadctx, void * varctx){
	struct service_data_header *ctx = loadctx;
	struct server_t *newserver;
	int ret;
	
	if((newserver = calloc(1,sizeof( server_t)))){
		struct config_callback PORT_CONFIG[] = {
			{"BINDADDRESS",config_set_string_handler,&newserver->bind_address},
			{"PORT",config_set_int_handler,&newserver->port},
			{"SSL",config_set_int_handler,&newserver->SSL},
			{NULL,NULL,NULL}};
		
		llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"parsing portfile \"%s\"",portfile);
		if((ret = config_load_callbackable_config(portfile, PORT_CONFIG, newserver))){
			if(newserver->SSL){
				newserver->transport = &SERVER_TRANSPORT_METHODS[1];
			}else{
				newserver->transport = &SERVER_TRANSPORT_METHODS[0];
			}
			
			if(newserver->SSL){
				newserver->sslctx = SSL_CTX_new(SSLv3_server_method());
			}
			add_new_server_struct(newserver);
		}
	}else{
		ret = 0;
	}
	return ret;
}

int config_add_vhost(char * vhostfile, void * loadctx,void * varctx){
	vhost_t  *newvhost;
	int ret;
	char * tpath;
	
	
	if((newvhost = calloc(1,sizeof(vhost_t)))){
		struct config_callback VHOST_CONFIG[] = {
			{"ADMIN", config_set_string_handler,&newvhost->admin_email},
			{"HOSTNAME",config_set_string_handler,&newvhost->hostname},
			{"WEBROOT",config_set_string_handler,&newvhost->document_root},
			{"CGI_ROOT",config_set_string_handler,&newvhost->cgi_bin_root},
			{"LOGFILE",config_set_string_handler,&newvhost->logfilename},
			{"SSL",config_set_int_handler,&newvhost->SSL},
			{"CERT",config_set_string_handler,&newvhost->sslcert},
			{"CERT_KEYFILE",config_set_string_handler,&newvhost->sslkey},
			{NULL,NULL,NULL}};
		
		llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"parsing vhost file \"%s\"",vhostfile);
		
		if((ret = config_load_callbackable_config(vhostfile, VHOST_CONFIG, newvhost))){
			
			if((tpath = realpath(newvhost->document_root,NULL)) == NULL){
				ret = 0;
				llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"document root does not exist!");
			}else{
				free(newvhost->document_root);
				newvhost->document_root = tpath;
			}
			
			if((tpath = realpath(newvhost->cgi_bin_root,NULL)) == NULL){
				ret = 0;
				llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"cgi-bin root does not exist!");
				
			}else{
				free(newvhost->cgi_bin_root);
				newvhost->cgi_bin_root = tpath;
			}
			
			if(newvhost->SSL){
				llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"loading SSL certificate");
				newvhost->sslctx = SSL_CTX_new(SSLv3_server_method());
				
				if (SSL_CTX_use_certificate_file(newvhost->sslctx, newvhost->sslcert, SSL_FILETYPE_PEM) <= 0 ){
					llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG, "error loading certificate file\n");
					ret = 0;
				}
				
				if (SSL_CTX_use_PrivateKey_file(newvhost->sslctx, newvhost->sslkey, SSL_FILETYPE_PEM) <= 0 ){
					llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG, "error loading key file\n");
					ret = 0;
				}
				
				if (!SSL_CTX_check_private_key(newvhost->sslctx)){
					llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG, "Private key does not match the public certificate\n");
					ret = 0;
				}
				
				if(!(SSL_CTX_set_tlsext_servername_callback(newvhost->sslctx, sni_callback_handler))){
					ret = 0;
				}
			}
		}
		
		add_new_vhost_struct(newvhost);
	}else{
		ret = 0;
	}
	return ret;
	
}


void generate_dir_listing(client_t * client,char * dir){
	DIR *dp;
	struct dirent *ep;
	char buffer[1024];
	struct stat path_stat;
	int l;
	char *residualpath;
	
	if((residualpath = strstr(dir, client->vhost->document_root))){
		residualpath += strlen(client->vhost->document_root);
	}else{
		residualpath = dir;
	}
	
	add_header(&client->outbound_headers, "Content-type", "text/html");
	append_buffer(client->outputbuffer,"<html>",6);
	append_buffer(client->outputbuffer,"<body>",6);
	l = snprintf(&buffer,sizeof(buffer),"<H1>Index of %s</h1><br>\n",residualpath);
	append_buffer(client->outputbuffer,buffer,l);
	append_buffer(client->outputbuffer,"<tr colspan=3><hr></tr>\n",23);
	l = snprintf(&buffer,sizeof(buffer),"<table><tr><td>Filename</td><td>Size</td><td>Last Modified</td></tr>\n");
	append_buffer(client->outputbuffer,buffer,l);
	
	dp = opendir (dir);
	if (dp != NULL){
		while ((ep = readdir (dp))){
			
			l = snprintf(&buffer,sizeof(buffer),"%s%s%s",dir, DIR_SEP_CHR, ep->d_name, ep->d_name);
			stat64(buffer, &path_stat);
			
			if(is_file(buffer)){
				l = snprintf(&buffer,sizeof(buffer),"\t<tr><td><a href=\"%s%s%s\">%s</a></td><td>%d</td><td>%s</td></tr>\n",residualpath,DIR_SEP_CHR, ep->d_name, ep->d_name, path_stat.st_size,sctime(path_stat.st_mtime));
			}else{
				l = snprintf(&buffer,sizeof(buffer),"\t<tr><td><a href=\"%s%s%s\">%s</a</td><td></td><td>%s</td></tr>\n",residualpath,DIR_SEP_CHR, ep->d_name, ep->d_name,sctime(path_stat.st_mtime));
				
			}
			append_buffer(client->outputbuffer,buffer,l);
		}
		(void) closedir (dp);
	}else{
		
	}
	
	l = snprintf(buffer,sizeof(buffer),"</table>\n",buffer);
	append_buffer(client->outputbuffer,buffer,l);
	
	l = snprintf(&buffer,sizeof(buffer),"%s Server at %s on port %d<br>\n",SERVER_VERSION_STRING ,client->vhost->hostname, client->server->port);
	append_buffer(client->outputbuffer,buffer,l);
	append_buffer(client->outputbuffer,"</body>",7);
	append_buffer(client->outputbuffer,"</html>",7);
 
	l = snprintf(&buffer,sizeof(buffer),"%d",client->outputbuffer->len);
	add_header(&client->outbound_headers, "Content-Length", buffer);
	
}
