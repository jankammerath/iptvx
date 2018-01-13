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
   while (sqlite3_step(stmt) == SQLITE_ROW) {
      int i;
      int column_count = sqlite3_column_count(stmt);
      for(i = 0; i < column_count; i++){
         result = sqlite3_column_int64(stmt, i);
      }
   }

   sqlite3_finalize(stmt);

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
   while (sqlite3_step(stmt) == SQLITE_ROW) {
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

      result = sqlite3_last_insert_rowid(db);
   }

   sqlite3_finalize(stmt);

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
   }
}

/*
   Returns the db id of a given channel name if exists
   @param         channelname    name of the channel to get id for
   @return                       db id of the channel or -1 if not exists 
*/
long iptvx_db_get_channel_id(char* channelname){
   int channelid = -1;
   
   char* get_id_sql = sqlite3_mprintf("SELECT channelid FROM channel "
                        "WHERE channelname = '%q'",channelname);
   
   sqlite3_stmt *getid_stmt;
   sqlite3_prepare_v2(db,get_id_sql, -1, &getid_stmt, NULL);
   while(sqlite3_step(getid_stmt) == SQLITE_ROW){
      channelid = sqlite3_column_int64(getid_stmt,0);
   }

   sqlite3_finalize(getid_stmt);

   return channelid;
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
   while (sqlite3_step(stmt) == SQLITE_ROW) {
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
               "(channelname, channelepgupdated) "
               "VALUES ('%q',%lld)", chan->name->str, chan->lastUpdated);
      sqlite3_exec(db,insert_sql,NULL,NULL,&errmsg);
   }else{
      /* do not update the channel if last updated is 0 */
      if(chan->lastUpdated > 0){
         /* update when channel already exists */
         char* update_sql = sqlite3_mprintf("UPDATE channel "
                           "SET channelepgupdated = %lld "
                           "WHERE channelname = '%q'",
                           chan->lastUpdated, chan->name->str);
         sqlite3_exec(db,update_sql,NULL,NULL,NULL);
      }
   }

   /* get the id of the channel */
   int channelid = iptvx_db_get_channel_id(chan->name->str);

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
}

/*
   Gets the database id of an existing recording
   @param         rec      the recording to get id for
   @return                 db id or -1 if it doesn't exist
*/
long iptvx_db_get_recording_id(recording* rec){
   long result = -1;

   /* get the channel id for this channel */
   long channelid = iptvx_db_get_channel_id(rec->channel->str);

   /* build the sql */
   if(channelid != -1){
      char* get_id_sql = sqlite3_mprintf("SELECT recordid FROM record "
                           "WHERE recordstart = %lld "
                           "AND recordstop = %lld "
                           "AND recordchannelid = %lld",
                           rec->start, rec->stop, channelid);
      
      sqlite3_stmt *getid_stmt;
      sqlite3_prepare_v2(db,get_id_sql, -1, &getid_stmt, NULL);
      while(sqlite3_step(getid_stmt) == SQLITE_ROW){
         result = sqlite3_column_int64(getid_stmt,0);
      }
   
      sqlite3_finalize(getid_stmt);
   }

   return result;
}

/*
   Removes a recording from the database
   @param         rec         the recording to remove from database
*/
void iptvx_db_remove_recording(recording* rec){
   /* get the db id of the channel the recording is for */
   long recchannelid = iptvx_db_get_channel_id(rec->channel->str);

   /* prepare the delete statement for the recording */
   char* delete_sql = sqlite3_mprintf("DELETE FROM record "
                        "WHERE recordchannelid = %lld "
                        "AND recordstart = %lld "
                        "AND recordstop = %lld",
                        recchannelid, rec->start,rec->stop);
      sqlite3_exec(db,delete_sql,NULL,NULL,NULL);
}

/*
   Inserts a recording into the database if it doesn't exist
   @param      rec         recording struct with the new recording
*/
void iptvx_db_insert_recording(recording* rec){
   int rec_id = iptvx_db_get_recording_id(rec);

   /* check if the recording does not yet exist */
   if(rec_id == -1){
      /* insert the recording into the database */
      long channelid = iptvx_db_get_channel_id(rec->channel->str);
      char* insert_sql = sqlite3_mprintf("INSERT INTO record "
               "(recordstart, recordstop, recordchannelid, recordtitle) "
               "VALUES (%lld, %lld, %lld, '%q')" ,
               rec->start,rec->stop,channelid,rec->title->str);
      sqlite3_exec(db,insert_sql,NULL,NULL,NULL);
   }
}

/*
   Updates the recording in the database
   @param         reclist        array with all current recordings
*/
void iptvx_db_update_recording(GArray* reclist){
   int r = 0;
   for(r = 0; r<reclist->len; r++){
      recording* rec = &g_array_index(reclist,recording,r);
      iptvx_db_insert_recording(rec);
   }
}

/*
   Returns a list of all recordings from the database
   @return           an array with all recordings in the database
*/
GArray* iptvx_db_get_recording_list(){
   GArray* result = g_array_new(false,false,sizeof(recording));

   char* get_recording_sql = "SELECT c.channelname, r.recordstart, "
                     "r.recordstop, r.recordtitle FROM record r, channel c "
                     "WHERE c.channelid = r.recordchannelid";
   
   sqlite3_stmt *getrec_stmt;
   sqlite3_prepare_v2(db,get_recording_sql, -1, &getrec_stmt, NULL);
   while(sqlite3_step(getrec_stmt) == SQLITE_ROW){
      recording rec;
      rec.channel = g_string_new(sqlite3_column_text(getrec_stmt,0));
      rec.start = sqlite3_column_int64(getrec_stmt,1);
      rec.stop = sqlite3_column_int64(getrec_stmt,2);
      rec.title = g_string_new(sqlite3_column_text(getrec_stmt,3));
      g_array_append_val(result,rec);
   }

   sqlite3_finalize(getrec_stmt);

   return result;
}

/*
   Gets the programme for a given channel from the database
   @param            channelname       name of the channel to get program for
   @return                             returns an array with all programmes for the channel
*/
GArray* iptvx_db_get_channel_programme(GString* channelname){
   GArray* result = g_array_new(false,false,sizeof(programme));

   /* define start time as now minus 5 hours */
   long starttime = time(NULL)-18000;

   char* get_prog_sql = sqlite3_mprintf(
               "SELECT prog.programmetitle, prog.programmedescription, "
               "prog.programmestart, prog.programmestop, "
               "prog.programmeproductiondate "
               "FROM programme prog, channel chan "
               "WHERE chan.channelname = '%q' "
               "AND prog.programmestart > %lld "
               "AND chan.channelid = prog.programmechannelid",
               channelname->str,starttime);

   sqlite3_stmt *getprog_stmt;
   sqlite3_prepare_v2(db,get_prog_sql, -1, &getprog_stmt, NULL);
   while(sqlite3_step(getprog_stmt) == SQLITE_ROW){
       /* create programme struct */
       programme prog;

       /* get all the db string values */
       char* prog_title = (char*)sqlite3_column_text(getprog_stmt,0);
       char* prog_desc = (char*)sqlite3_column_text(getprog_stmt,1);

       /* copy the string into the struct */
       prog.title = g_string_new(prog_title);
       prog.description = g_string_new(prog_desc);
       prog.category = g_string_new("");

       /* get the integer values */
       prog.start = sqlite3_column_int64(getprog_stmt,2);
       prog.stop = sqlite3_column_int64(getprog_stmt,3);
       prog.productionDate = sqlite3_column_int(getprog_stmt,4);

       /* append to the result array */
       g_array_append_val(result,prog);
   }

   /* finalise the statement */
   sqlite3_finalize(getprog_stmt);

   return result;
}

/*
   Updates the database with epg data
   @param      epg      array with all epg data
   @param      min_age  minimum age of epg data in hours
*/
void iptvx_db_update(GArray* epg, int min_age){
   long expiry_time = time(NULL)-(min_age*3600);

   /* go through all channels and insert them
      into the database if they do not already exist */
   int c = 0;
   for(c = 0; c < epg->len; c++){
      /* get the channel from the list */
      channel* chan = &g_array_index(epg,channel,c);

      /* insert to database when data outdated */
      if(chan->lastUpdated < expiry_time){
         iptvx_db_insert_channel(chan);
      }
   }
}

/*
   Returns the timestamp of the last epg update of a channel
   @param         channelname    name of the channel to get timestamp for
   @return                       long with timestamp of last epg update
*/
long iptvx_db_get_channel_last_updated(GString* channelname){
   long result = 0;

   char* get_recording_sql = sqlite3_mprintf(
                           "SELECT channelepgupdated FROM channel "
                           "WHERE channelname = '%q'",channelname->str);

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db,get_recording_sql, -1, &stmt, NULL);
   while(sqlite3_step(stmt) == SQLITE_ROW){
      result = sqlite3_column_int64(stmt,0);
   }

   sqlite3_finalize(stmt);  

   return result;
}

/*
   Closes the database
*/
void iptvx_db_close(){
   sqlite3_close(db);
}