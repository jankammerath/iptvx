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

#ifndef	WEBKIT_H
#define WEBKIT_H

#include <webkit2/webkit2.h>

/* 
  returns overlay png data link 
  @return       pointer to PNG data
*/
extern void* iptvx_get_overlay_ptr();

/*
  returns a char* with the current title of the webkit window
  @return       char* with the title text
*/
const char* iptvx_get_overlay_title();

/* 
  returns ptr to bool indicating if busy
  @return        true when budy, otherwise false
*/
extern void* iptvx_get_overlay_ready_ptr();

/*
  starts webkit thread
  @param    file                          char ptr with the file path to the html app
  @param    width                         int defining the width of the webkit window
  @param    height                        int defining the height of the webkit window
  @param    load_finished_callback_func   ptr to func to call when load finished
*/
extern void iptvx_webkit_start_thread(char *file,int width, int height,void (*load_finished_callback_func)(void*));

#endif