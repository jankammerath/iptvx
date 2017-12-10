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

#include <vlc/vlc.h>
#include <unistd.h>
#include <glib.h>

libvlc_instance_t * inst;
libvlc_media_player_t *mp;
libvlc_media_t *m;

int video_width;
int video_height;
bool video_log_output;

struct audiotrack{
	int 		id;
	GString*	name;
	bool		active;
} typedef audiotrack;

/*
	defines whether log data should be written
	@param 			output_log 		true will write output, false not
*/
void iptvx_video_set_log_output(bool output_log){
	video_log_output = output_log;
}


/* 
	initialises video playback and opens media 
	@param			videofile 		the video url to open
	@param 			width 			width of the video (to draw)
	@param 			height 			height of the video (to draw)
*/
void iptvx_video_init(char *videofile, int width, int height){
	char* vlc_args[] = {
		"--no-xlib", "--quiet" /* tell VLC to not use Xlib */
	};

	if(video_log_output){
		vlc_args[1] = "";
	}

	/* create the VLC instance */
	inst = libvlc_new (sizeof(vlc_args) / sizeof(vlc_args[0]), (const char* const*)vlc_args);

	/* open the defined media file */
	m = libvlc_media_new_location(inst,videofile);
	mp = libvlc_media_player_new_from_media(m);

	/* configure width and height of the video */
	video_width = width;
	video_height = height;
}

/*
	Sets the audio volume of the playback in percent
	@param         percent        percentage value of the audio volume
*/
void iptvx_video_set_volume(int percentage){
	/* pass volume to libvlc */
	libvlc_audio_set_volume(mp,percentage);
}

/*
	Gets the current audio volume of the playback in percent
	@return 		current audio volume in percent
*/
int iptvx_video_get_volume(){
	return libvlc_audio_get_volume(mp);
}

/* 
	start the actual video playback 
	@param		lock 			callback to lock the surface
	@param 		unlock 			callback to unlock the surface
	@param 		display 		display that vlc draws on
	@param 		context 		context to draw video in
*/
void iptvx_video_play(libvlc_video_lock_cb lock, libvlc_video_unlock_cb unlock, 
						libvlc_video_display_cb display, void* context){	
	/* give it a little time to start up */
	usleep(1000000);
	libvlc_media_release (m);

	libvlc_video_set_callbacks(mp, lock, unlock, display, context);

	/* configure width and height of the video */
	libvlc_video_set_format(mp, "RV16", video_width, video_height, video_width*2);

	/* start playback */
	libvlc_media_player_play (mp);
}

/*
   	Returns the current state of the media 

	The following are supported:

	0	libvlc_NothingSpecial 	
	1	libvlc_Opening 	
	2	libvlc_Buffering 	
	3	libvlc_Playing 	
	4	libvlc_Paused 	
	5	libvlc_Stopped 	
	6	libvlc_Ended 	
	7	libvlc_Error 

   	@return        the status value as int
*/
int iptvx_video_get_state(){
	int result = 0;

	if(mp != NULL){
		libvlc_media_t* cmedia = libvlc_media_player_get_media(mp);
		if(cmedia != NULL){
			result = libvlc_media_get_state	(cmedia);
		}
	}

	return result;
}

/*
	Stops current playback
*/
void iptvx_video_stop(){
	/* tell vlc to stop current playback */
	libvlc_media_player_stop (mp);
}

/*
   Stops and frees the instance
*/
void iptvx_video_free(){
	/* terminate the whole thing */
	libvlc_media_player_stop (mp);
	libvlc_media_player_release (mp);
	libvlc_release (inst);	
}

/*
	Gets the available audio tracks
	@return 	GArray containing audio tracks
*/
GArray* iptvx_video_get_audiotracks(){
	GArray* result = g_array_new(false,true,sizeof(audiotrack));
	int player_state = iptvx_video_get_state();

	/* only get data when playing or paused */
	if(player_state == 3 || player_state == 4){
		/* get the currently active track */
		int active_track_id = libvlc_audio_get_track(mp);

		/* get the number of tracks */
		int track_count = libvlc_audio_get_track_count(mp);

		if(track_count > 0){
			/* get the current audio tracks */
			libvlc_track_description_t* tdesc = libvlc_audio_get_track_description(mp);

			audiotrack track[track_count];
			track[0].name = g_string_new(tdesc->psz_name);
			track[0].id = tdesc->i_id;
			track[0].active = false;
			if(track[0].id == active_track_id){
				/* track is currently active */
				track[0].active = true;
			}

			g_array_append_val(result,track[0]);

			for(int i=1;i<track_count;i++){
				tdesc = tdesc->p_next;
				track[i].name = g_string_new(tdesc->psz_name);
				track[i].id = tdesc->i_id;
				track[i].active = false;
				if(track[i].id == active_track_id){
					/* track is currently active */
					track[i].active = true;
				}
				g_array_append_val(result,track[i]);
			}
		}
	}

	return result;
}

/*
	Gets the available subtitle tracks
	@return 	GArray containing subtitle tracks
*/
GArray* iptvx_video_get_subtitles(){
	GArray* result = g_array_new(false,true,sizeof(audiotrack));
	int player_state = iptvx_video_get_state();

	/* only get data when playing or paused */
	if(player_state == 3 || player_state == 4){
		/* get the currently active track */
		int active_subtitle_id = libvlc_video_get_spu(mp);

		/* get the number of tracks */
		int track_count = libvlc_video_get_spu_count(mp);

		if(track_count > 0){
			/* get the current audio tracks */
			libvlc_track_description_t* tdesc = libvlc_video_get_spu_description(mp);

			audiotrack track[track_count];
			track[0].name = g_string_new(tdesc->psz_name);
			track[0].id = tdesc->i_id;
			track[0].active = false;
			if(track[0].id == active_subtitle_id){
				/* track is currently active */
				track[0].active = true;
			}

			g_array_append_val(result,track[0]);

			for(int i=1;i<track_count;i++){
				tdesc = tdesc->p_next;
				track[i].name = g_string_new(tdesc->psz_name);
				track[i].id = tdesc->i_id;
				track[i].active = false;
				if(track[i].id == active_subtitle_id){
					/* track is currently active */
					track[i].active = true;
				}
				g_array_append_val(result,track[i]);
			}
		}
	}

	return result;
}