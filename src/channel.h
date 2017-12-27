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

#pragma once

#ifndef  CHANNEL_H
#define  CHANNEL_H

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

#endif