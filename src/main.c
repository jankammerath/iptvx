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

/* url of the daemon's channel list */
char* channelListUrl;

/* the thread polling and pushing updates */
SDL_Thread* update_thread;

/* handles mouse events */
void mouse_event(int mouse_event_type, int mouse_x, int mouse_y, int mouse_button){
	GArray* mouseEvent = g_array_new(true,true,4);
	g_array_append_val(mouseEvent,mouse_event_type);
	g_array_append_val(mouseEvent,mouse_x);
	g_array_append_val(mouseEvent,mouse_y);
	g_array_append_val(mouseEvent,mouse_button);
	iptvx_webkit_sendmouse(mouseEvent);
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
			int convertedKeyCode = keycode_convert_sdl_to_gtk(keyCode);
			if(convertedKeyCode != -1){
				iptvx_webkit_sendkey(convertedKeyCode);
			}
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
			/* in case the epg channel list is empty, 
				re-initialise the list from the daemon*/
			if(iptvx_epg_get_channel_count() == 0){
				iptvx_epg_init_client(channelListUrl);
			}

			/* this is a channel switch control message,
				so we need to get the channel number */
			int chanId = g_ascii_strtoll(ctlMsg[1],NULL,0);
			iptvx_epg_set_current_channel_id(chanId);
			channel* newChan = iptvx_epg_get_current_channel();

			/* ensure the channel is not null */
			if(newChan != NULL){
				/* request player to start playback */
				channel_video_play(newChan->url->str,true);

				/* update JS with new channel */
				int currentChannelId = iptvx_epg_get_current_channel_id();
				g_idle_add((GSourceFunc)iptvx_js_set_current_channel,
							GINT_TO_POINTER(currentChannelId));
			}
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
			iptvx_db_update(epg_data,iptvx_epg_get_min_age_hours());

			/* is daemon, so send epg data in raw and as json */
			GString* epg_data_json = iptvx_epg_get_json();
			iptvx_daemon_set_epg_json(epg_data_json);
			iptvx_daemon_set_epg_data(epg_data);

			/* signal epg status to daemon */
			iptvx_daemon_set_epg_status(progressVal);
		}
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

/* 
	Starts main window application providing user interface
*/
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

	/* initialise the client with the epg */
	channelListUrl = iptvx_config_get_setting_string("daemon_list",
								"http://127.0.0.1:8085/list.json");	
	bool init_result = iptvx_epg_init_client(channelListUrl);

	/* continue initialising the application when the
		epg was successfully set up with the server */
	if(init_result == true){
		/* start the webkit thread */
		char* overlayUrl = iptvx_config_get_setting_string("daemon_url",
									"http://127.0.0.1:8085/app/app.html");

		iptvx_webkit_start_thread(overlayUrl,main_window_width,
								main_window_height,load_finished);

		/* start the thread to update the js api */
		update_thread = SDL_CreateThread(update,NULL);

		/* create the main window which 
			will lock this main thread */
		iptvx_create_window(main_window_width,main_window_height,
							render_support,keydown,mouse_event,
							window_ready);
	}else{
		/* terminate the application and ask 
			the user to start the daemon first */
		printf("\e[1;1H\e[2J");
		printf("\033[1m\033[31m*** Connection to daemon failed ***\x1B[0m\n\n"
			"Connection to the daemon could not be established on:\n%s\n\n"
			"Please ensure that the daemon is up, running and it's\n"
			"address configured correctly. You can launch the daemon\n"
			"either through the system's service management or by typing:\n\n"
			"\033[33m\tservice iptvx start\x1B[0m\n\n"
			"This version cannot function without the daemon.\n\n"
			"For any further assistance, please visit iptvx.org.\n\x1B[0m",
			channelListUrl);
	}
}

/*
	Sets up epg with the config data
*/
void config_epg(){
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
}

/*
	Sets up the database with config data
*/
void config_db(){
	char* db_file;
	db_file = iptvx_config_get_setting_string("db","/var/iptvx/db");
	iptvx_db_init(db_file);
}

/*
	Starts the daemon application to provide application services
*/
void start_daemon(){
	/* set up the database */
	config_db();

	/* set the overlay app directory for serving it */
	GString* appdir = g_string_new(iptvx_config_get_overlay_app_dir());
	iptvx_daemon_set_app_dir(appdir);

	/* set the data dir for serving the logo files */
	GString* datadir = g_string_new(iptvx_config_get_data_dir());
	iptvx_daemon_set_data_dir(datadir);

	/* Set up epg config */
	config_epg();

	/* initialise the epg */
	config_t* cfg = iptvx_get_config();
	iptvx_epg_init(cfg,epg_status_update,true);

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

	/* close the database when finished */
	iptvx_db_close();
}

/*
	This allows the application to be tested without
	any threads or time-based procedures which allows
	development to check vital operations for their fitness.
*/
void test(){
	if(iptvx_config_init() == true){
		/* be a little more verbose for testing */
		printf("Configuration files are good to go.\n");

		/* give a little info that we're in test mode */
		printf("Performing epg test run...\n");
	
		/* set up the database */
		config_db();

		/* Set up epg config */
		config_epg();

		/* initialise the epg */
		config_t* cfg = iptvx_get_config();
		iptvx_epg_init(cfg,epg_status_update,false);

		/* close db when finished */
		iptvx_db_close();

		/* print out a message when finished */
		printf("Test run completed.\n");
	}else{
		/*
			Impossible to execute the 
			test due to a bad config
		*/
		printf("Test aborted due to bad config.\n");
	}	
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

	/*
		Application testing support argument:
		Many parts of this application run in a 
		timed/scheduled multi-threaded environment 
		making it hard to debug epg and other 
		functionality. Therefore the -t option
		performs actions in a single thread
		for better testing and debugging.
	*/
	if(app_args.test == true){
		/* execute the test procedures */
		test();

		/* quit the application upon finish */
		return 0;
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

		/* check if daemon or client mode */
		if(is_daemon){
			/* daemon mode */
			start_daemon();
		}else{
			/* window or client app mode */
			start_window();
		}
	}else{
		/* throw an error when config not initialised */
		printf("Failed to initialise the configuration\n");
	}

	/* tells all threads to kill themselves */
	application_active = false;

	return 0;
}