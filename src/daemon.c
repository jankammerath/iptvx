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
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <microhttpd.h>
#include <glib.h>
#include <json-c/json.h>

/* indicator to stay alive */
volatile sig_atomic_t iptvx_daemon_alive;

/* define the port to run the daemon on */
int iptvx_daemon_server_port;

/* defines recording tolerance in minutes */
int iptvx_daemon_record_tolerance;

/* sets the percentage of epg content loaded */
int iptvx_daemon_epg_status;

/* represents the working dir */
GString* record_dir;

/* string with epg json data */
GString* iptvx_daemon_epg_json;

/* garray with all epg data */
GArray* iptvx_daemon_epg_data;

/* a list with all recordings */
GArray* recordlist;

/* represents a single recording */
struct recording{
   /* name of the channel to record */
   GString* channel;

   /* start and stop timings */
   long start;
   long stop;

   /* 0 = scheduled, 1 = in progress, 
      2 = ready, 9 = failed */
   int status;

   /* filename of the recording */
   GString* filename;
} typedef recording;

/*
   Sets the percentage amount of loaded epg content
   @param      percentage     int with percentage loaded (0-100)
*/
void iptvx_daemon_set_epg_status(int percentage){
   iptvx_daemon_epg_status = percentage;
}

/*
   Sets the recording tolerance in minutes
   @param      tolerance      the tolerance in minutes
*/
void iptvx_daemon_set_record_tolerance(int tolerance){
   iptvx_daemon_record_tolerance = tolerance;
}

/*
   Sets the directory to store data in
   @param      dirname     full path of directory to work in
*/
void iptvx_daemon_set_dir(char* dirname){
   record_dir = g_string_new(dirname);
}

/*
   Sets the epg data for the daemon to return
   @param      epg_data          epg data as json string
*/
void iptvx_daemon_set_epg_data(GArray* epg_data){
   iptvx_daemon_epg_data = epg_data;
}

/*
   Sets the epg data for the daemon to return
   @param      epg_data          epg data as json string
*/
void iptvx_daemon_set_epg_json(GString* epg_data){
   iptvx_daemon_epg_json = epg_data;
}

/*
   Sets the port number for the daemons http server
   @param         port_number       int with port number to listen on
*/
void iptvx_daemon_set_server_port(int port_number){
   iptvx_daemon_server_port = port_number;
}

/*
   Kills/ stops the current daemon
*/
void iptvx_daemon_kill(){
   /* set to false and main loop will end itself */
   iptvx_daemon_alive = false;
}

/*
   Returns the list with all recordings as json
   @return        list with all recordings as json
*/
GString* iptvx_daemon_get_recordlist_json(){
   GString* result;

   json_object* j_rec_array = json_object_new_array();

   if(recordlist != NULL){
      int c = 0;
      for(c = 0; c < recordlist->len; c++){
         /* fetch the recording from the array */
         recording* rec = &g_array_index(recordlist,recording,c);

         /* create json object for this recording */
         json_object* j_rec = json_object_new_object();
         json_object_object_add(j_rec,"channel",
               json_object_new_string(rec->channel->str));
         json_object_object_add(j_rec,"filename",
               json_object_new_string(rec->filename->str));
         json_object_object_add(j_rec,"start",
               json_object_new_int(rec->start));
         json_object_object_add(j_rec,"stop",
               json_object_new_int(rec->stop));
         json_object_object_add(j_rec,"status",
               json_object_new_int(rec->status));

         /* add to json array */
         json_object_array_add(j_rec_array,j_rec);
      }
   }

   /* finally pass j_object to result string */
   result = g_string_new(json_object_to_json_string(j_rec_array));

   return result;
}

/*
   Creates a new recording struct from given data
   @param      channel     name of channel to record
   @param      start       timestamp of start time
   @param      stop        timestamp of stop time
   @return                 the recording struct with the data   
*/
recording iptvx_daemon_create_recording(char* channel, long start, long stop){
   recording result;

   result.channel = g_string_new(channel);
   result.start = start;
   result.stop = stop;
   result.status = 0;
   result.filename = g_string_new("");

   return result;
}

/*
   Adds a new recording to the list if it does not already exist
   @param         rec         the new recording to add to the list
*/
void iptvx_daemon_add_recording(recording rec){
   bool exists = false;

   int c;
   for(c = 0; c<recordlist->len;c++){
      recording* exist_rec = &g_array_index(recordlist,recording,c);

      /* check if this is the same or equal */
      if(g_strcmp0(exist_rec->channel->str,rec.channel->str)==0 
         && exist_rec->start == rec.start
         && exist_rec->stop == rec.stop){
         /* set the marker, we have it already */
         exists = true;
      }
   }

   if(!exists){
      /* this is a new one, so add it */
      g_array_append_val(recordlist,rec);
   }
}

/*
   Gets the current status of the daemon and 
   its operations as json string
   @return        json string with status info
*/
GString* iptvx_daemon_get_status_json(){
   json_object* j_status = json_object_new_object();
   json_object_object_add(j_status,"epg_loaded",json_object_new_int(iptvx_daemon_epg_status));
   return g_string_new(json_object_to_json_string(j_status));
}

/*
   Provides the proper result for the url
   @param      request_url    url requested from the client
   @param      connection     handle for the current connection
   @return                    string with JSON contents to show
*/
GString* iptvx_daemon_get_response(char* request_url, struct MHD_Connection* connection){
   GString* result = g_string_new("{}");

   /* check for requested data */
   if(g_strcmp0(request_url,"/")==0){
      /* current epg status is requested */
      result = iptvx_daemon_get_status_json();
   }if(g_strcmp0(request_url,"/epg.json")==0){
      /* full epg in json is requested */
      if(iptvx_daemon_epg_json != NULL){
         result = iptvx_daemon_epg_json;
      }
   }if(g_strcmp0(request_url,"/record.json")==0){
      /* recorder information requested */
      const char* requestedAction = MHD_lookup_connection_value 
                     (connection, MHD_GET_ARGUMENT_KIND, "action");

      /* check for any actions requested */
      if(g_strcmp0(requestedAction,"add")==0){
         /* requested to add new recording */
         const char* rec_channel = MHD_lookup_connection_value 
                  (connection, MHD_GET_ARGUMENT_KIND, "channel");
         const char* rec_start = MHD_lookup_connection_value 
                  (connection, MHD_GET_ARGUMENT_KIND, "start");
         const char* rec_stop = MHD_lookup_connection_value 
                  (connection, MHD_GET_ARGUMENT_KIND, "stop");

         /* esnure values exist and are not null */
         if(rec_channel != NULL && rec_start != NULL && rec_stop != NULL){
            /* create new recording struct */
            recording new_rec = iptvx_daemon_create_recording((char*)rec_channel,
                                                atol(rec_start), atol(rec_stop));

            /* append new recording to list */
            iptvx_daemon_add_recording(new_rec);
         }
      }

      /* output list with recordings */
      result = iptvx_daemon_get_recordlist_json();
   }

   return result;
}

/*
   Handles http requests to this daemon
*/
static int iptvx_daemon_handle_request(void * cls, struct MHD_Connection * connection,
                                       const char * url, const char * method,
                                       const char * version, const char * upload_data,
                                       size_t * upload_data_size, void ** ptr) {
  static int dummy;
  struct MHD_Response * response;
  int ret;

  /* define the response */
  GString* content = iptvx_daemon_get_response((char*)url,connection);

  /* create the response for the query */
  response = MHD_create_response_from_buffer(strlen(content->str), (void*)content->str,
                                             MHD_RESPMEM_PERSISTENT);

  /* set the response content type to json */
  MHD_add_response_header(response, "Content-Type", "application/json");

  /* queue the response */
  ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);

  return ret;
}

/*
   Checks if any scheduled recording is bound
   to be started and initialises the recording process
*/
void iptvx_daemon_check_recording(){
   /* get the current timestamp */
   long now = time(NULL);

   /* go through the list of scheduled recordings */
   int c = 0;
   for(c = 0; c < recordlist->len; c++){
      recording* rec = &g_array_index(recordlist,recording,c);
   
      /* check if recording not active 
         and schedule for now */
      if(rec->start >= now && rec->stop <= now && rec->status == 0){
         /* supposed to start now, has not yet finished and still scheduled */
         
      }
   }
}

/*
   Runs the daemon loop, checks for signals
   and performs the necessary operations.
*/
void iptvx_daemon_run(){
   /* indicate the daemon to stay alive */
   iptvx_daemon_alive = true;

   /* initialise epg status to zero */
   iptvx_daemon_epg_status = 0;

   /* initialise recording list */
   recordlist = g_array_new(false,false,sizeof(recording));

   /* attach sigterm handler to kill daemon */
   struct sigaction action;
   memset(&action, 0, sizeof(struct sigaction));
   action.sa_handler = iptvx_daemon_kill;
   sigaction(SIGTERM, &action, NULL);

   /* create the http lib */
   struct MHD_Daemon * d;

   /* start the http daemon and listen */
   d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
             iptvx_daemon_server_port, NULL, NULL,
             &iptvx_daemon_handle_request,
             "", MHD_OPTION_END);

   /* monitor scheduled actions until a signal 
      is caught to stop or interrupt */
   while(iptvx_daemon_alive == true){
      /* start recording if necessary */
      iptvx_daemon_check_recording();

      /* wait a sec */
      sleep(1);
   }

   /* stop the daemon when finished */
   MHD_stop_daemon(d);
}