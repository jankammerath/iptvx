#ifndef	VIDEO_H
#define VIDEO_H

#include <vlc/vlc.h>

extern libvlc_instance_t * inst;
extern libvlc_media_player_t *mp;
extern libvlc_media_t *m;

extern void iptvx_video_play(char*);
extern void iptvx_video_set_window_xid(int);


#endif