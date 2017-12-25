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
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <microhttpd.h>
#include <glib.h>

/* indicator to stay alive */
volatile sig_atomic_t iptvx_daemon_alive;

int iptvx_daemon_server_port;

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
  char* page = (char*)url;

  /* create the response for the query */
  response = MHD_create_response_from_buffer(strlen(page), (void*) page,
                                             MHD_RESPMEM_PERSISTENT);
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