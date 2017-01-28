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
#ifndef JS_H
#define JS_H

#include <webkit2/webkit2.h>

/*
   Initialises the JavaScript API interface
   @param   webview                          the webview to work on
   @param   controlMessageCallbackFunc       callback func when control msgs arrive
*/
void iptvx_js_init(WebKitWebView* webView,void (*controlMessageCallbackFunc)(void*));

/*
  Signals current epg status to the js app
  @param      percentage        int percentage value of progress
*/
void iptvx_js_update_epg_status(int percentage);

/*
  Signals the current channel to the JS app
  @param      channel_id        int value with channel list index
*/
void iptvx_js_set_current_channel(int channel_id);

/*
  Signals the epg data to the JS app
  @param      epg_data        JSON Array with complete epg data
*/
void iptvx_js_set_epg_data(GString* epg_data);

/*
   Sends a key down event to the js application
   @param   keyCode           the code of the key to transmit
*/
void iptvx_js_sendkey(int keyCode);

#endif