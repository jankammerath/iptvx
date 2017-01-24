#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <libconfig.h>
#include <SDL/SDL.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "util.h"

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
	bool isDefault;
	GString* name;
	GString* url;
	GString* epgUrl;
	GString* logoUrl;
	GArray* programmeList;
} typedef channel;

/* the channel list */
GArray* list;

GString* iptvx_epg_get_default_channel_url(){
	GString* result = g_string_new("");

	int c = 0;
	for(c = 0; c < list->len; c++){
		channel* current = &g_array_index(list,channel,c);
		if(current->isDefault == true){
			result = current->url;
		}
	}

	return result;
}

/* parses the programmes from xmltv and returns it as 
	a GArray holding programme structs */
GArray* iptvx_epg_get_programmelist(GString* xmltv){
	GArray* result = g_array_new(false,false,sizeof(programme));

	/* create libxml object instance */
	xmlDocPtr doc;
	xmlNodePtr cur, progNode, valNode;

	/* parse from the xmltv string */
	doc = xmlParseDoc(xmltv->str);
	cur = xmlDocGetRootElement(doc);

	/* get all programme nodes */
	for(progNode = cur->children; progNode != NULL; progNode = progNode->next){
		if(xmlStrcmp(progNode->name,"programme")==0){
			/* create prorgramme struct */
			programme prog;

			for(valNode = progNode->children; valNode != NULL; valNode = valNode->next){
				if(xmlStrcmp(valNode->name,"title")==0){
					prog.title = g_string_new(valNode->children->content);
				}
			}

			/* flush programme into result */
			g_array_append_val(result,prog);
		}
	}

	return result;
}

/* loads the epg of the defined channel */
void iptvx_epg_load_channel(channel* current){
	/* create EPG url for today */
	char epg_url[256];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(epg_url,sizeof(epg_url)-1,(char*)current->epgUrl,t);

	char* cacheFile = g_strrstr(epg_url,"/")+1;
	char* cacheFilePath = g_strjoin("","cache/epg/",cacheFile,NULL);

	GString* xmltv = g_string_new(NULL);
	if(!util_file_exists(cacheFilePath)){
		/* file doesn't exist, we need to fetch it */
		xmltv = util_download_string(epg_url);

		/* finally flush the xmltv to disk cache */

	}else{
		/* file exists, we'll get it */
	}

	/* parse the programme list from the xmltv data */
	current->programmeList = iptvx_epg_get_programmelist(xmltv);
}

/* initiates the epg load for each channel */
void iptvx_epg_load(){
	int c = 0;
	for(c = 0; c < list->len; c++){
		/* get this channel */
		channel* current = &g_array_index(list,channel,c);

		/* start the thread to capture xmltv epg */
		iptvx_epg_load_channel(current);
	}
}

/* initialise epg */
bool iptvx_epg_init(config_t* cfg){
	list = g_array_new (false,false,sizeof(channel));

	/* get the channels array from the config */
	config_setting_t* root = config_root_setting(cfg);
	config_setting_t* channels = config_setting_get_member(root,"channels");

	/* ensure channels are present in config */
	if(channels != NULL){
		int count = config_setting_length(channels);
		int i = 0;

		/* iterate channels in config */
		for (i = 0; i < count; i++) {
			/* create the new channel */
			channel current;

			/* get the config channel element */
			config_setting_t *element = config_setting_get_elem(channels, i);

			GString* channelName = g_string_new("");
			if (config_setting_lookup_string(element,"name",(const char**)&channelName)) {
            	current.name = channelName;
            }

            int channelDefault = 0;
            current.isDefault = false;
			if (config_setting_lookup_bool(element,"default",&channelDefault)) {
            	current.isDefault = (bool)channelDefault;
            }

            GString* channelUrl = g_string_new("");
			if (config_setting_lookup_string(element,"url",(const char**)&channelUrl)) {
            	current.url = channelUrl;
            }

            GString* logoUrl = g_string_new("");
			if (config_setting_lookup_string(element,"logoUrl",(const char**)&logoUrl)) {
            	current.logoUrl = logoUrl;
            }

            GString* epgUrl = g_string_new("");
			if (config_setting_lookup_string(element,"epgUrl",(const char**)&epgUrl)) {
            	current.epgUrl = epgUrl;
            }

            /* append channel to list */
            g_array_append_val(list,current);
		}

		// all channels parsed, launch epg load
		iptvx_epg_load();
	}else{
		/* output and error when channels not present */
		printf("Error getting channels from config\n");
	}
}