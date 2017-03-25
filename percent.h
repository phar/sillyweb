#ifndef _PERCENT_H
#define _PERCENT_H
#include "buffer.h"


int percent_decode(char *in, char*out);
char * percent_encode(char *in);
char * html_encode(char *in);

#endif
