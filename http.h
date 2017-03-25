#ifndef _HTTP_H
#define _HTTP_H
#define CONNECTION_FLAG_LINE_ENDING_USES_CR	1
#define CONNECTION_FLAG_HTTP_1_0			2
#define CONNECTION_FLAG_HTTP_1_1			4
#define CONNECTION_FLAG_CLOSE				8

#define HTTP_VERSION_1_0	"1.0"
#define HTTP_VERSION_1_1	"1.1"

#define HTTP_RESULT_OK	200
#define HTTP_RESULT_INTERNAL_SERVER_ERROR 500
#define HTTP_RESULT_NOT_FOUND	404
#define HTTP_RESULT_ACCEPTED 202
#define HTTP_RESULT_NO_CONTENT	204
#define HTTP_RESULT_BAD_REQUEST 400
#define HTTP_RESULT_UNAUTHORIZED 401
#define HTTP_RESULT_FORBIDDEN 403
#define HTTP_RESULT_METHOD_NOT_ALLOWED 405
#define HTTP_RESULT_NOT_ACCEPTED 406
#define HTTP_RESULT_REQUEST_TIMEOUT 408
#define HTTP_RESULT_URI_TOO_LONG 414
#define HTTP_RESULT_NOT_IMPLEMENTED 501
#define HTTP_RESULT_HTTP_VERSION_NOT_SUPPORTED 505


typedef struct http_error_codes_t{
	int	result_code;
	char * result_string;
}http_error_codes_t;


struct HTTP_Method{
	char * method;
	int (*handler) (struct client_t *client);
};

extern http_error_codes_t HTTP_ERRORS[];

#define SERVER_VERSION_STRING "Stoned v.1 (something spelled correctly)"

#define DEFAULT_FILENAME  "index.html"

char * get_http_result_text(int code);

#endif
