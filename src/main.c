#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include "args.c"
#include "window.c"

/* main application code */
int main (int argc, char *argv[]){
	/* parse input arguments first */
	struct arguments arguments = iptvx_parse_args(argc,argv);

	/* ensure sufficient parameters where provided */
	if(arguments.sufficient == true){
		iptvx_create_window_thread();
	
		while(1){
			printf("I am still here!\n");
			usleep(5000000);
		}
	}

	return 0;
}