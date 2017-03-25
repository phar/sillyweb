#include "log.h"
#include "util.h"
#include "vhost.h"


static time_t LAST_LOG_EVENT_TIME = 0;



void llog_heartbeat(){
	if(LAST_LOG_EVENT_TIME + MAX_LOG_HEARTBEAT_SECS < time(NULL))
		llog((client_t *)NULL,LLOG_LOG_LEVEL_DEBUG,"heartbeat!");
}

void llog( client_t * client, int loglevel, char * logmsg, ...){
va_list argp;
int i;
char buffer1[512];
char buffer2[512];
	int l;
	
	va_start(argp, logmsg);
	i = vsnprintf(buffer1, sizeof(buffer1)-1, logmsg, argp);
	if(client){
		sprintf(buffer2, "%s - %s [%s] - %s\n", client->ipaddrstr, client->vhost->hostname, sctime(NULL), filter_nonprintables(&buffer1,sizeof(buffer1),'.'));
	}else{
		sprintf(buffer2, "SERVICE - [%s] - %s\n", sctime(NULL), filter_nonprintables(&buffer1,sizeof(buffer1),'.'));
	}

	if(client){
		if(client->vhost->logfilename){
			if(client->vhost->logfilefd <= 0)
				client->vhost->logfilefd = open(client->vhost->logfilename,O_APPEND|O_CREAT);
			if(client->vhost->logfilefd)
				write(client->vhost->logfilefd,buffer2,l);
		}
	}
	
	LAST_LOG_EVENT_TIME = time(NULL);
	printf("%s", buffer2);
	va_end(argp);
}
