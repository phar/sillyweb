#include "log.h"
#include "util.h"
#include "vhost.h"

void llog( client_t * client, int loglevel, char * logmsg, ...){
va_list argp;
int i;
char buffer1[512];
char buffer2[512];
	int l;
	
	va_start(argp, logmsg);
	i = vsnprintf(buffer1, sizeof(buffer1)-1, logmsg, argp);
	if(client){
		sprintf(buffer2, "[%s : %s] - %s\n", sctime(NULL), client->vhost->hostname, filter_nonprintables(&buffer1,sizeof(buffer1),'.'));
	}else{
		sprintf(buffer2, "[%s] - %s\n", sctime(NULL), filter_nonprintables(&buffer1,sizeof(buffer1),'.'));
	}

	if(client){
		if(client->server->logfilename){
			printf("%d\n",client->server->logfilename);
			if(!client->server->logfilefd){
				client->server->logfilefd = open(client->server->logfilename,O_APPEND|O_RDWR);
			}
			write(client->server->logfilefd,buffer2,l);
		}
	}
	printf("%s", buffer2);
	va_end(argp);
}
