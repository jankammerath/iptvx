#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "args.c"
#include "window.c"
#include "video.c"

/* main application code */
int main (int argc, char *argv[]){
	/* parse input arguments first */
	struct arguments arguments = iptvx_parse_args(argc,argv);

	/* ensure sufficient parameters where provided */
	if(arguments.sufficient == true){
		iptvx_create_window_thread();
		iptvx_video_play(arguments.input_video_file,arguments.input_html_file);
	}

	return 0;
}