#include "http.h"

 http_error_codes_t HTTP_ERRORS[] = {
	{HTTP_RESULT_OK							,"OK"},
	{HTTP_RESULT_INTERNAL_SERVER_ERROR		,"some shit be broken, we're not good at technology"},
	{HTTP_RESULT_NOT_FOUND					,"oh, you were searious about wanting that?"},
	{HTTP_RESULT_ACCEPTED					,"sure, why not"},
	{HTTP_RESULT_NO_CONTENT					,"wait.. i know i was thinking of something"},
	{HTTP_RESULT_BAD_REQUEST				,"wait, that didnt make any sense"},
	{HTTP_RESULT_UNAUTHORIZED				,".. no.."},
	{HTTP_RESULT_FORBIDDEN					,".. fuck no.."},
	{HTTP_RESULT_METHOD_NOT_ALLOWED			,"... uh.. nah."},
	{HTTP_RESULT_NOT_ACCEPTED				,"nope"},
	{HTTP_RESULT_REQUEST_TIMEOUT			,"...hey, bro.. its puff puff pass and your camping.."},
	{HTTP_RESULT_URI_TOO_LONG				,".. you just sort of tailed off there without finishing your thought"},
	{HTTP_RESULT_NOT_IMPLEMENTED			,"yeah I was feeling lazy and didnt do it yet"},
	{HTTP_RESULT_HTTP_VERSION_NOT_SUPPORTED	,"woah.. you think this is some microsoft shit?"}
};



char * get_http_result_text(int code){
int i;
	
	for(i=0;HTTP_ERRORS[i].result_string;i++){
		if(HTTP_ERRORS[i].result_code == code){
			return HTTP_ERRORS[i].result_string;
		}
	}
	return "Unknown Error";
}


