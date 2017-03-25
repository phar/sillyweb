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
		sprintf(buffer2, "[%s : >SERVICE<] - %s\n", sctime(NULL), filter_nonprintables(&buffer1,sizeof(buffer1),'.'));
	}

	if(client){
		if(client->vhost->logfilename){
			if(client->vhost->logfilefd <= 0)
				client->vhost->logfilefd = open(client->vhost->logfilename,O_APPEND|O_CREAT);
			if(client->vhost->logfilefd)
				write(client->vhost->logfilefd,buffer2,l);
		}
	}
	printf("%s", buffer2);
	va_end(argp);
}
