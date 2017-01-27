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
   GString* epgUrl;
   GString* logoUrl;
   GArray* programmeList;
} typedef channel;

bool iptvx_epg_init(config_t* cfg);
channel* iptvx_epg_get_default_channel();
void iptvx_epg_load_channel(GString* channelName);
void iptvx_epg_load();

#endif