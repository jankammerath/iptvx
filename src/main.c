#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "args.h"
#include "window.h"
#include "video.h"
#include "webkit.h"

/* main application code */
int main (int argc, char *argv[]){
	/* parse input arguments first */
	struct arguments arguments = iptvx_parse_args(argc,argv);

	/* ensure sufficient parameters where provided */
	if(arguments.sufficient == true){
		iptvx_webkit_start(arguments.input_html_file);

		/*iptvx_create_window_thread();

		printf("getting xid\n");
		int window_xid = -1;
		while(window_xid == -1){
			window_xid = iptvx_get_window_xid();
		}

		printf("setting xid\n");
		iptvx_video_set_window_xid(window_xid);
		printf("starting video playback\n");
		iptvx_video_play(arguments.input_video_file);*/
	}

	return 0;
}