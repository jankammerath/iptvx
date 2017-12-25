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
#include <stdbool.h>
#include <assert.h>
#include <glib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mutex.h>

/* window args type */
struct create_window_args{
    int width;
    int height;
} typedef create_window_args;

/* png data type */
struct png_data{
    unsigned char* data;
    unsigned int length;
} typedef png_data;

/* SDL context type */
struct sdl_context{
    SDL_Surface *surf;
    SDL_mutex *mutex;
} typedef sdl_context;

/* defines a window size */
struct window_size{
    int width;
    int height;
} typedef window_size;

/* callback function types */
typedef void keydown_callback(int keyCode);
typedef void mouseevent_callback(int mouse_event_type, int mouse_x, 
                                int mouse_y, int mouse_button);
typedef void startplay_callback(void*);

/* window variables */
SDL_Thread *window_thread;
bool window_terminate;
bool window_fullscreen;
sdl_context ctx;

/* ptr to rendered data */
void* overlay_data;
bool* overlay_ready;

/* local storage */
GByteArray* current_overlay;

/*
    Initialises window and graphics library
*/
void iptvx_window_init(){
    /* initialise the SDL lib */
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1){
        printf("Unable to initialize SDL\n");
    }
}

/*
    Defines whether to start window in fullscreen or not
    @param      fullscreen_val      true when in full, false to be windowed
*/
void iptvx_window_set_fullscreen(bool fullscreen_val){
    window_fullscreen = fullscreen_val;
}

/*
    Returns the preferred window size
    @return         window size with width and height
*/
window_size iptvx_window_get_size(){
    window_size result;

    /* get the current video values */
    const SDL_VideoInfo* info = SDL_GetVideoInfo(); 
    result.width = info->current_w; 
    result.height = info->current_h; 

    return result;
}

/*
    defines the overlay data
    @param      overlay_ptr         pointer to PNG data
    @param      ready_ptr           pointer to bool indicating if ready
*/
void iptvx_window_set_overlay(void* overlay_ptr, bool* ready_ptr){
    overlay_data = overlay_ptr;
    overlay_ready = ready_ptr;
}

/*
    sets the title of the window
    @param      title               const char defining the title text
*/
void iptvx_window_set_title(const char* title){
    SDL_WM_SetCaption(title,0);
}

/* 
    creates the main window for this application 
    @param          width               defines width of the window
    @param          height              defines height of the window
    @param          keydown_callback     callback func when key down event
    @param          startplay_callback   callback func to call when playback can start
*/
int iptvx_create_window(int width, int height, 
                    keydown_callback* keydown_func,
                    mouseevent_callback* mouseevent_func,
                    startplay_callback* startplay_func){
    SDL_Surface *screen, *overlay;
    SDL_Event event;

    /* set window terminate to false */
    window_terminate = false;

    /* defines if fullscreen is active */
    bool is_fullscreen = false;

    /* create the SDL surface */
    int sdl_surface_option = SDL_HWSURFACE; // SDL_SWSURFACE for software
    ctx.surf = SDL_CreateRGBSurface(sdl_surface_option, width, height, 16, 0x001f, 0x07e0, 0xf800, 0);
    ctx.mutex = SDL_CreateMutex();

    /* alternatively SDL_SWSURFACE can be used for CPU rendering.
     * Use SDL_RESIZABLE to make the window resizable.
     */
    int sdl_video_options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF; 

    if(window_fullscreen){
        sdl_video_options ^= SDL_FULLSCREEN;
    }

    screen = SDL_SetVideoMode(width, height, 32, sdl_video_options);
    if(!screen){
        printf("Unable to set video mode for SDL\n");
    }

    /* set window caption */
    SDL_WM_SetCaption("iptvx",0);

    /* exec callback to call for player */
    startplay_func(&ctx);

    while(!window_terminate){
        int keyPressed;

        /* poll SDL events and act accordingly */
        while(SDL_PollEvent(&event)){ 
            /* handle the SDL events */
            switch(event.type){
                /* kill the application */
                case SDL_QUIT:
                    window_terminate = true;
                    break;
                case SDL_MOUSEMOTION:
                    /* the computer mouse was moved */
                    mouseevent_func(0,event.motion.x,event.motion.y,
                                    event.motion.state);
                    break;
                case SDL_MOUSEBUTTONUP:
                    /* call the handler for clicks (same as motion above) */
                    mouseevent_func(1,event.motion.x,event.motion.y,
                                    event.motion.state);
                    break;
                case SDL_KEYDOWN:
                    keyPressed = event.key.keysym.sym;

                    /* toggle fullscreen when '^' is pressed
                        which is SDL_KeyCode 94 */
                    if(keyPressed == 94){
                        SDL_WM_ToggleFullScreen(screen);
                    }

                    keydown_func(keyPressed);
                    break;
            }

            /* handle pressed keys */
            switch(keyPressed){
                case SDLK_END:
                    window_terminate = true;
                    break;
            }
        }

        /* Blitting the surface does not prevent it from being locked and
        * written to by another thread, so we use this additional mutex. */
        SDL_LockMutex(ctx.mutex);

        /* blit the video surface */
        SDL_BlitSurface(ctx.surf, NULL, screen, NULL);

        /* take the data from the new overlay */
        if(overlay_ready){
            png_data* overlay_png_ref = (png_data*)overlay_data;
            g_byte_array_free(current_overlay,false);
            current_overlay = g_byte_array_new_take(overlay_png_ref->data,
                                                    overlay_png_ref->length);
        }

        if(current_overlay->data != NULL){
            SDL_RWops *overlay_rwops = SDL_RWFromMem(current_overlay->data,current_overlay->len);
            overlay = IMG_LoadPNG_RW(overlay_rwops);
            SDL_BlitSurface(overlay, NULL, screen, NULL);
            SDL_FreeRW(overlay_rwops);
            SDL_FreeSurface(overlay);
        }

        /* unlock mutex */
        SDL_UnlockMutex(ctx.mutex);

        /* flush to screen */
        SDL_Flip(screen);
        SDL_Delay(5);
    }

    /* Close window and clean up SDL */
    SDL_DestroyMutex(ctx.mutex);
    SDL_FreeSurface(ctx.surf);
    SDL_Quit();
}

/*
    locks the window surface and mutex
    @param          data        pointer to data
    @param          p_pixels    pointer to pixels
*/
extern void *iptvx_window_lock(void *data, void **p_pixels){
    sdl_context *ctx = data;

    SDL_LockMutex(ctx->mutex);
    SDL_LockSurface(ctx->surf);
    *p_pixels = ctx->surf->pixels;
    return NULL; /* picture identifier, not needed here */
}

/*
    unlocks the window surface and mutex
    @param          data        pointer to data
    @param          id          picture identifier (not needed, legacy)
    @param          p_pixels    pointer to pixels
*/
extern void iptvx_window_unlock(void *data, void *id, void *const *p_pixels){
    sdl_context *ctx = data;

    SDL_UnlockSurface(ctx->surf);
    SDL_UnlockMutex(ctx->mutex);

    assert(id == NULL); /* picture identifier, not needed here */
}

/*
    displays the window surface
    @param          data        pointer to data
    @param          id          picture identifier (not needed, legacy)
*/
extern void iptvx_window_display(void *data, void *id){
    /* LibVLC wants to display the video */
    (void) data;
    assert(id == NULL);
}