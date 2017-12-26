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

/* indicator to stay alive */
volatile sig_atomic_t iptvx_daemon_alive;

/* define the port to run the daemon on */
int iptvx_daemon_server_port;

/* defines recording tolerance in minutes */
int iptvx_daemon_record_tolerance;

/* represents the working dir */
GString* record_dir;

/* string with epg json data */
GString* iptvx_daemon_epg_json;

/* garray with all epg data */
GArray* iptvx_daemon_epg_data;

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
   Provides the proper result for the url
   @param      request_url    url requested from the client
   @return                    string with JSON contents to show
*/
GString* iptvx_daemon_get_response(char* request_url){
   GString* result = g_string_new("{}");

   /* check for requested data */
   if(g_strcmp0(request_url,"/epg.json")==0){
      /* full epg in json is requested */
      if(iptvx_daemon_epg_json != NULL){
         result = iptvx_daemon_epg_json;
      }
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
  GString* content = iptvx_daemon_get_response((char*)url);

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
   Runs the daemon loop, checks for signals
   and performs the necessary operations.
*/
void iptvx_daemon_run(){
   /* indicate the daemon to stay alive */
   iptvx_daemon_alive = true;

   /* attach sigterm handler to kill daemon */
   struct sigaction action;
   memset(&action, 0, sizeof(struct sigaction));
   action.sa_handler = iptvx_daemon_kill;
   sigaction(SIGTERM, &action, NULL);

   /* create the http lib */
   struct MHD_Daemon * d;

   d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
             iptvx_daemon_server_port, NULL, NULL,
             &iptvx_daemon_handle_request,
             "", MHD_OPTION_END);

   /* monitor scheduled actions until a signal 
      is caught to stop or interrupt */
   while(iptvx_daemon_alive == true){
      
      /* wait a sec */
      sleep(1);
   }

   /* stop the daemon when finished */
   MHD_stop_daemon(d);
}