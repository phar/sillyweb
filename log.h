#ifndef _LOG_H
#define _LOG_H
#include "server.h"
#include "client.h"

#define LLOG_LOG_LEVEL_LOG		1
#define LLOG_LOG_LEVEL_DEBUG	2
#define MAX_LOG_HEARTBEAT_SECS	15

static int SERVICE_LOG_LEVEL = LLOG_LOG_LEVEL_DEBUG;


void llog(client_t * client,int loglevel, char * logmsg, ...);

#endif
