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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <libconfig.h>
#include <glib.h>
#include "args.h"
#include "config.h"
#include "window.h"
#include "video.h"
#include "webkit.h"
#include "js.h"
#include "epg.h"
#include "keycode.h"

/* context of the window */
void* main_window_context;

/* window dimensions */
int main_window_width;
int main_window_height;

/* module status */
bool main_js_ready;
bool main_window_ready;
bool main_epg_ready;

/* handles any key down event */
void keydown(int keyCode){
	if(keyCode == 27){
		/* termination of application */
		iptvx_video_free();
	}else{
		/* forward key after converting from SDL to JS */
		if(main_js_ready){
			/* only when JS API alive */
			iptvx_js_sendkey(keycode_convert_sdl_to_js(keyCode));
		}		
	}
}

/*
	Handles retrieval of API control messages
	@param		message 	the control message as string
*/
void control_message_received(void* message){
	printf("CONTROL MESSAGE RECEIVED: %s\n",message);
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
	plays a channel's url with the video player 
	@param		url 		the url to play
*/
void channel_video_play(char* url){
	/* initialise the video playback */
	iptvx_video_init(url,main_window_width,main_window_height);

	/* start the playback on screen */
	iptvx_video_play(iptvx_window_lock,iptvx_window_unlock,
					iptvx_window_display,main_window_context);	
}

/* 
	handles signal when SDL window is ready 
	@param		context 	the SDL context of the finished window
*/
void window_ready(void* context){
	main_window_ready = true;

	/* keep window context as it might be required later on */
	main_window_context = context;

	/* start playback when epg is ready */
	if(main_epg_ready){
		/* get the default channel and play it */
		channel* defaultChannel = iptvx_epg_get_default_channel();
		channel_video_play((char*)defaultChannel->url);
	}
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
		main_epg_ready = true;

		/* we need to wait for js to come up as 
			otherwise it'll not get the notification 
			that the epg is ready */
		while(!main_js_ready){
			/* wait 1s for it to show up */
			sleep(1);
		}

		/* signal complete epg data */
		GString* epg_data = iptvx_epg_get_json();
		iptvx_js_set_epg_data(epg_data);

		/* signal current channel */
		iptvx_js_set_current_channel(iptvx_epg_get_current_channel_id());
	}

	if(main_js_ready){
		/* send epg status update to js */
		iptvx_js_update_epg_status(progressVal);		
	}
}

/* main application code */
int main (int argc, char *argv[]){
	/* parse input arguments first */
	struct arguments arguments = iptvx_parse_args(argc,argv);

	/* ensure that there is a config file */
	if(iptvx_config_init() == true){
		main_js_ready = false;
		main_window_ready = false;
		main_epg_ready = false;

		main_window_width = iptvx_config_get_setting_int("width",1280);
		main_window_height = iptvx_config_get_setting_int("height",720);

		/* initialise the epg */
		config_t* cfg = iptvx_get_config();
		iptvx_epg_init(cfg,epg_status_update);

		/* get the pointers to the webkit png data and status */
		void* overlay_data = iptvx_get_overlay_ptr();
		void* overlay_ready = iptvx_get_overlay_ready_ptr();
		iptvx_window_set_overlay(overlay_data,overlay_ready);

		/* start the webkit thread */
		char* overlayApp = iptvx_config_get_overlay_app();
		iptvx_webkit_start_thread(overlayApp,load_finished);

		/* create the thread for the main window */
		iptvx_create_window(main_window_width,
							main_window_height,
							keydown,window_ready);
	}


	return 0;
}