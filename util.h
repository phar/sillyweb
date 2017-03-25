#ifndef _UTIL_H
#define _UTIL_H

char * filter_nonprintables(char *filterstring,int len,  char filterchar);
char * sctime();
const char *get_file_ext(const char *file);
int is_dir(const char *path);
int is_file(const char *path);

char * trim_whitespace(char * str);
#endif
