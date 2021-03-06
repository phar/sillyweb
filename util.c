#include "util.h"
#include <time.h>
#include <sys/stat.h>
#include "common.h"


char * sctime(time_t tt){
char * ctimeptr;
time_t ttime;
	
	if(tt == 0){
		ttime = time(NULL);
	}else{
		ttime = tt;
	}
	ctimeptr = ctime(&ttime);
	ctimeptr[strlen(ctimeptr)-1] = 0;
	return ctimeptr;
}


char * filter_nonprintables(char *filterstring,int len,  char filterchar){
int i;

	for(i=0;(i<len) && (filterstring[i] != 0);i++){
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


int is_file(const char *path){
struct stat path_stat;
	
	stat(path, &path_stat);
	return S_ISREG(path_stat.st_mode);
}

int is_dir(const char *path){
	struct stat path_stat;
	
	stat(path, &path_stat);
	return S_ISDIR(path_stat.st_mode);
}

char * trim_whitespace(char * str){
char *end;
	/*
	// Trim leading space
	while(isspace((unsigned char)*str)) str++;
	
	if(*str == 0)  // All spaces?
		return str;
	
	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace((unsigned char)*end)) end--;
	
	// Write new null terminator
	*(end+1) = 0;
	*/
	return str;
	
}
