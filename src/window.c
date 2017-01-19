#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mutex.h>

/* SDL context type */
struct sdl_context
{
    SDL_Surface *surf;
    SDL_mutex *mutex;
} typedef sdl_context;

/* window args type */
struct create_window_args{
    int width;
    int height;
} typedef create_window_args;

/* window variables */
SDL_Thread *window_thread;
int window_terminate;

/* creates the main window for this application */
int iptvx_create_window(void* window_arguments){
    SDL_Surface *screen, *overlay;
    SDL_Event event;
    SDL_Rect rect;
    sdl_context ctx;

    /* cast window argument to proper struct */
    create_window_args* args = (create_window_args*)window_arguments;

    /* set window terminate to false */
    window_terminate = false;

    /* initialise the SDL lib */
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTTHREAD) == -1){
        printf("Unable to initialize SDL\n");
    }

    /* create the SDL surface */
    ctx.surf = SDL_CreateRGBSurface(SDL_SWSURFACE, args->width, args->height,
                                    16, 0x001f, 0x07e0, 0xf800, 0);
    ctx.mutex = SDL_CreateMutex();

    int options = SDL_ANYFORMAT | SDL_HWSURFACE | SDL_DOUBLEBUF;
    screen = SDL_SetVideoMode(args->width, args->height, 32, options);
    if(!screen)
    {
        printf("Unable to set video mode for SDL\n");
    }

    while(!window_terminate){
      /* Blitting the surface does not prevent it from being locked and
       * written to by another thread, so we use this additional mutex. */
      SDL_LockMutex(ctx.mutex);
      SDL_BlitSurface(ctx.surf, NULL, screen, NULL);
      SDL_BlitSurface(overlay, NULL, screen, NULL);
      SDL_UnlockMutex(ctx.mutex);
    }

    /* Close window and clean up SDL */
    SDL_DestroyMutex(ctx.mutex);
    SDL_FreeSurface(ctx.surf);
    SDL_FreeSurface(overlay);
    SDL_Quit();
}

static void *iptvx_window_lock(void *data, void **p_pixels){
    sdl_context *ctx = data;

    SDL_LockMutex(ctx->mutex);
    SDL_LockSurface(ctx->surf);
    *p_pixels = ctx->surf->pixels;
    return NULL; /* picture identifier, not needed here */
}

static void iptvx_window_unlock(void *data, void *id, void *const *p_pixels){
    sdl_context *ctx = data;

    SDL_UnlockSurface(ctx->surf);
    SDL_UnlockMutex(ctx->mutex);

    assert(id == NULL); /* picture identifier, not needed here */
}

static void iptvx_window_display(void *data, void *id){
    /* LibVLC wants to display the video */
    (void) data;
    assert(id == NULL);
}

void iptvx_create_window_thread(int width,int height){
  create_window_args args;
  args.width = width;
  args.height = height;
  window_thread = SDL_CreateThread(iptvx_create_window,&args);
}
