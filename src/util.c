/*

   Copyright 2017   Jan Kammerath

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

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

/* flushes the content into the file located at file */
void file_put_contents(GString* file, GString* content){
	FILE *fp = fopen(file->str, "ab");
    if (fp != NULL){
        fputs(content->str, fp);
        fclose(fp);
    }
}

/* reads contents of file and returns as GString pointer */
GString* file_get_contents(GString* file){
	GString* result;

	char * buffer = 0;
	long length;
	FILE * f = fopen (file->str, "rb");

	if (f){
	  fseek (f, 0, SEEK_END);
	  length = ftell (f);
	  
	  fseek (f, 0, SEEK_SET);
	  buffer = malloc (length);

	  if (buffer){
	    fread (buffer, 1, length, f);
	  }
	  fclose (f);
	}	

	result = g_string_new(buffer);
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