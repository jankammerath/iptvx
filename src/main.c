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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <libconfig.h>
#include <glib.h>
#include <SDL/SDL.h>
#include "args.h"
#include "config.h"
#include "window.h"
#include "video.h"
#include "webkit.h"
#include "js.h"
#include "epg.h"
#include "keycode.h"
#include "daemon.h"
#include "db.h"

/* application status */
bool application_active;

/* context of the window */
void* main_window_context;

/* window dimensions */
int main_window_width;
int main_window_height;

/* module status */
bool main_js_ready;
bool main_window_ready;
bool main_epg_ready;

/* daemon mode indicator */
bool is_daemon;

/* the thread polling and pushing updates */
SDL_Thread* update_thread;

/* handles mouse events */
void mouse_event(int mouse_event_type, int mouse_x, int mouse_y, int mouse_button){
	GArray* mouseEvent = g_array_new(true,true,4);
	g_array_append_val(mouseEvent,mouse_event_type);
	g_array_append_val(mouseEvent,mouse_x);
	g_array_append_val(mouseEvent,mouse_y);
	g_array_append_val(mouseEvent,mouse_button);

	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
			(GSourceFunc)iptvx_js_sendmouse,mouseEvent,NULL);
}

/* handles any key down event */
void keydown(int keyCode){
	if(keyCode == 279){
		/* tells all threads to kill themselves */
		application_active = false;

		/* termination of application */
		iptvx_video_free();
	}else{
		/* forward key after converting from SDL to JS */
		if(main_js_ready){
			/* only when JS API alive */
			int convertedKeyCode = keycode_convert_sdl_to_js(keyCode);
			g_idle_add((GSourceFunc)iptvx_js_sendkey,
					GINT_TO_POINTER(convertedKeyCode));
		}		
	}
}

/* 
	plays a channel's url with the video player 
	@param		url 		the url to play
*/
void channel_video_play(char* url, bool stopCurrent){
	/* check if required to stop current playback */
	if(stopCurrent){
		/* ask video to stop current playback */
		iptvx_video_free();
	}

	/* initialise the video playback */
	iptvx_video_init(url,main_window_width,main_window_height);

	/* start the playback on screen */
	iptvx_video_play(iptvx_window_lock,iptvx_window_unlock,
					iptvx_window_display,main_window_context);	
}

/*
	Handles retrieval of API control messages
	@param		message 	the control message as string
*/
void control_message_received(void* message){
	/* create new string with control message */
	GString* controlMessage = g_string_new(message);

	/* defines the argument split char */
	gchar* splitChar = " ";

	/* control messages can have parameters or 
		are just a single string with a command 
		that takes no arguments */
	if(g_strrstr(controlMessage->str,splitChar) == NULL){
		/* this is an individual control message 
			that comes with no arguments attached */
	}else{
		/* this is a command with one or multiple 
			parameters attached; split em up */
		gchar** ctlMsg = g_strsplit(controlMessage->str,splitChar,-1);

		/* check which command this is */
		if(g_ascii_strncasecmp(ctlMsg[0],"switch-channel",13)==0){
			/* this is a channel switch control message,
				so we need to get the channel number */
			int chanId = g_ascii_strtoll(ctlMsg[1],NULL,0);
			iptvx_epg_set_current_channel_id(chanId);
			channel* newChan = iptvx_epg_get_current_channel();
			channel_video_play(newChan->url->str,true);

			/* update JS with new channel */
			int currentChannelId = iptvx_epg_get_current_channel_id();
			g_idle_add((GSourceFunc)iptvx_js_set_current_channel,
							GINT_TO_POINTER(currentChannelId));
		}

		if(g_ascii_strncasecmp(ctlMsg[0],"set-volume",10)==0){
			/* this is a channel switch control message,
				so we need to get the channel number */
			int volume_percent = g_ascii_strtoll(ctlMsg[1],NULL,0);

			/* set vol max to 150 percent */
			if(volume_percent > 150){
				volume_percent = 150;
			}if(volume_percent < 0){
				volume_percent = 0;
			}

			/* set volume to desired value */
			g_idle_add((GSourceFunc)iptvx_video_set_volume,
							GINT_TO_POINTER(volume_percent));

			/* update play and js */
			g_idle_add((GSourceFunc)iptvx_js_update_volume,
							GINT_TO_POINTER(volume_percent));
		}

		if(g_ascii_strncasecmp(ctlMsg[0],"set-audiotrack",10)==0){
			int track_id = g_ascii_strtoll(ctlMsg[1],NULL,0);
			g_idle_add((GSourceFunc)iptvx_video_set_audiotrack,
								GINT_TO_POINTER(track_id));
		}

		if(g_ascii_strncasecmp(ctlMsg[0],"set-subtitle",10)==0){
			int subtitle_id = g_ascii_strtoll(ctlMsg[1],NULL,0);
			g_idle_add((GSourceFunc)iptvx_video_set_subtitle,
								GINT_TO_POINTER(subtitle_id));
		}
	}

	/* free control message */
	g_string_free(controlMessage,true);
}

/*
	Handles browser load finished
	@param		webview 	the webview that finished
*/
void load_finished(void* webview){
	/* initialise JS API with webview and callback func */
	iptvx_js_init(webview,control_message_received);
	main_js_ready = true;
}

/* 
	handles signal when SDL window is ready 
	@param		context 	the SDL context of the finished window
*/
void window_ready(void* context){
	main_window_ready = true;

	/* keep window context as it might be required later on */
	main_window_context = context;
}

/*
	handles epg status updates
	@param 		progress 	int ptr with load percentage

	@warning	There is a race condition between the JS API
				and the EPG thread. Should in any occasion the 
				JS API take significant time to load, then the 
				while loop to wait for it might freeze the application
				or at least the thread executed from
*/
void epg_status_update(void* progress){
	/* get epg progress value */
	int progressVal = *(int*)progress;

	/* mark ready when 100 percent reached */
	if(progressVal == 100){
		/* check if daemon or client */
		if(is_daemon){
			/* get the epg data and flush into db */
			GArray* epg_data = iptvx_epg_get_data();
			iptvx_db_update(epg_data);

			/* is daemon, so send epg data in raw and as json */
			GString* epg_data_json = iptvx_epg_get_json();
			iptvx_daemon_set_epg_json(epg_data_json);
			iptvx_daemon_set_epg_data(epg_data);
		}else{
			/* we need to wait for js to come up as 
				otherwise it'll not get the notification 
				that the epg is ready */
			while(!main_js_ready){
				/* wait 1s for it to show up */
				sleep(1);
			}

			/* signal complete epg data, string will be freed inside */
			GString* epg_data = iptvx_epg_get_json();
			g_idle_add((GSourceFunc)iptvx_js_set_epg_data,epg_data);
			
			/* only start the initial channel playback 
				when the epg was not ready before as otherwise 
				it might interrupt existing playback when 
				the epg just received updates */
			if(!main_epg_ready){
				/* set epg ready to true */
				main_epg_ready = true;

				/* signal current channel */
				int currentChannelId = iptvx_epg_get_current_channel_id();
				g_idle_add((GSourceFunc)iptvx_js_set_current_channel,
								GINT_TO_POINTER(currentChannelId));

				/* activate video playback by getting
					the default channel and play it */
				channel* defaultChannel = iptvx_epg_get_default_channel();
				channel_video_play(defaultChannel->url->str,false);
			}
		}
	}

	/* send epg status to daemon if alive */
	if(is_daemon){
		iptvx_daemon_set_epg_status(progressVal);
	}

	/* send epg status to js client */
	if(main_js_ready){
		/* send epg status update to js */
		g_idle_add((GSourceFunc)iptvx_js_update_epg_status,
							GINT_TO_POINTER(progressVal));		
	}
}

/*
	Updates the media state from the player to the JS API
*/
int update(void* nothing){
	int media_state;

	/* ensure JS and App is active */
	while(application_active){
	 	if(main_js_ready == true){
			/* get the current media state */
			media_state = iptvx_video_get_state();

			/* signal it to the js api */
			g_idle_add((GSourceFunc)iptvx_js_update_state,
							GINT_TO_POINTER(media_state));

			/* get the window title from the overlay
				and update our actual window rendered */
			iptvx_window_set_title(iptvx_get_overlay_title());

			/* get the audio track information and forward 
				it to the frontend overlay app */
			GArray* audioTrackList = iptvx_video_get_audiotracks();
			g_idle_add((GSourceFunc)iptvx_js_set_audiotracks,audioTrackList);

			/* get subtitles and forward them to overlay app */
			GArray* subtitleList = iptvx_video_get_subtitles();
			g_idle_add((GSourceFunc)iptvx_js_set_subtitles,subtitleList);

			/* wait a sec */
			usleep(1000000);
		}
	}

	return 0;
}

/* starts main window application */
void start_window(){
	/* initialise window */
	iptvx_window_init();

	/* get preferred video size */
	window_size win_size = iptvx_window_get_size();

	/* define window size */
	main_window_width = iptvx_config_get_setting_int("width",win_size.width);
	main_window_height = iptvx_config_get_setting_int("height",win_size.height);

	/* get the rendering support setting */
	char* render_support = iptvx_config_get_setting_string("render","sw");

	/* get fullscreen setting */
	bool window_fullscreen = iptvx_config_get_setting_bool("fullscreen",true);
	iptvx_window_set_fullscreen(window_fullscreen);

	/* get log output setting */
	bool video_log = iptvx_config_get_setting_bool("stream_log_output",false);
	iptvx_video_set_log_output(video_log);

	/* get the pointers to the webkit png data and status */
	void* overlay_data = iptvx_get_overlay_ptr();
	void* overlay_ready = iptvx_get_overlay_ready_ptr();
	void* overlay_rendering = iptvx_get_overlay_rendering_ptr();
	iptvx_window_set_overlay(overlay_data,overlay_ready,overlay_rendering);

	/* start the webkit thread */
	char* overlayApp = iptvx_config_get_overlay_app();
	GString* gs_overlay_app = g_string_new(overlayApp);
	if(gs_overlay_app->len > 0){
		iptvx_webkit_start_thread(gs_overlay_app->str,main_window_width,
								main_window_height,load_finished);

		/* start the thread to update the js api */
		update_thread = SDL_CreateThread(update,NULL);

		/* create the main window which 
			will lock this main thread */
		iptvx_create_window(main_window_width,main_window_height,
							render_support,keydown,mouse_event,
							window_ready);
	}else{
		/* throw an error when we don't have the app */
		printf("App path not found or inaccessible\n");
	}

	/* free overlay app string */
	g_string_free(gs_overlay_app,true);
}

/* main application code */
int main (int argc, char *argv[]){
	/* set activity indicator to true */
	application_active = true;

	/* set the daemon indicator */
	is_daemon = false;

	/* parse input arguments first */
	struct arguments app_args = iptvx_parse_args(argc,argv);
	if(app_args.configFile != NULL){
		/* use the config file from args */
		iptvx_set_config_filename(app_args.configFile);
	}

	/* check if daemon process was requested */
	if(app_args.daemon == true){
		is_daemon = true;
	}

	/* ensure that there is a config file */
	if(iptvx_config_init() == true){
		main_js_ready = false;
		main_window_ready = false;
		main_epg_ready = false;

		/* initialise the database when daemon */
		char* db_file;
		if(is_daemon){
			db_file = iptvx_config_get_setting_string("db","/var/iptvx/db");
			iptvx_db_init(db_file);

			/* set the overlay app directory for serving it */
			GString* appdir = g_string_new(iptvx_config_get_overlay_app_dir());
			iptvx_daemon_set_app_dir(appdir);
		}

		/* get the hours to store in the epg */
		int epg_hours = iptvx_config_get_setting_int("epg_hours",48);
		iptvx_epg_set_storage_hours(epg_hours);

		/* get the days after the epg files expire */
		int epg_min_age_hours = iptvx_config_get_setting_int("epg_min_age_hours",3);
		iptvx_epg_set_min_age_hours(epg_min_age_hours);

		/* get the days after the epg files expire */
		int epg_expiry_days = iptvx_config_get_setting_int("epg_expiry_days",7);
		iptvx_epg_set_expiry_days(epg_expiry_days);

		/* get the data directory for the epg */
		char* epg_dir = iptvx_config_get_data_dir();
		iptvx_epg_set_data_dir(epg_dir);

		/* initialise the epg */
		config_t* cfg = iptvx_get_config();
		iptvx_epg_init(cfg,epg_status_update);

		/* check if window or daemon mode */
		if(is_daemon){
			/* get the configured server port */
			int daemon_port = iptvx_config_get_setting_int("daemon_port",8085);
			iptvx_daemon_set_server_port(daemon_port);

			/* set the recording tolerance for the daemon */
			iptvx_daemon_set_record_tolerance(iptvx_config_get_setting_int
												("record_tolerance",5));

			/* set the directory to put recordings into */
			iptvx_daemon_set_dir(iptvx_config_get_setting_string
									("record","/var/iptvx/video"));

			/* daemon mode execution which 
				will lock the thread */
			iptvx_daemon_run();
		}else{
			/* window or client app mode */
			start_window();
		}

		/* close the database when finished */
		iptvx_db_close();
	}else{
		/* throw an error when config not initialised */
		printf("Failed to initialise the configuration\n");
	}

	/* tells all threads to kill themselves */
	application_active = false;

	return 0;
}