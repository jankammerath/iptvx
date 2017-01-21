#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "args.h"
#include "config.h"
#include "window.h"
#include "video.h"
#include "webkit.h"

/* handles any key down event */
void keydown(int keyCode){
	if(keyCode == 27){
		// termination of application
		iptvx_video_free();
	}else{
		// forward key input to webkit
		iptvx_webkit_sendkey(keyCode);		
	}
}

/* starts playback when SDL window is ready */
void startplay(void* context){
	/* start the playback on screen */
	iptvx_video_play(iptvx_window_lock,iptvx_window_unlock,iptvx_window_display,context);
}

/* main application code */
int main (int argc, char *argv[]){
	/* parse input arguments first */
	struct arguments arguments = iptvx_parse_args(argc,argv);

	/* ensure that there is a config file */
	if(iptvx_config_init() == true){
		int width = iptvx_config_get_setting_int("width",1280);
		int height = iptvx_config_get_setting_int("height",720);

		/* initialise the video playback */
		iptvx_video_init("http://daserste_live-lh.akamaihd.net/i/daserste_de@91204/index_2692_av-b.m3u8?sd=10&rebase=on",width,height);

		/* get the pointers to the webkit png data and status */
		void* overlay_data = iptvx_get_overlay_ptr();
		void* overlay_ready = iptvx_get_overlay_ready_ptr();
		iptvx_window_set_overlay(overlay_data,overlay_ready);

		/* start the webkit thread */
		char* overlayApp = iptvx_config_get_overlay_app();
		iptvx_webkit_start_thread(overlayApp);

		/* create the thread for the main window */
		iptvx_create_window(width,height,keydown,startplay);
	}


	return 0;
}