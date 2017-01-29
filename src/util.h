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

#ifndef	UTIL_H
#define UTIL_H

/*
  Executes a shell command and returns stdout
  @param      command     the shell command to execute
  @return                 the STDOUT of the command
*/
GString* util_shell_exec(GString* command);

/*
   Checks if a file exists on disk
   @param   fileName    the file path to check for
   @return              true when the file exists, otherwise false
*/
bool util_file_exists(char* fileName);

/*
   Writes a string into a file
   @param   file        the file path to write into
   @param   content     the content to write into the file
*/
void file_put_contents(GString* file, GString* content);

/*
   Gets the contents of a file as string
   @param   file        the file path to read
   @return              the contents of the file
*/
GString* file_get_contents(GString* file);

/*
  Downloads a URL as file
  @param    url         the url to download
  @param    filePath    the file to write to
*/
void* util_download_file(char* url, char* filePath);

/*
   Downloads a URL as string
   @param   url         the url to download
   @return              the contents as string
*/
GString* util_download_string(char* url);

#endif