#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "args.h"
#include "window.h"
#include "video.h"
#include "webkit.h"

/* handles any key down event */
void keydown(int keyCode){
	printf("key %d\n",keyCode);
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

	/* ensure sufficient parameters where provided */
	if(arguments.sufficient == true){
		/* initialise the video playback */
		iptvx_video_init(arguments.input_video_file,1280,720);

		/* create the thread for the main window */
		iptvx_create_window(1280,720,keydown,startplay);

		/* start the webkit thread */
		//iptvx_webkit_start_thread(arguments.input_html_file);
	}

	return 0;
}