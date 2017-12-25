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

#ifndef	EPG_H
#define EPG_H

#include <stdbool.h>
#include <glib.h>

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

/*
   Sets the data directory to store epg cache and logo files
   @param      data_dir       path of the directory for the data
*/
void iptvx_epg_set_data_dir(char* data_dir);

/*
   Defines after how many days the files will be deleted
   @param         days        int defining the days
*/
void iptvx_epg_set_expiry_days(int days);

/*
   Defines how many hours of programme 
   to be stored in the epg data
   @param         hours          int defining the hours
*/
void iptvx_epg_set_storage_hours(int hours);

/*
   Returns all channel and epg info as JSON string
   @return     JSON string with all EPG info
*/
GString* iptvx_epg_get_json();

/*
   Returns the id (list index) of the current channel
   @return  the list index of the current channel
*/
int iptvx_epg_get_current_channel_id();

/*
   Sets the current channel id
   @param      channelId      the channel to set as current
*/
void iptvx_epg_set_current_channel_id(int channelId);

/*
   Gets the current channel
   @return     ptr to the current channel struct
*/
channel* iptvx_epg_get_current_channel();

/*
   Initialises EPG and loads XMLTV files
   @param      cfg                     Config struct from libconfig holding channel config
   @param      statusUpdateCallback    Callback to call when status changes (e.g. finish)
*/
bool iptvx_epg_init(config_t* cfg,void (*statusUpdateCallback)(void*));

/* 
   Gets the default channel as defined in config
   @return     ptr to the channel struct
*/
channel* iptvx_epg_get_default_channel();

/*
   Loads the defined channel epg for the defined time
   @param            current           current channel to load
   @param            epg_time          time to get epg for
*/
void iptvx_epg_load_channel(channel* current, time_t epg_time);

/* 
   initiates the epg load for each channel 
*/
int iptvx_epg_load(void* nothing);

#endif