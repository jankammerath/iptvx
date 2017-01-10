#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "window.c"

int main (int argc, char *argv[])
{
	xiptv_create_window_thread();
	
	while(1){
		printf("I am still here!\n");
		usleep(5000000);
	}
}
