#include <pthread.h>
#include <SDL.h>

static pthread_t win_thread;
static SDL_Surface *screen;
static SDL_Event event;

/* creates the main window for this application */
static void * iptvx_create_window(void* arg){
    SDL_Init(SDL_INIT_VIDEO);
    screen = SDL_SetVideoMode(640, 480, 16, SDL_SWSURFACE);
    SDL_WM_SetCaption(“Simple Window”, “Simple Window”);

    bool done = false;

    while(!done) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done=true;
        }
    }

    // fill the screen with black color
    SDL_FillRect(screen, &screen->clip_rect, SDL_MapRGB(screen->format, 0, 0, 0));

    // update the screen buffer
    SDL_Flip(screen);

    while(1){
        // nothing here
    }

    SDL_Quit();
}

/* create and process the window in separate thread */
void iptvx_create_window_thread(){
	if(pthread_create(&win_thread,NULL,iptvx_create_window,NULL) != 0) {
		fprintf (stderr, "Failed to launch thread for main window.\n");
        exit (EXIT_FAILURE);
	}	
}
