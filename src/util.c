/*

   Copyright 2018   Jan Kammerath

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
#include <sys/stat.h>
#include <glib.h>
#include <gio/gio.h>
#include <time.h>

/*
  Converts FOURCC UInt32 into a char buffer (string)
  @param        fourcc     FOURCC as UInt32
  @param        result     char buf with FOURCC value as string
*/
void util_get_fourcc_string(int fourcc, char* result){
  sprintf(result, "%c%c%c%c",
      fourcc & 0xFF, (fourcc >> 8) & 0xFF,
      (fourcc >> 16) & 0xFF, (fourcc >> 24) & 0xFF);
}

/*
  Gets the current time in milliseconds
  @returns      the current time in milliseconds as long
*/
long util_get_time_ms(){
  long result = 0;

  struct timespec tm;
  clock_gettime(CLOCK_REALTIME,&tm);

  /* 1 s = 1000 ms, 1 ms = 1000000 ns */
  result = (long)(tm.tv_sec*1000)+(tm.tv_nsec/1000000);

  return result;
}

/*
  Deletes a file from disk
  @param    fileName    file path of the file to delete
*/
void util_delete_file(char* fileName){
  remove(fileName);
}

/*
  Gets a portion of the string (sub string)
  @param      str       the string to take portion off
  @param      index     position on where to start
  @param      len       length of how much to take
  @return               the portion result string
*/
GString* util_substr(GString *str, int index, int len){
  GString* result = g_string_new("");

  /* take the max when len is less than 1 */
  if(len < 1){
    len = str->len - index;
  }

  int c;
  int pos;
  for(c=0;c<len;c++){
    pos = index + c;
    if(pos < str->len){
      g_string_append_c(result,str->str[pos]);
    }
  }

  return result;
}

/*
  Executes a shell command and returns stdout
  @param      command     the shell command to execute
  @return                 the STDOUT of the command
*/
GString* util_shell_exec(GString* command){
  GString* result = g_string_new("");

  /* open ptr to process */
  FILE* ptr = popen(command->str, "r");

  /* read STDOUT */
  if(ptr){
    int buffer_size = 100;
    gchar buf[buffer_size];
    while(fgets(buf,buffer_size,ptr)){
      g_string_append(result,buf);
    }
    pclose(ptr);
  }

  /* return result */
  return result;
}

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
  Returns the size of a file in byte
  @param    fileName    full file path of the file
  @return               long with the size in byte
*/
long util_get_filesize(char* fileName){
  long result = 0;

  if(util_file_exists(fileName)){
    struct stat st;
    stat(fileName, &st);
    result = st.st_size;
  }

  return result;
}

/*
   Writes a string into a file
   @param   file        the file path to write into
   @param   content     the content to write into the file
*/
void file_put_contents(GString* file, GString* content){
	FILE *fp = fopen(file->str, "wt");
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
  char *out;
  GError *e=NULL;
  GIOChannel *f  = g_io_channel_new_file(file->str, "r", &e);
  if (!f) {
    fprintf(stderr, "failed to open file '%s': %s\n", file, e->message);
    return NULL;
  }

  /* set default encoding for that file */
  g_io_channel_set_encoding(f, NULL, &e);

  long data_length = 0;
  if (g_io_channel_read_to_end(f, &out, &data_length, &e) != G_IO_STATUS_NORMAL){
    fprintf(stderr, "found file '%s' but couldn't read it:\n%s\n", 
                    file->str, e->message);
    return NULL;
  }

  /* free the io channel data and shut it down */
  g_io_channel_shutdown(f,false,NULL);
  g_io_channel_unref(f);

  /* copy into result string and free char buf */
  GString* result = g_string_new_len(out,data_length);
  free(out);

	return result;
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
  @return                       True when successfull, otherwise false
*/
bool curl_download_url(char* url, void* write_func, void* write_data){
  bool result = false;

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

    if(write_func != NULL){
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    }
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, write_data);

    /* execute the query */
    res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if(http_code == 200){
      result = true;
    }else{
      /* output error message */
      printf("HTTP %d for '%s'\n",http_code,url);
    }

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

/*
  Downloads a URL as file
  @param    url         the url to download
  @param    filePath    the file to write to
*/
void* util_download_file(char* url, char* filePath){
  FILE* file = fopen(filePath, "w");
  bool http_result = curl_download_url(url,NULL,file);
  fclose(file);

  if(!http_result){
    /* delete file on failure */
    unlink(filePath);
  }
}

/*
   Downloads a URL as string
   @param   url         the url to download
   @return              the contents as string
*/
GString* util_download_string(char* url){
	GString* result = g_string_new("");

  bool http_result = curl_download_url(url,util_curl_write_string,result);
  if(!http_result){
    result = g_string_new("");
  }

 	return result;
}

/*
  Gets the timestamp of the last modification time
  @param      fileName    path of the file to get date for
  @return                 the timestamp of the last modification
*/
long util_file_lastmodified(char* fileName){
  long result = 0;

  if(util_file_exists(fileName)){
    struct stat attrib;
    stat(fileName, &attrib);
    result = attrib.st_mtime;
  }

  return result;
}

/*
  Returns the mime type of the provided file
  @param        filename      string with the full file path of the file
  @return                     string with the guessed mime type
*/
GString* util_file_get_mime_type(GString* filename){
  /* have the gio lib guess the mime type */
  char* mimetype = g_content_type_guess(filename->str,NULL,0,NULL);

  /* copy the char buf into a gstring */
  GString* result = g_string_new(mimetype);

  /* free that mime type guessing char buf */
  g_free(mimetype);

  return result;
}

/*
  Returns the first position of needle in haystack or -1 if not present
  @returns        position of needle or -1 if not present   
*/
long util_strpos(char *haystack, char *needle){
   char *p = strstr(haystack, needle);
   if (p)
      return p - haystack;
   return -1; 
}

/*
  Returns the last position of needle in haystack or -1 if not present
  @returns        position of needle or -1 if not present   
*/
long util_strrpos(char *haystack, char *needle){
   char *p = g_strrstr(haystack, needle);
   if (p)
      return p - haystack;
   return -1; 
}