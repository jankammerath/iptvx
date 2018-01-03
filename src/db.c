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
#include <string.h>
#include <glib.h>
#include "channel.h"
#include "recording.h"

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
   Checks if a programme already exists in the database and returns it's id
   @param         chanid         id of the channel the programme is on
   @param         prog           programme struct with the actual data
   @return                       db id of the programme or -1 if not exists
*/
long iptvx_db_get_programme_id(int chanid, programme* prog){
   long result = -1;

   /* prepare the sql statement */
   char* sql = sqlite3_mprintf("SELECT programmeid FROM programme "
                              "WHERE programmetitle = '%q' "
                              "AND programmestart = %lld "
                              "AND programmestop = %lld "
                              "AND programmechannelid = %lld",
                              prog->title->str, prog->start, 
                              prog->stop, chanid);

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   while (sqlite3_step(stmt) != SQLITE_DONE) {
      int i;
      int column_count = sqlite3_column_count(stmt);
      for(i = 0; i < column_count; i++){
         result = sqlite3_column_int64(stmt, i);
      }
   }

   return result;
}

/*
   Gets the id of a category by it's name. 
   If it doesn't exist, it'll be created
   @param         category_name     name of the category to get id for
   @return                          id of the category
*/
long iptvx_db_get_category_id(char* category_name){
   long result = 0;

   /* prepare the sql statement */
   char* sql = sqlite3_mprintf("SELECT categoryid FROM category "
                              "WHERE categoryname = '%q'", category_name);

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
   while (sqlite3_step(stmt) != SQLITE_DONE) {
      int i;
      int column_count = sqlite3_column_count(stmt);
      for(i = 0; i < column_count; i++){
         result = sqlite3_column_int64(stmt, i);
      }
   }

   /* make a gstring out of the name */
   GString* catname = g_string_new(category_name);

   if(result == 0 && catname->len > 0){
      char* insert_sql = sqlite3_mprintf("INSERT INTO category "
               "(categoryname) VALUES ('%q')", category_name);
      sqlite3_exec(db,insert_sql,NULL,NULL,NULL);
      sqlite3_free(insert_sql);
      result = sqlite3_last_insert_rowid(db);
   }

   /* finally free the gstring again */
   g_string_free(catname,false);

   return result;
}

/*
   Inserts a channel's programme into the db if it does not yet exist
   @param         chanid      db id of the channel to add programme to
   @param         prog        programme struct with the programme
*/
void iptvx_db_insert_programme(int chanid, programme* prog){
   /* check if the programme is already in the database */
   long exist_prog_id = iptvx_db_get_programme_id(chanid,prog);
   
   /* get the category id from the database */
   long exist_category_id = iptvx_db_get_category_id(prog->category->str);

   if(exist_prog_id >= 0){
      /* programme already exists in db, just update it */
      char* update_sql = sqlite3_mprintf("UPDATE programme "
                  "SET programmedescription = '%q', "
                  "programmecategoryid = %lld, "
                  "programmeproductiondate = %lld "
                  "WHERE programmeid = %lld",
                  prog->description->str, exist_category_id,
                  prog->productionDate, exist_prog_id);
      sqlite3_exec(db,update_sql,NULL,NULL,NULL);
      sqlite3_free(update_sql);
   }else{
      /* the programme is not in the database, insert it */
      char* insert_sql = sqlite3_mprintf(
            "INSERT INTO programme (programmetitle, programmedescription, "
            "programmecategoryid, programmestart, programmestop, "
            "programmeproductiondate, programmechannelid) VALUES "
            "('%q','%q',%lld,%lld,%lld,%lld,%lld)",
            prog->title->str, prog->description->str,
            exist_category_id, prog->start, prog->stop, 
            prog->productionDate, chanid);
      sqlite3_exec(db,insert_sql,NULL,NULL,NULL);
      sqlite3_free(insert_sql);
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
      char* errmsg;
      char* insert_sql = sqlite3_mprintf("INSERT INTO channel "
               "(channelname) VALUES ('%q')" ,chan->name->str);
      sqlite3_exec(db,insert_sql,NULL,NULL,&errmsg);
      sqlite3_free(insert_sql);
   }

   /* get the id of the channel */
   int channelid = -1;
   char* get_id_sql = sqlite3_mprintf("SELECT channelid FROM channel "
                        "WHERE channelname = '%q'",chan->name->str);
   sqlite3_stmt *getid_stmt;
   sqlite3_prepare_v2(db,get_id_sql, -1, &getid_stmt, NULL);
   while(sqlite3_step(getid_stmt) != SQLITE_DONE){
      channelid = sqlite3_column_int64(getid_stmt,0);
   }

   /* make sure the channel exists and has an id */
   if(channelid != -1){
      int p = 0;
      for(p = 0;p < chan->programmeList->len; p++){
         programme* prog = &g_array_index(chan->programmeList,programme,p);
         iptvx_db_insert_programme(channelid,prog);
      }
   }

   /* finalise statement */
   sqlite3_finalize(stmt);
   sqlite3_free(sql);
}

/*
   Updates the recording in the database
   @param         reclist        array with all current recordings
*/
void iptvx_db_update_recording(GArray* reclist){

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