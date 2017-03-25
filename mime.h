
#ifndef _MIME_H
#define _MIME_H


struct mime_list{
	char * extension;
	char * mimetype;
};


extern struct mime_list MIME_LIST[];

char * get_mime_from_filename(char * filename);
#endif

