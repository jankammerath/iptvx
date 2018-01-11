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

#ifndef DAEMON_H
#define DAEMON_H

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <microhttpd.h>
#include <glib.h>

/*
   Sets the application directory for the overlay
   @param       appdir        path where the app files are in
*/
void iptvx_daemon_set_app_dir(GString* appdir);

/*
   Sets the data directory where all application data is in
   @param       datadir        path where the data files are in
*/
void iptvx_daemon_set_data_dir(GString* datadir);

/*
   Sets the percentage amount of loaded epg content
   @param      percentage     int with percentage loaded (0-100)
*/
void iptvx_daemon_set_epg_status(int percentage);

/*
   Sets the recording tolerance in minutes
   @param      tolerance      the tolerance in minutes
*/
void iptvx_daemon_set_record_tolerance(int tolerance);

/*
   Sets the directory to store data in
   @param      dirname     full path of directory to work in
*/
void iptvx_daemon_set_dir(char* dirname);

/*
   Sets the epg data for the daemon to return
   @param      epg_data          epg data as json string
*/
void iptvx_daemon_set_epg_data(GArray* epg_data);

/*
   Sets the epg data for the daemon to return
   @param      epg_data          epg data as json string
*/
void iptvx_daemon_set_epg_json(GString* epg_data);

/*
   Runs the daemon loop, checks for signals
   and performs the necessary operations.
*/
void iptvx_daemon_run();

/*
   Sets the port number for the daemons http server
   @param         port_number       int with port number to listen on
*/
void iptvx_daemon_set_server_port(int port_number);

#endif