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
#include <stdbool.h>
#include <sqlite3.h>
#include <glib.h>
#include "channel.h"

/* db handle */
sqlite3* db;

/*
   Initialises and opens the database
   @param         filename       file path of the db file
*/
void iptvx_db_init(char* filename){
   /* open the application database */
   int rc = sqlite3_open(filename, &db); 

   if(rc){
      /* print an error message to stderr */
      fprintf(stderr, "Database error: %s\n", 
               sqlite3_errmsg(db));
   }
}

/*
   Inserts a channel into the db if it doesn't exist
   @param         chan        the channel struct
*/
void iptvx_db_insert_channel(channel* chan){
   /* indicates whether channel exists or not */
   bool channel_exists;

   /* set to false by default */
   channel_exists = false;

   /* prepare the sql statement */
   char* sql = sqlite3_mprintf("SELECT channelname FROM channel "
                     "WHERE channelname = '%q'",chan->name->str);

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   while (sqlite3_step(stmt) != SQLITE_DONE) {
      int i;
      int column_count = sqlite3_column_count(stmt);
      for(i = 0; i < column_count; i++){
         if(sqlite3_column_type(stmt, i) == SQLITE3_TEXT){
            /* check if the column has the channel name */
            if(g_strcmp0(sqlite3_column_text(stmt, i),chan->name->str)==0){
               /* this means the channel is already there */
               channel_exists = true;
            }
         }
      }
   }

   /* insert if it doesn't exist */
   if(channel_exists == false){
      char* insert_sql = sqlite3_mprintf("INSERT INTO channel "
               "(channelname) VALUES ('%q')" ,chan->name->str);
      sqlite3_exec(db,insert_sql,NULL,NULL,NULL);
      sqlite3_free(insert_sql);
   }

   /* finalise statement */
   sqlite3_finalize(stmt);
   sqlite3_free(sql);
}

/*
   Updates the database with epg data
   @param      epg      array with all epg data
*/
void iptvx_db_update(GArray* epg){
   /* go through all channels and insert them
      into the database if they do not already exist */
   int c = 0;
   for(c = 0; c < epg->len; c++){
      /* get the channel from the list */
      channel* chan = &g_array_index(epg,channel,c);
      iptvx_db_insert_channel(chan);
   }
}

/*
   Closes the database
*/
void iptvx_db_close(){
   sqlite3_close(db);
}