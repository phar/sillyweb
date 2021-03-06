#include "common.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>


int percent_decode(char *in, char*out){
int ll;
int i,o,x;
char t;
	ll = strlen(in);
	for(i=0,o=0;i<ll;i++){
		if(in[i] == '%'){
			if((ll >= i+2) && (sscanf(&in[i+1],"%02x",&t))){
				out[o++] = t;
				i+=2;
			}//bad character, sink it and move on	
		}else{
			out[o++] = in[i];
		}
	}
	return o;
}


char * percent_encode(char *in){
int ll,i;
char cbuff[10];
char * outbuf;

	ll = strlen(in);
	if((outbuf = malloc(ll * 3))){
		outbuf[0] = 0;
		for(i=0;i<ll;i++){
			sprintf(cbuff, "%%%02x",in[i]);
			strcat(outbuf,cbuff);
		}
		return outbuf;
	}
	return NULL;

}

char * html_encode(char *in){
int ll,i;
char * outbuf;
char cbuff[10];
	
	ll = strlen(in);
	if((outbuf = malloc(ll * 4))){
		outbuf[0] = 0;
		for(i=0;i<ll;i++){
			sprintf(cbuff, "&#%02x",in[i]);
			strcat(outbuf,cbuff);
		}
		return outbuf;
	}
	return NULL;
	
}

