#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <libconfig.h>

/* a programme on a channel */
struct programme{
	GString* title;
	long start;
	long stop;
	int productionDate;
	GString* category;
	GString* description;
} typedef programme;

/* a channel */
struct channel{
	GString* name;
	GString* url;
	GString* epgUrl;
	programme programmeList[];
} typedef channel;

/* the channel list */
channel list[];

/* initialise epg */
bool iptvx_epg_init(config_t* cfg){
	
}