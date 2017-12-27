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

#include <glib.h>
#include <vlc/vlc.h>
#include <SDL/SDL.h>

/* represents a single recording */
struct recording{
   /* name of the channel to record */
   GString* channel;

   /* the url of the channel */
   GString* url;

   /* start and stop timings */
   long start;
   long stop;

   /* number of minutes already recorded */
   int seconds_recorded;

   /* size of recorded file in bytes */
   long filesize;

   /* 0 = scheduled, 1 = in progress, 
      2 = finished, 9 = failed */
   int status;

   /* ptr to recording thread */
   void* thread;

   /* filename of the recording */
   GString* filename;
} typedef recording;

/*
   Actually executes the recording using the recording
   data structure. It is supposed to be executed within
   a thread.
   @param         record      void-ptr to recording struct
*/
int iptvx_record_start_thread(void* recordingptr){
   int result = 0;

   /* case the void ptr to recording struct */
   recording* rec = (recording*)recordingptr;

   /* set recording indicator */
   rec->status = 1;

   /* initialise vlc structs */
   libvlc_instance_t * inst;
   libvlc_media_player_t *mp;
   libvlc_media_t *m;

   /* create param for recording output */
   char rec_param[512];
   sprintf(rec_param,"--sout=file/ts:%s",rec->filename->str);

   /* define vlc args for recording */
   char* vlc_args[] = {
      "--no-xlib", /* don't use the graphics lib */
      "--repeat", /* ensure it retries on failure */
      "--quiet", /* no verbose output */
      rec_param /* the actual file dumping */
   };

   /* create the vlc instance */
   inst = libvlc_new (sizeof(vlc_args) / sizeof(vlc_args[0]), (const char* const*)vlc_args);

   /* open the defined media file */
   m = libvlc_media_new_location(inst,rec->url->str);
   mp = libvlc_media_player_new_from_media(m);

   return result;
}

/*
   Starts the recording process and finishes it after stoptime
   @param         rec      struct representing recording data
*/
void iptvx_record_start(recording* rec){
   /* actually executes the thread */
   rec->thread = SDL_CreateThread(iptvx_record_start_thread,rec);
}