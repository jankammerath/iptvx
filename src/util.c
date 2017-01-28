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

/*
   @author     Jan Kammerath
   @date       25 Jan 2017

   Utility functions for the application to
   write files, read files, download URLs etc.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>
#include <glib.h>

/*
   Checks if a file exists on disk
   @param   fileName    the file path to check for
   @return              true when the file exists, otherwise false
*/
bool util_file_exists(char* fileName){
	bool result = false;

	if(access(fileName, F_OK) != -1) {
		result = true;
	}

	return result;
}

/*
   Writes a string into a file
   @param   file        the file path to write into
   @param   content     the content to write into the file
*/
void file_put_contents(GString* file, GString* content){
	FILE *fp = fopen(file->str, "ab");
    if (fp != NULL){
        fputs(content->str, fp);
        fclose(fp);
    }
}

/*
   Gets the contents of a file as string
   @param   file        the file path to read
   @return              the contents of the file
*/
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

/*
   cURL write function to write into string
   @param   in     		input char pointer
   @param	  nmemb		  size of the memory to write
   @param	  out			  string to write result into
   @return            result status
*/
int util_curl_write_string(char* in, uint size, uint nmemb, GString* out){
  g_string_append_len(out, in, nmemb);
  return nmemb;
}

/*
   cURL write function to write into byte array
   @param   in          input char pointer
   @param   nmemb       size of the memory to write
   @param   out         byte array to write result into
   @return              result status
*/
int util_curl_write_bytearray(char* in, uint size, uint nmemb, GByteArray* out){
  g_byte_array_append(out, in, nmemb);
  return nmemb;
}

/*
  Queries a URL with cURL and writes using the passed func and buf
  @param        url             The url to download
  @param        write_func      The function to use for writing
  @param        write_data      The data to write into
*/
void curl_download_url(char* url, void* write_func, void* write_data){
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
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, write_data);

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
}

/*
  Downloads a URL as byte array (raw data)
  @param    url         the url to download
  @return               the contents as byte array
*/
GByteArray* util_download_data(char* url){
  GByteArray* result = g_byte_array_new();

  curl_download_url(url,util_curl_write_bytearray,result);

  return result;
}

/*
   Downloads a URL as string
   @param   url         the url to download
   @return              the contents as string
*/
GString* util_download_string(char* url){
	GString* result = g_string_new(NULL);

  curl_download_url(url,util_curl_write_string,result);

 	return result;
}