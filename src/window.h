#ifndef	WINDOW_H
#define WINDOW_H

void iptvx_create_window_thread(int width, int height);
static void *iptvx_window_lock(void *data, void **p_pixels);
static void iptvx_window_unlock(void *data, void *id, void *const *p_pixels);
static void iptvx_window_display(void *data, void *id);

#endif