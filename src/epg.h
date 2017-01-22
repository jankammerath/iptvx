#ifndef	EPG_H
#define EPG_H

#include <stdbool.h>
#include <glib.h>

bool iptvx_epg_init(config_t* cfg);
GString* iptvx_epg_get_default_channel_url();
int iptvx_epg_load_channel(void* channelName);
void iptvx_epg_load();

#endif