/*

   Copyright 2017   Jan Kammerath

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