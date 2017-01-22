#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>

struct programme{
	GString* title;
	long start;
	long stop;
	int productionDate;
	GString* category;
	GString* description;
} typedef programme;

struct channel{
	GString* name;
	GString* url;
	GString* epgUrl;
	programme programmeList[];
} typedef channel;

/* initialise epg */
bool epg_init(channel channelList[]){

}