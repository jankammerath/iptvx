#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <glib.h>

/* checks if a file exists on disk */
bool util_file_exists(char* fileName){
	bool result = false;

	if(access(fileName, F_OK) != -1) {
		result = true;
	}

	return result;
}

/* replace a string in a string */
char* str_replace(const char* orig_str, const char* old_token, const char* new_token){
   char*       new_str = 0;
   const char* pos = strstr(orig_str, old_token);

   if (pos){
      new_str = calloc(1, strlen(orig_str) - strlen(old_token) + strlen(new_token) + 1);
      strncpy(new_str, orig_str, pos - orig_str);
      strcat(new_str, new_token);
      strcat(new_str, pos + strlen(old_token));
   }

   return new_str;
}

/* curl write function to flush into gstring */
static size_t curl_write_data(void *buffer, size_t G_GNUC_UNUSED size, size_t nmemb, void *userp){
	GString *s = userp;
	g_string_append_len(s, buffer, nmemb);
	return nmemb;
}

/* downloads a URL and returns the result as string */
GString* util_download_string(GString* url){
	GString* result = g_string_new("");

	/* curl types */
	CURL *curl;
  	CURLcode res;

  	/* initialise curl library */
	curl_global_init(CURL_GLOBAL_DEFAULT);
 	curl = curl_easy_init();

 	/* ensure CURL initialised properly */
 	if(curl){
 		/* define the url to fetch */
 		curl_easy_setopt(curl,CURLOPT_URL,url);

 		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_data);
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);

 		/* execute the query */
 		res = curl_easy_perform(curl);
    
    	/* check for any errors */ 
    	if(res != CURLE_OK){
      		fprintf(stderr, "cURL failed: %s\n",
            				curl_easy_strerror(res));
    	}
 
	    /* cleanup curl */ 
	    curl_easy_cleanup(curl);
 	}

 	/* cleanup global curl */
 	curl_global_cleanup();

 	return result;
}