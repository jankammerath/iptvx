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
#ifndef JS_H
#define JS_H

#include <webkit2/webkit2.h>
#include "videoinfo.h"

/*
   Initialises the JavaScript API interface
   @param   webview                          the webview to work on
   @param   control_message_callback_func    callback func when control msgs arrive
*/
void iptvx_js_init(WebKitWebView* webView,void (*control_message_callback_func)(void*));

/*
  Updates the volume info in the JS api
  @param      percentage        current volume percentage
*/
void iptvx_js_update_volume(int percentage);

/*
  Signals current epg status to the js app
  @param      percentage        int percentage value of progress
*/
void iptvx_js_update_epg_status(int percentage);

/*
  Updates the video codec
  @param    codec       current codec as char buf
*/
void iptvx_js_update_codec(char* codec);

/*
  Updates the video bitrate
  @param    bitrate       current bitrate as float
*/
void iptvx_js_update_bitrate(float* bitrate);

/*
  Updates the video size
  @param      sizeinfo    videosize struct with size information
*/
void iptvx_js_update_videosize(videosize* sizeinfo);

/*
  Signals media state to the js api
  @param      current_state        defines current media state
*/
void iptvx_js_update_state(int current_state);

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

/*
  underlying function for audio tracks and subtitles
  @param    tracklist     GArray with either audio or subtitle tracks
  @param    jsobject      the jsobject of iptvx-obj to assign json to
*/
void iptvx_js_set_tracks(GArray* tracklist, char* jsobject);

/*
  Sets the audio track information (name and id)
  @param    tracklist         GArray with audiotrack structs
*/
void iptvx_js_set_audiotracks(GArray* tracklist);

/*
  Sets the subtitle information (name and id)
  @param    tracklist         GArray with audiotracks for subtitles
*/
void iptvx_js_set_subtitles(GArray* tracklist);

/* 
  handles any mouse move, scroll or click event
  @param    mouse_event_type  0 = move, 1 = button 
  @param    mouse_x       x pos of the cursor
  @param    mouse_y       y pos of the cursor
  @param    mouse_button    0 = left, 1 = middle, 2 = right
*/
void iptvx_js_sendmouse(int mouse_event_type, int mouse_x, int mouse_y, int mouse_button);

#endif