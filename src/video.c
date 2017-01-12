#include <vlc/vlc.h>

libvlc_instance_t * inst;
libvlc_media_player_t *mp;
libvlc_media_t *m;

void iptvx_video_play(char *videofile, char *htmlfile){
	inst = libvlc_new (0, NULL);
	printf("Playback of %s\n",videofile);
	
	/* open the defined media file */
	m = libvlc_media_new_path(inst,videofile);
	mp = libvlc_media_player_new_from_media(m);
	
	/* give it a little time to start up */
	usleep(1000000);
	libvlc_media_release (m);

	/* attach to xwindow */
	libvlc_media_player_set_xwindow (mp, xiptv_window_id);

	/* start playback */
	libvlc_media_player_play (mp);
	
	/* wait for it to start playing */
	usleep(1000000);
	while(libvlc_media_player_is_playing(mp)){
		/* wait until finished */
	}	
	
	/* terminate the whole thing */
	libvlc_media_player_stop (mp);
	libvlc_media_player_release (mp);
	libvlc_release (inst);
}