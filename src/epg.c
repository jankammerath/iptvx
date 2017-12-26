/*

   Copyright 2018   Jan Kammerath

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
#include <dirent.h>
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
	GString* epgChannelId;
	GString* epgShell;
	GString* epgInterval;
	GString* logoFile;
	GString* logoUrl;
	GArray* programmeList;
} typedef channel;

/* the channel list */
GArray* list;

/* path of epg data directory */
GString* epg_data_dir;

/* define how many hours are to be stored */
int iptvx_epg_storage_hours;

/* define after how many days to delete files */
int iptvx_epg_file_expiry_days;

/* defines current channel */
int iptvx_epg_current_channel;

/* indicates if ready */
bool iptvx_epg_ready;
int iptvx_epg_percentage_loaded;

/* status update callback */
void (*epg_status_update_callback)(void*);

/*
	Sets the data directory to store epg cache and logo files
	@param 		data_dir 		path of the directory for the data
*/
void iptvx_epg_set_data_dir(char* data_dir){
	epg_data_dir = g_string_new(data_dir);
}

/*
	Defines after how many days the files will be deleted
	@param 			days 			int defining the days
*/
void iptvx_epg_set_expiry_days(int days){
	iptvx_epg_file_expiry_days = days;
}

/*
	Defines how many hours of programme 
	to be stored in the epg data
	@param 			hours 			int defining the hours
*/
void iptvx_epg_set_storage_hours(int hours){
	iptvx_epg_storage_hours = hours;
}

/*
	Cleans local files when they are outdated
*/
void iptvx_epg_clean_files(){
	DIR *d;
	struct dirent *dir;
	char* epgCacheDir = g_strjoin("/",epg_data_dir->str,"epg",NULL);
	d = opendir(epgCacheDir);
	if(d){
		while ((dir = readdir(d)) != NULL){
			/* make sure its not . or .. as filename */
			if(g_strcmp0(dir->d_name,".") != 0 && g_strcmp0(dir->d_name,"..") != 0){
				char* epgFileName = g_strjoin("/",epgCacheDir,dir->d_name,NULL);

				struct stat attrib;
    			stat(epgFileName, &attrib);
    			char modified_date[10];
    			strftime(modified_date, 10, "%d-%m-%y", gmtime(&(attrib.st_ctime)));

    			double fileAge = difftime(time(NULL),attrib.st_ctime);
    			if(fileAge > (iptvx_epg_file_expiry_days*3600)){
    				/* file is outdated and needs to go */
    				util_delete_file(epgFileName);
    			}
			}
    	}
	    closedir(d);
	}
}

/*
	Returns all epg data in the channel and 
	programme structs within a GArray
	@return 		GArray with all epg data
*/
GArray* iptvx_epg_get_data(){
	return list;
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

	/* free the offset gstring */
	g_string_free(gstr_tz_offset,true);

	return result;
}

/*
	Gets the stop time of the last programme stored
	@param 			channel 		ptr to the channel to check
	@return 						int value with max epoch stored
*/
int iptvx_epg_get_max_time(channel* current){
	int result = 0;

	int p;
	for(p=0;p<current->programmeList->len;p++){
		programme* cur_prog = &g_array_index(current->programmeList,programme,p);	

		if(cur_prog->stop > result){
			result = cur_prog->stop;
		}	
	}	

	return result;
}

/*
	Checks if the current epg already contains
	the programme passed as parameter
	@param 			current 		channel to check programme of
	@param 			programme 		programme to check for existance
	@return 						true when exists otherwise false
*/
bool iptvx_epg_contains_programme(channel* current, programme* prog){
	bool result = false;

	int p;
	for(p=0;p<current->programmeList->len;p++){
		/* get the programme from the list */
		programme* cur_prog = &g_array_index(current->programmeList,programme,p);	

		/* check if its the same as the one provided */
		if(cur_prog->start == prog->start && cur_prog->stop == prog->stop){
			result = true;
		}	
	}

	return result;
}

/* parses the programmes from xmltv and returns it as 
	a GArray holding programme structs */
GArray* iptvx_epg_get_programmelist(GString* xmltv, channel* chan){
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
			/* defines whether this node should be parsed or not */
			bool parseCurrentNode = false;
			if(xmlStrcmp(progNode->name,"programme")==0){
				if(chan->epgChannelId->len > 0){
					/* there is an epg channel id defined in the channel,
						so we need to check if this is a programme node
						and the node actually matches the epgChannelId */
					GString* xmlChannelId = g_string_new(xmlGetProp(progNode,"channel"));
					if(g_strcmp0(xmlChannelId->str,chan->epgChannelId->str)==0){
						/* the channel attribute in the xml matches 
							the config epgChannelId */
						parseCurrentNode = true;
					}
				}else{
					parseCurrentNode = true;
				}
			}

			/* start parsing the node if its the correct one*/
			if(parseCurrentNode == true){
				/* create prorgramme struct */
				programme prog;

				/* get start and stop as string */
				GString* startTime = g_string_new(xmlGetProp(progNode,"start"));
				GString* stopTime = g_string_new(xmlGetProp(progNode,"stop"));
				prog.start = iptvx_epg_get_xmltv_timestamp(startTime);
				prog.stop = iptvx_epg_get_xmltv_timestamp(stopTime);

				/* free the date text strings */
				g_string_free(startTime,true);
				g_string_free(stopTime,true);

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
								g_string_free(prog.title,true);
								prog.title = g_string_new((char*)valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"desc")==0){
							if(valNode->children != NULL){
								g_string_free(prog.description,true);
								prog.description = g_string_new((char*)valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"category")==0){
							/* only take first category found */
							if(prog.category->len == 0 && valNode->children != NULL){
								g_string_free(prog.category,true);
								prog.category = g_string_new((char*)valNode->children->content);
							}
						}if(xmlStrcmp(valNode->name,"date")==0){
							if(valNode->children != NULL){
								GString* dateVal = g_string_new((char*)valNode->children->content);
								prog.productionDate = g_ascii_strtoll(dateVal->str,NULL,0);
								g_string_free(dateVal,true);
							}
						}
					}

					/* flush programme into result */
					g_array_append_val(result,prog);
				}
			}
		}

		/* free the xml doc */
		xmlFreeDoc(doc);
	}

	/* cleanup global libxml */
	xmlCleanupParser();

	return result;
}

/*
	Loads the logo for a channel and stores it on disk
	@param 			current 			current channel to load logo for
*/
void iptvx_epg_load_channel_logo(channel* current){
	/* check if the logo file needs to be downloaded 
		which is the case when a logoUrl setting is set */
	if(current->logoUrl->len > 0){
		GString* logoFile;
		if(current->logoFile->len > 0){
			/* there is a logo file defined in config */
			logoFile = g_string_new(current->logoFile->str);
		}else{
			/* there is no logo file defined, check for url filename first */
			if(current->logoUrl->len > 0){
				logoFile = g_string_new(g_strrstr(current->logoUrl->str,"/")+1);
			}else{
				/* there is no url defined, so we use the channel name */
				logoFile = g_string_new(g_strconcat(current->name->str,".logo"));
			}
		}

		/* define the full path of the logo file */
		char* logoCacheDir = g_strjoin("/",epg_data_dir->str,"logo",NULL);
		char* logoFilePath = g_strjoin("/",logoCacheDir,logoFile->str,NULL);		

		/* there is a logo file defined, so we download it
			if it has not already been in the past */
		if(!util_file_exists(logoFilePath)){
			/* file does not exist on disk, download it */
			util_download_file(current->logoUrl->str,logoFilePath);
		}
	}
}

/*
   Loads the defined channel epg for the defined time
   @param            current           	current channel to load
   @param            epg_time          	time to get epg for
   @param 			 overwrite_cache	true when cache should be ignored
*/ 
void iptvx_epg_load_channel(channel* current, time_t epg_time, bool overwrite_cache){
	/* load the logo file for this channel */
	iptvx_epg_load_channel_logo(current);

	/* create EPG url for today */
	char epg_url[256];

	/* convert epg time to local time 
		and apply time pattern to url */
	struct tm *t = localtime(&epg_time);
	strftime(epg_url,sizeof(epg_url)-1,current->epgUrl->str,t);

	/* define the cache file for epg */
	char* cacheFile;

	/* check if the url is defined and use 
		the filename defined in there */
	if(current->epgUrl->len > 1 && current->epgFile->len == 0){
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
	char* epgCacheDir = g_strjoin("/",epg_data_dir->str,"epg",NULL);
	char* cacheFilePath = g_strjoin("/",epgCacheDir,cacheFile,NULL);

	/* free cache dir var and cache file var */
	free(epgCacheDir);
	
	/* check if cache should be ignored which means 
		existing file will be deleted */
	if(overwrite_cache){
		if(util_file_exists(cacheFilePath)){
			util_delete_file(cacheFilePath);
		}
	}

	GString* xmltv = g_string_new(NULL);
	if(!util_file_exists(cacheFilePath)){
		/* fetch url if defined */
		if(current->epgUrl->len > 0){
			/* free string before re-assigning */
			g_string_free(xmltv,true);

			/* file doesn't exist, we need to fetch it */
			xmltv = util_download_string(epg_url);	
		}

		/* fetch xmltv epg from shell if defined */
		if(current->epgShell->len > 0){
			/* free string before re-assigning */
			g_string_free(xmltv,true);

			/* fetch new string from shell exec */
			xmltv = util_shell_exec(current->epgShell);
		}
		
		/* finally flush the xmltv to disk cache
			when the data is not empty */
		if(xmltv != NULL && xmltv->len > 0){
			GString* gs_cache_file_path = g_string_new(cacheFilePath);
			file_put_contents(gs_cache_file_path,xmltv);
			g_string_free(gs_cache_file_path,true);
		}
	}else{
		/* file exists, we'll get it */
		GString* gs_cache_file_path = g_string_new(cacheFilePath);
		xmltv = file_get_contents(gs_cache_file_path);
		g_string_free(gs_cache_file_path,true);
	}

	/* parse the programme list from the xmltv data */
	GArray* plist = iptvx_epg_get_programmelist(xmltv,current);

	/* append the programme data to the existing array */
	int p;
	for(p = 0; p<plist->len;p++){
		programme* new_prog = &g_array_index(plist,programme,p);

		/* only insert programme when not yet present */
		if(!iptvx_epg_contains_programme(current,new_prog)){
			g_array_append_val(current->programmeList,*new_prog);
		}
	}

	/* free the temporary programme array */
	g_array_free(plist,true);

	/* free the xmltv string and its mem */
	g_string_free(xmltv,true);
}

/* 
	initiates the epg load for each channel 
*/
int iptvx_epg_load(void* nothing){
	/* delete any trash that might still be on disk */
	iptvx_epg_clean_files();

	int c = 0;
	for(c = 0; c < list->len; c++){
		/* get this channel */
		channel* current = &g_array_index(list,channel,c);

		/* start the thread to capture xmltv epg */
		iptvx_epg_load_channel(current,time(NULL)-18000,false);

		/* update percentage status */
		iptvx_epg_percentage_loaded = (int)((float)((float)c / (float)list->len) * 100);
		epg_status_update_callback(&iptvx_epg_percentage_loaded);
	}

	/* update status indicators */
	iptvx_epg_ready = true;
	iptvx_epg_percentage_loaded = 100;
	epg_status_update_callback(&iptvx_epg_percentage_loaded);

	/* process daily epg files */
	int additional_epg_data_count = 0;
	for(c = 0; c < list->len; c++){
		channel* current = &g_array_index(list,channel,c);
		if(g_strcmp0(current->epgInterval->str,"daily")==0){
			/* load epg data for each day until we hit 
				the storage limit defined in the config */
			int days = (int)((float)iptvx_epg_storage_hours/(float)24);

			int d;
			for(d = 0; d < days; d++){
				iptvx_epg_load_channel(current,time(NULL)+(d*86400),false);
				additional_epg_data_count++;
			}
		}else{
			/* this is a static file with a variable amount of 
				hours of programme included. We update the file 
				when the cached one does not include the amount 
				of hours defined in config */
			int max_epg_time = time(NULL)+(iptvx_epg_storage_hours*3600);
			int max_stored = iptvx_epg_get_max_time(current);

			if(max_stored < max_epg_time){
				/* fetch epg data again */
				iptvx_epg_load_channel(current,time(NULL)-18000,true);
			}
		}
	}

	/* fire up callback when additional data 
		was captured for the coming days */
	epg_status_update_callback(&iptvx_epg_percentage_loaded);

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
		/* free original val */
		g_string_free(result,true);

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
	epg_status_update_callback = statusUpdateCallback;

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
			current.logoUrl = iptvx_epg_config_get_string(element,"logoUrl");
			current.logoFile = iptvx_epg_config_get_string(element,"logoFile");
			current.epgUrl = iptvx_epg_config_get_string(element,"epgUrl");
			current.epgFile = iptvx_epg_config_get_string(element,"epgFile");
			current.epgChannelId = iptvx_epg_config_get_string(element,"epgChannelId");
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