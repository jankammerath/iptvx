#ifndef	WINDOW_H
#define WINDOW_H

#include <SDL/SDL.h>

/* SDL context type */
struct sdl_context
{
    SDL_Surface *surf;
    SDL_mutex *mutex;
} typedef sdl_context;

void iptvx_window_set_overlay(void* overlay_ptr, bool* ready_ptr);
int iptvx_create_window(int width, int height,void (*keyDownCallback)(int),void (*startPlayCallback)(void*));
extern void *iptvx_window_lock(void *data, void **p_pixels);
extern void iptvx_window_unlock(void *data, void *id, void *const *p_pixels);
extern void iptvx_window_display(void *data, void *id);
sdl_context* iptvx_get_window_context();

#endif