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
	GString* epgInterval;
	GString* logoFile;
	GArray* programmeList;
} typedef channel;

/* the channel list */
GArray* list;

/* define how many hours are to be stored */
int iptvx_epg_storage_hours;

/* defines current channel */
int iptvx_epg_current_channel;

/* indicates if ready */
bool iptvx_epg_ready;
int iptvx_epg_percentage_loaded;

/* status update callback */
void (*epgStatusUpdateCallback)(void*);

/*
	Defines how many hours of programme 
	to be stored in the epg data
	@param 			hours 			int defining the hours
*/
void iptvx_epg_set_storage_hours(int hours){
	iptvx_epg_storage_hours = hours;
}

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
			json_object_new_string(chan->name->str));
		json_object_object_add(j_chan,"logoFile",
			json_object_new_string(chan->logoFile->str));

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

			if(prog->description->len > 0){
				json_object_object_add(j_prog,"description",
						json_object_new_string(prog->description->str));
			}

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

	/* check if the current channel has an empty URL 
		and requires us to fetch its URL from a script 
		that is defined in the 'urlShell' variable */
	if(result != NULL){
		if(result->url->len == 0 && result->urlShell->len > 0){
			GString* shell_url = util_shell_exec(result->urlShell);
			result->url->str = shell_url->str;
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

	/* only parse the time and fix the timezone separately.
		we'll be using GMT time here and only calculate 
		the offset manually so that the epoch time is created
		out of the GMT time of the programme */
	strptime(xmltvDate->str,"%Y%m%d%H%M%S",&timeStruct);
	GString* gstr_tz_offset = util_substr(xmltvDate,16,2);
	int tz_offset = g_ascii_strtoll(gstr_tz_offset->str,NULL,0);

	/* check if negative and multiply offset by -1 */
	if(xmltvDate->str[15] == '+'){
		tz_offset = tz_offset * -1;
	}

	/* add the timezone offset to the hour */
	timeStruct.tm_hour = timeStruct.tm_hour+tz_offset;

	/* make epoch from GMT tm struct */
	result = timegm(&timeStruct);

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
		printf("Unable to parse XMLTV string:\n%s\n",xmltv->str);
	}else{
		/* get the root element */
		cur = xmlDocGetRootElement(doc);

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
								prog.title = g_string_new((char*)valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"desc")==0){
							if(valNode->children != NULL){
								prog.description = g_string_new((char*)valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"category")==0){
							/* only take first category found */
							if(prog.category->len == 0 && valNode->children != NULL){
								prog.category = g_string_new((char*)valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"date")==0){
							if(valNode->children != NULL){
								GString* dateVal = g_string_new((char*)valNode->children->content);
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

/*
   Loads the defined channel epg for the defined time
   @param            current           current channel to load
   @param            epg_time          time to get epg for
*/
void iptvx_epg_load_channel(channel* current, time_t epg_time){
	/* create EPG url for today */
	char epg_url[256];

	struct tm *t = localtime(&epg_time);
	strftime(epg_url,sizeof(epg_url)-1,current->epgUrl->str,t);

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
			strftime(epg_file,sizeof(epg_file)-1,current->epgFile->str,t);

			/* set cache file to disk epg file */
			cacheFile = epg_file;
		}else{
			/* if not, define it from the name */
			cacheFile = current->name->str;
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
		
		/* finally flush the xmltv to disk cache
			when the data is not empty */
		if(xmltv != NULL && xmltv->len > 0){
			file_put_contents(g_string_new(cacheFilePath),xmltv);
		}
	}else{
		/* file exists, we'll get it */
		xmltv = file_get_contents(g_string_new(cacheFilePath));
	}

	/* parse the programme list from the xmltv data */
	GArray* plist = iptvx_epg_get_programmelist(xmltv);

	/* append the programme data to the existing array */
	int p;
	for(p = 0; p<plist->len;p++){
		programme* new_prog = &g_array_index(plist,programme,p);
		g_array_append_val(current->programmeList,*new_prog);
	}

	/* free the xmltv string and its mem */
	g_string_free(xmltv,false);
}

/* 
	initiates the epg load for each channel 
*/
int iptvx_epg_load(void* nothing){
	int c = 0;
	for(c = 0; c < list->len; c++){
		/* get this channel */
		channel* current = &g_array_index(list,channel,c);

		/* start the thread to capture xmltv epg */
		iptvx_epg_load_channel(current,time(NULL));

		/* update percentage status */
		iptvx_epg_percentage_loaded = (int)((float)((float)c / (float)list->len) * 100);
		epgStatusUpdateCallback(&iptvx_epg_percentage_loaded);
	}

	/* update status indicators */
	iptvx_epg_ready = true;
	iptvx_epg_percentage_loaded = 100;
	epgStatusUpdateCallback(&iptvx_epg_percentage_loaded);

	return 0;
}

/*
	Reads a string config value from setting
	@param 			element 			the config element to read from
	@param 			setting_name 		the config setting to read
	@return 							the string result or empty string
*/
GString* iptvx_epg_config_get_string(config_setting_t* element, char* setting_name){
	GString* result = g_string_new("");

	char* config_val = "";
	if (config_setting_lookup_string(element,setting_name,
							(const char**)&config_val)) {
		/* create GString with result value */
		result = g_string_new(config_val);
	}

	return result;
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

			/* get channel default setting */
            int channelDefault = 0;
            current.isDefault = false;
			if (config_setting_lookup_bool(element,"default",&channelDefault)) {
            	current.isDefault = (bool)channelDefault;
            }

            /* get numerous string setting values */
			current.name = iptvx_epg_config_get_string(element,"name");
			current.url = iptvx_epg_config_get_string(element,"url");
			current.urlShell = iptvx_epg_config_get_string(element,"urlShell");
			current.logoFile = iptvx_epg_config_get_string(element,"logoFile");
			current.epgUrl = iptvx_epg_config_get_string(element,"epgUrl");
			current.epgFile = iptvx_epg_config_get_string(element,"epgFile");
			current.epgShell = iptvx_epg_config_get_string(element,"epgShell");
			current.epgInterval = iptvx_epg_config_get_string(element,"epgInterval");

			/* initialise programme array */
			current.programmeList = g_array_new(false,false,sizeof(programme));

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