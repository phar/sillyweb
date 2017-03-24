#include "config.h"
#include <sys/types.h>
#include <pwd.h>
//#include <uuid/uuid.h>
#include <grp.h>

int config_load_callbackable_config(char * cfile, struct config_callback callbacks[], void * context){
char c;
char varname[MAX_VAR_NAME_LENGTH];
char varval[MAX_VAR_VALUE_LENGTH];
int l,i,sm,e,f;
	
	if((f = open(cfile,O_RDONLY))){
		for(i=0,sm=0,l=1;l;){
			if((l = read(f,&c,1))>0){
				switch(sm){
					case 0:
						if(c == '='){
							sm = 1;
							i = 0;
						}else if(i < (MAX_VAR_NAME_LENGTH-1)){
							varname[i++] = c;
							varname[i] = 0;
						}else{
							return 0;
						}
						break;
						
					case 1:
						if(c == '\n'){
							for(e=0;callbacks[e].varname;e++){
								if(!strcmp(varname,callbacks[e].varname)){
									if(callbacks[e].callback(varval, context, callbacks[e].varctx) == 0){
										return 0;
									}
									break;
								}
							}
							i = 0;
							sm = 0;
						}else if(c == '\r'){
							//do nothing
						}else if(i < (MAX_VAR_VALUE_LENGTH-1)){
							varval[i++] = c;
							varval[i] = 0;
						}else{
							return 0;
						}
						break;
				}
			}else
				break;
		}
		return 1;
	}
	return 0;
}


int config_set_uint_handler(char * arg, void * loadctx, void * varctx){
char * endptr;
int ret;
	
	if (varctx){
		*(unsigned int*)varctx = strtoul(arg,&endptr,10);
		if(errno == EINVAL)
			ret = 0;
		else
			ret = 1;
	}else{
		ret = 1;
	}
	return ret;
}


int config_set_int_handler(char * arg, void * loadctx,void * varctx){
char * endptr;
int ret;
	
	if (varctx){
		*(int*)varctx = strtol(arg,&endptr,10);
		if(errno == EINVAL)
			ret = 0;
		else
			ret = 1;
	}else{
		ret = 1;
	}
	return ret;
}

int config_set_string_handler(char * arg, void * loadctx,void * varctx){
int ret = 1;
	
	if (varctx){
		*(char**)varctx = strdup(arg);
		if(*(char **)varctx == 0)
			ret = 0;
		else
			ret = 1;
	}else{
		ret = 0;
	}
	return ret;
}

int config_dummy_handler(char * arg, void * loadctx,void * varctx){
	printf("dummy:%s\n",arg);
	return 1;
}


		
int config_set_gid(char * arg, void * loadctx, void * varctx){
struct group * grp;

	if((grp = getgrnam(arg))){
		*(int*)varctx = grp->gr_gid;
	}else{
		*(int*)varctx = 0;
	}
	
	return 1;
}

int config_set_uid(char * arg, void * loadctx, void * varctx){
struct passwd *pwd;

	if((pwd = getpwnam(arg))){
		*(int*)varctx = pwd->pw_uid;
	}else{
		*(int*)varctx = 0;
	}
	return 1;
}
