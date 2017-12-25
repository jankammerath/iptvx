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

/*
   @author     Jan Kammerath
   @date       25 Jan 2017

   Application configuration module working 
   with libconfig to handle the config files,
   read them and provide the settings to 
   the application
*/

#ifndef	CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/*
   Gets the configuration struct pointer
   @return           Config struct pointer
*/
config_t* iptvx_get_config();

/*
   Gets the directory to store the data in
   @return     dir name of where to store data in
*/
char* iptvx_config_get_data_dir();

/*
   Initialises the configuration
   @return           true when ok, otherwise false
*/
bool iptvx_config_init();

/*
   Checks if config file exists
   @return           true when it's there, false when not
*/
bool iptvx_config_file_exists();

/*
   Sets the path of the config file to use
   @param      configFile     the file name of the config file
*/
void iptvx_set_config_filename(char* configFile);

/*
   Gets the config file name
   @return           the config file name and path
*/
char* iptvx_get_config_filename();

/*
   Gets the directory to store the data in
   @return     dir name of where to store data in
*/
char* iptvx_config_get_data_dir();

/* 
   Gets the overlay app file
   @return           file name of the overlay application
*/
char* iptvx_config_get_overlay_app();

/* 
   Gets a setting value as integer
   @param   setting_name      name of the setting
   @param   default_value     default value to use when not present
   @return                    integer value of the setting
*/
int iptvx_config_get_setting_int(char* setting_name, int default_value);

/* 
   Gets a setting value as bool
   @param   setting_name      name of the setting
   @param   default_value     default value to use when not present
   @return                    bool value of the setting
*/
bool iptvx_config_get_setting_bool(char* setting_name, bool default_value);

/* 
   Gets a setting value as string
   @param   setting_name      name of the setting
   @param   default_value     default value to use when not present
   @return                    string value of the setting
*/
char* iptvx_config_get_setting_string(char* setting_name, char* default_value);

#endif