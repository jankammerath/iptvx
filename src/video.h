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

#ifndef	VIDEO_H
#define VIDEO_H

#include <vlc/vlc.h>
#include "videoinfo.h"

/*
   defines whether log data should be written
   @param         output_log     true will write output, false not
*/
void iptvx_video_set_log_output(bool output_log);

/* 
   initialises video playback and opens media 
   @param         videofile      the video url to open
   @param         width          width of the video (to draw)
   @param         height         height of the video (to draw)
*/
void iptvx_video_init(char *videofile, int width, int height);

/*
   Sets the audio track to use in playback
   @param      track_id       id of the audio track in the stream
*/
void iptvx_video_set_audiotrack(int track_id);

/*
   Sets the audio track to use in playback
   @param      track_id       id of the audio track in the stream
*/
void iptvx_video_set_subtitle(int subtitle_id);

/*
   Sets the audio volume of the playback in percent
   @param         percent        percentage value of the audio volume
*/
void iptvx_video_set_volume(int percentage);

/*
   Gets the current audio volume of the playback in percent
   @return     current audio volume in percent
*/
int iptvx_video_get_volume();

/* 
   start the actual video playback 
   @param      lock        callback to lock the surface
   @param      unlock         callback to unlock the surface
   @param      display     display that vlc draws on
   @param      context     context to draw video in
*/
void iptvx_video_play(libvlc_video_lock_cb lock, libvlc_video_unlock_cb unlock, 
						libvlc_video_display_cb display, void* context);

/*
   Returns the current state of the media 

   The following are supported:

   0  libvlc_NothingSpecial   
   1  libvlc_Opening    
   2  libvlc_Buffering  
   3  libvlc_Playing    
   4  libvlc_Paused  
   5  libvlc_Stopped    
   6  libvlc_Ended   
   7  libvlc_Error 

   @return        the status value as int
*/
int iptvx_video_get_state();

/*
   Stops current playback
*/
void iptvx_video_stop();

/*
   Stops and frees the instance
*/
void iptvx_video_free();

/*
   Gets the available audio tracks
   @return  GArray containing audio tracks
*/
GArray* iptvx_video_get_audiotracks();

/*
   Returns the input bitrate of the stream or media
   @return     input bitrate as float
*/
float iptvx_video_get_bitrate();

/*
   Returns the size of the currently played video
   @return     array with 0 = x and 1 = y as integer
*/
videosize iptvx_video_get_size();

/*
   Gets the available subtitle tracks
   @return  GArray containing subtitle tracks
*/
GArray* iptvx_video_get_subtitles();

#endif