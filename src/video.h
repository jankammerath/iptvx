#ifndef	VIDEO_H
#define VIDEO_H

#include <vlc/vlc.h>

void iptvx_video_init(char *videofile, int width, int height);
void iptvx_video_play(libvlc_video_lock_cb lock, libvlc_video_unlock_cb unlock, 
						libvlc_video_display_cb display, void* context);
void iptvx_video_free();

#endif