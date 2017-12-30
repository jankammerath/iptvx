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
#include <glib.h>

/*
   Initialises and opens the database
   @param         filename       file path of the db file
*/
void iptvx_db_init(char* filename);

/*
   Updates the database with epg data
   @param      epg      array with all epg data
*/
void iptvx_db_update(GArray* epg);

/*
   Closes the database
*/
void iptvx_db_close();