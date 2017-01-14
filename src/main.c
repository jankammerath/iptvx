#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <X11/Xlib.h>
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
		/* init threads for X11 */
		XInitThreads();

		/* create the thread for the main window */
		iptvx_create_window_thread();

		/* wait for the main window to get its XID */
		int window_xid = -1;
		while(window_xid == -1){
			window_xid = iptvx_get_window_xid();
		}

		/* start the webkit thread */
		iptvx_webkit_start_thread(arguments.input_html_file);

		/* pass the XID onto the player and play */
		iptvx_video_set_window_xid(window_xid);
		iptvx_video_play(arguments.input_video_file);
	}

	return 0;
}