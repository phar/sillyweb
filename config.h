

#ifndef _CONFIG_H
#define _CONFIG_H
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_VAR_NAME_LENGTH 64
#define MAX_VAR_VALUE_LENGTH 512

struct config_callback{
	char * varname;
	int (*callback)(char * varval, void * loadctx,void * varctx);
	void * varctx;
};

int config_set_uint_handler(char * arg, void * loadctx,void * varctx);
int config_set_int_handler(char * arg, void * loadctx,void * varctx);
int config_set_string_handler(char * arg, void * loadctx,void * varctx);
int config_dummy_handler(char * arg, void * loadctx,void * varctx);
int config_load_callbackable_config(char * cfile, struct config_callback callbacks[], void * context);
int config_set_uid(char * arg, void * loadctx, void * varctx);
int config_set_gid(char * arg, void * loadctx, void * varctx);

#endif
