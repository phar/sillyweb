#ifndef _VHOST_H
#define _VHOST_H
#include "common.h"
#include "service.h"

typedef struct vhost_t{
	char * document_root;
	char * cgi_bin_root;
	char * admin_email;
	char * hostname;
	char	* logfilename;
	int		 logfilefd;

	struct  vhost_t *next_vhost;
	int SSL;
	int vhostsetuid;
	int vhostsetgid;
	char	*sslcert;
	char	*sslkey;
	SSL_CTX	*sslctx;
}vhost_t;

void add_new_vhost_struct( vhost_t *newvhost);
vhost_t * get_default_vhost();
vhost_t *  get_vhost(char * vhost);
#endif
