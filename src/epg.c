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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <libconfig.h>
#include <SDL/SDL.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <time.h>
#include <json-c/json.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "util.h"

/* thread loading the epg data */
SDL_Thread *epg_thread;

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
	GString* urlShell;
	GString* epgUrl;
	GString* epgFile;
	GString* epgShell;
	GString* logoFile;
	GArray* programmeList;
} typedef channel;

/* the channel list */
GArray* list;

/* defines current channel */
int iptvx_epg_current_channel;

/* indicates if ready */
bool iptvx_epg_ready;
int iptvx_epg_percentage_loaded;

/* status update callback */
void (*epgStatusUpdateCallback)(void*);

/*
	Returns all channel and epg info as JSON string
	@return 		JSON string with all EPG info
*/
GString* iptvx_epg_get_json(){
	GString* result = g_string_new(NULL);

	/* json result object array */
	json_object* j_chan_array = json_object_new_array();

	int c = 0;
	for(c = 0; c < list->len; c++){
		/* get the channel */
		channel* chan = &g_array_index(list,channel,c);

		/* create the channel js object */
		json_object* j_chan = json_object_new_object();

		json_object_object_add(j_chan,"name",
			json_object_new_string((char*)chan->name));
		json_object_object_add(j_chan,"logoFile",
			json_object_new_string((char*)chan->logoFile));

		/* json array with the programme */
		json_object* j_prog_array = json_object_new_array();

		int p = 0;
		for(p = 0; p < chan->programmeList->len; p++){
			/* get the programme */
			programme* prog = &g_array_index(chan->programmeList,programme,p);

			/* create the programme js object */
			json_object* j_prog = json_object_new_object();

			json_object_object_add(j_prog,"title",
				json_object_new_string(prog->title->str));
			json_object_object_add(j_prog,"description",
				json_object_new_string(prog->description->str));
			json_object_object_add(j_prog,"category",
				json_object_new_string(prog->category->str));
			json_object_object_add(j_prog,"start",
				json_object_new_int(prog->start));
			json_object_object_add(j_prog,"stop",
				json_object_new_int(prog->stop));
			json_object_object_add(j_prog,"productionDate",
				json_object_new_int(prog->productionDate));

			/* add the programme to the programmelist json array */
			json_object_array_add(j_prog_array,j_prog);
		}

		/* add programmelist json array */
		json_object_object_add(j_chan,"programmeList",j_prog_array);

		/* add the channel to the result json array */
		json_object_array_add(j_chan_array,j_chan);
	}

	/* finally pass j_object to result string */
	result = g_string_new(json_object_to_json_string(j_chan_array));

	return result;
}

/*
	Returns the id (list index) of the current channel
	@return 	the list index of the current channel
*/
int iptvx_epg_get_current_channel_id(){
	return iptvx_epg_current_channel;
}

/*
	Sets the current channel id
	@param 		channelId 		the channel to set as current
*/
void iptvx_epg_set_current_channel_id(int channelId){
	/* don't set the val if it exceeds the list */
	if(channelId < list->len){
		iptvx_epg_current_channel = channelId;
	}
}

/*
	Gets the current channel
	@return 		ptr to the current channel struct
*/
channel* iptvx_epg_get_current_channel(){
	channel* result = NULL;

	int c = 0;
	for(c = 0; c < list->len; c++){
		if(c == iptvx_epg_current_channel){
			result = &g_array_index(list,channel,c);
		}
	}

	return result;
}

/* 
	Gets the default channel as defined in config
	@return 		ptr to the channel struct
*/
channel* iptvx_epg_get_default_channel(){
	channel* result;

	int c = 0;
	for(c = 0; c < list->len; c++){
		channel* current = &g_array_index(list,channel,c);
		if(current->isDefault == true){
			result = current;
			iptvx_epg_current_channel = c;
		}
	}

	return result;
}

/* takes an XMLTV date string, converts it to UNIX 
	timestamp and returns that timestamp as long */
long iptvx_epg_get_xmltv_timestamp(GString* xmltvDate){
	long result = 0;

	struct tm timeStruct;

	strptime(xmltvDate->str,"%Y%m%d%H%M%S %z",&timeStruct);
	result = timegm(&timeStruct)-3600;

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
	if(doc == NULL){
		/* output an error when parser failed */
		printf("Unable to parse XMLTV string.\n");
	}else{
		/* get the root element */
		cur = xmlDocGetRootElement(doc);

		printf("docroot picked up\n");

		/* get all programme nodes */
		for(progNode = cur->children; progNode != NULL; progNode = progNode->next){
			if(xmlStrcmp(progNode->name,"programme")==0){
				/* create prorgramme struct */
				programme prog;

				/* get start and stop as string */
				GString* startTime = g_string_new(xmlGetProp(progNode,"start"));
				GString* stopTime = g_string_new(xmlGetProp(progNode,"stop"));
				prog.start = iptvx_epg_get_xmltv_timestamp(startTime);
				prog.stop = iptvx_epg_get_xmltv_timestamp(stopTime);

				/* initialise properties */
				prog.title = g_string_new("");
				prog.description = g_string_new("");
				prog.category = g_string_new("");
				prog.productionDate = 0;

				/* get programme information */
				if(progNode->children != NULL){
					for(valNode = progNode->children; valNode != NULL; valNode = valNode->next){
						if(xmlStrcmp(valNode->name,"title")==0){
							if(valNode->children != NULL){
								prog.title = g_string_new(valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"desc")==0){
							if(valNode->children != NULL){
								prog.description = g_string_new(valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"category")==0){
							/* only take first category found */
							if(prog.category->len == 0 && valNode->children != NULL){
								prog.category = g_string_new(valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"date")==0){
							if(valNode->children != NULL){
								GString* dateVal = g_string_new(valNode->children->content);
								prog.productionDate = g_ascii_strtoll(dateVal->str,NULL,0);
							}
						}
					}

					/* flush programme into result */
					g_array_append_val(result,prog);
				}
			}
		}
	}

	/* cleanup global libxml */
	xmlCleanupParser();

	return result;
}

/* loads the epg of the defined channel */
void iptvx_epg_load_channel(channel* current){
	/* create EPG url for today */
	char epg_url[256];
	time_t now = time(NULL);
	struct tm *t = localtime(&now);
	strftime(epg_url,sizeof(epg_url)-1,(char*)current->epgUrl,t);

	/* ensure the cache path exists */
	struct stat st = {0};

	/* if cache folder needs to be created, 
		we also need to create epg and logo */
	if (stat("data", &st) == -1) {
    	mkdir("data", 0700);
    	mkdir("data/epg", 0700);
    	mkdir("data/logo", 0700);
	}

	/* but also ensure to create each of the others */
	if (stat("data/epg", &st) == -1) {
		mkdir("data/epg", 0700);
	}

	/* for logo as well */
	if (stat("data/logo", &st) == -1) {
		mkdir("data/logo", 0700);
	}

	/* define the cache file for epg */
	char* cacheFile;

	/* check if the url is defined and use 
		the filename defined in there */
	if(current->epgUrl->len > 1){
		cacheFile = g_strrstr(epg_url,"/")+1;
	}else{
		/* use the name of the epg file if defined */
		if(current->epgFile->len > 1){
			/* define disk epg file */
			char epg_file[256];
			time_t now = time(NULL);
			struct tm *t = localtime(&now);
			strftime(epg_file,sizeof(epg_file)-1,(char*)current->epgFile,t);

			/* set cache file to disk epg file */
			cacheFile = epg_file;
		}else{
			/* if not, define it from the name */
			cacheFile = (char*)current->name;
		}
	}

	/* define the full path of the epg file */
	char* cacheFilePath = g_strjoin("","data/epg/",cacheFile,NULL);

	GString* xmltv = g_string_new(NULL);
	if(!util_file_exists(cacheFilePath)){
		/* fetch url if defined */
		if(current->epgUrl->len > 0){
			/* file doesn't exist, we need to fetch it */
			xmltv = util_download_string(epg_url);	
		}

		/* fetch xmltv epg from shell if defined */
		if(current->epgShell->len > 0){
			xmltv = util_shell_exec(current->epgShell);
		}
		
		/* finally flush the xmltv to disk cache */
		file_put_contents(g_string_new(cacheFilePath),xmltv);
	}else{
		/* file exists, we'll get it */
		xmltv = file_get_contents(g_string_new(cacheFilePath));
	}

	/* parse the programme list from the xmltv data */
	current->programmeList = iptvx_epg_get_programmelist(xmltv);

	/* free the xmltv string and its mem */
	g_string_free(xmltv,false);
}

/* initiates the epg load for each channel */
int iptvx_epg_load(void* nothing){
	int c = 0;
	for(c = 0; c < list->len; c++){
		/* get this channel */
		channel* current = &g_array_index(list,channel,c);

		/* start the thread to capture xmltv epg */
		iptvx_epg_load_channel(current);

		/* update percentage status */
		iptvx_epg_percentage_loaded = ((c / list->len) * 100);
		epgStatusUpdateCallback(&iptvx_epg_percentage_loaded);
	}

	/* update status indicators */
	iptvx_epg_ready = true;
	iptvx_epg_percentage_loaded = 100;
	epgStatusUpdateCallback(&iptvx_epg_percentage_loaded);

	return 0;
}

/*
   Initialises EPG and loads XMLTV files
   @param      cfg                     Config struct from libconfig holding channel config
   @param      statusUpdateCallback    Callback to call when status changes (e.g. finish)
*/
bool iptvx_epg_init(config_t* cfg,void (*statusUpdateCallback)(void*)){
	list = g_array_new (false,false,sizeof(channel));

	/* init current channel */
	iptvx_epg_current_channel = 0;

	/* assign the local callback function */
	epgStatusUpdateCallback = statusUpdateCallback;

	/* init status indicators */
	iptvx_epg_ready = false;
	iptvx_epg_percentage_loaded = 0;

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

			current.name = g_string_new("");
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
			current.url = g_string_new("");
			if (config_setting_lookup_string(element,"url",(const char**)&channelUrl)) {
            	current.url = channelUrl;
            }

            GString* channelUrlShell = g_string_new("");
			current.urlShell = g_string_new("");
			if (config_setting_lookup_string(element,"urlShell",(const char**)&channelUrlShell)) {
            	current.urlShell = channelUrlShell;
            }

            GString* logoFile = g_string_new("");
			current.logoFile = g_string_new("");
			if (config_setting_lookup_string(element,"logoFile",(const char**)&logoFile)) {
            	current.logoFile = logoFile;
            }

            GString* epgUrl = g_string_new("");
			current.epgUrl = g_string_new("");
			if (config_setting_lookup_string(element,"epgUrl",(const char**)&epgUrl)) {
            	current.epgUrl = epgUrl;
            }

            GString* epgFile = g_string_new("");
			current.epgFile = g_string_new("");
			if (config_setting_lookup_string(element,"epgFile",(const char**)&epgFile)) {
            	current.epgFile = epgFile;
            }

            GString* epgShell = g_string_new("");
			current.epgShell = g_string_new("");
			if (config_setting_lookup_string(element,"epgShell",(const char**)&epgShell)) {
            	current.epgShell = epgShell;
            }

            /* append channel to list */
            g_array_append_val(list,current);
		}

		// all channels parsed, launch epg load
		epg_thread = SDL_CreateThread(iptvx_epg_load,"");
	}else{
		/* output and error when channels not present */
		printf("Error getting channels from config\n");
	}
}