#include "util.h"
#include <time.h>
#include "common.h"


char * sctime(){
char * ctimeptr;
time_t ttime;
	
	ttime = time(NULL);
	ctimeptr = ctime(&ttime);
	ctimeptr[strlen(ctimeptr)-1] = 0;
	return ctimeptr;
}


char * filter_nonprintables(char *filterstring, char filterchar){
int i;

	for(i=0;i<strlen(filterstring);i++){	
		if(!isprint(filterstring[i])){
			filterstring[i] = filterchar;
		}
	}
	return filterstring;
}



const char *get_file_ext(const char *file) {
const char *d;
 
	d = strrchr(file, '.');
	if(!d || d == file) return "";
		return d + 1;
}
