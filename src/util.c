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

/* cURL write function to flush into the gstring */
int util_curl_write_data(char* in, uint size, uint nmemb, GString* out){
  g_string_append_len(out, in, nmemb);
  return nmemb;
}

/* downloads a URL and returns the result as string */
GString* util_download_string(char* url){
	GString* result = g_string_new(NULL);

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

 		/* set the curl options for fetching the data */
 		curl_easy_setopt(curl, CURLOPT_USERAGENT, "iptvx/1.0 (iptvx.org)");
 		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
 		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
 		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, util_curl_write_data);
    	curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);

 		/* execute the query */
 		res = curl_easy_perform(curl);

    	/* check for any errors */ 
    	if(res != CURLE_OK){
      		fprintf(stderr, "cURL failed for '%s':\n%s\n", 
      				url, curl_easy_strerror(res));
    	}
 
	    /* cleanup curl */ 
	    curl_easy_cleanup(curl);
 	}

 	/* cleanup global curl */
 	curl_global_cleanup();

 	return result;
}