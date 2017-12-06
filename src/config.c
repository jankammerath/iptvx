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

/*
   @author     Jan Kammerath
   @date       25 Jan 2017

   Application configuration module working 
   with libconfig to handle the config files,
   read them and provide the settings to 
   the application
*/
   
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libconfig.h>
#include <unistd.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "util.h"

config_t cfg;

/*
   Gets the configuration struct pointer
   @return           Config struct pointer
*/
config_t* iptvx_get_config(){
	return &cfg;
}

/*
   Gets the config file name
   @return           the config file name and path
*/
char* iptvx_get_config_filename(){
	char* result = "";

	bool configFileFound = false;

	/* check if file is in app directory */
	if(util_file_exists("cfg/iptvx.conf")){
		result = "cfg/iptvx.conf";
		configFileFound = true;
	}if(util_file_exists("/etc/iptvx/iptvx.conf")){
		result = "/etc/iptvx/iptvx.conf";
		configFileFound = true;
	}if(util_file_exists("~/.iptvx/iptvx.conf")){
		result = "~/.iptvx/iptvx.conf";
		configFileFound = true;
	}

	if(!configFileFound){
		printf("No config file found!\n"
				"Ensure it's readable in one of the following paths:\n"
				"./cfg/iptvx.conf\n"
				"~/.iptvx/iptvx.conf\n"
				"/etc/iptvx/iptvx.conf\n");
	}

	return result;
}

/*
   Checks if config file exists
   @return           true when it's there, false when not
*/
bool iptvx_config_file_exists(){
	bool result = false;

	char* configFile = iptvx_get_config_filename();
	if(configFile[0] != '\0'){
		result = true;
	}

	return result;
}

/*
	Checks if the current config has channels
	@return 		true when channels in config, otherwise false
*/
bool iptvx_config_has_channels(){
	bool result = false;

	config_setting_t* root = config_root_setting(&cfg);
	config_setting_t* channels = config_setting_get_member(root,"channels");
	if(channels != NULL){
		result = true;
	}

	return result;
}

/*
	Gets the directory to store the data in
	@return 		dir name of where to store data in
*/
char* iptvx_config_get_data_dir(){
	char* result = "";
	char* epgFolder = "";
	char* logoFolder = "";

	if (!config_lookup_string(&cfg, "data", (const char **)&result)){
		/* config setting not preset, default is './data' */
		result = "data";
	}

	/* define the epg and logo folder */
	epgFolder = g_strjoin("/",result,"epg/",NULL);
	logoFolder = g_strjoin("/",result,"logo/",NULL);

	/* check if the directories exist */
	struct stat st = {0};
	if (stat(result, &st) == -1) {
		/* data dir does not exists, create it */
		mkdir(result, 0777);
	}if (stat(logoFolder, &st) == -1) {
		/* logo dir does not exists, create it */
		mkdir(logoFolder, 0777);
	}if (stat(epgFolder, &st) == -1) {
		/* epg dir does not exists, create it */
		mkdir(epgFolder, 0777);
	}

	/* free folder variables */
	free(epgFolder);
	free(logoFolder);

	return result;
}

/*
   Initialises the configuration
   @return           true when ok, otherwise false
*/
bool iptvx_config_init(){
	bool result = false;

	if(iptvx_config_file_exists() == true){
		/* init config lib */
		config_init(&cfg);

		/* define config file */
		char* configFile = iptvx_get_config_filename();

		if (!config_read_file(&cfg,configFile)) {
			fprintf(stderr, "Bad configuration file.\n"
							"%s:%d - %s\n",
				config_error_file(&cfg),
				config_error_line(&cfg),
				config_error_text(&cfg));
			config_destroy(&cfg);
		}else{
			if(iptvx_config_has_channels()){
				char* data_dir = iptvx_config_get_data_dir();
				GString* gs_data_dir = g_string_new(data_dir);
				if(gs_data_dir->len > 0){
					/* config is good */
					result = true;
				}else{
					printf("Data directory not present "
							"or cannot be created.\n");
				}

				/* free the string */
				g_string_free(gs_data_dir,true);
			}else{
				/* show an error when there are not channels in config */
				printf("No channels configured, check your config.\n");
			}
		}
	}

	return result;
}

/* 
   Gets the overlay app file
   @return           file name of the overlay application
*/
char* iptvx_config_get_overlay_app(){
	char* result = "";

	const char* appFile = "";
	if (config_lookup_string(&cfg, "app", &appFile)){
		/* we have the app file and check if the defined 
			path is relative which means that we need to
			actually get the full dir */
		if(appFile[0] != '/'){
			/* we need to create the full file path */
    		char* appDir = realpath(appFile,NULL);
    		if(appDir != NULL) {
        		result = appDir;
    		}else{
    			printf("Application configuration problem:\n"
    					"Failed to determine path of '%s'\n",appFile);
    		}
		}else{
			/* it's an absolute path, just use it */
			result = (char*)appFile;
		}
	}

	return result;
}

/* 
   Gets a setting value as integer
   @param   setting_name      name of the setting
   @param   default_value     default value to use when not present
   @return                    integer value of the setting
*/
int iptvx_config_get_setting_int(char* setting_name, int default_value){
	int result = default_value;

	int base;
	if (config_lookup_int(&cfg, setting_name, &base)){
		result = base;
	}

	return result;
}

/* 
   Gets a setting value as bool
   @param   setting_name      name of the setting
   @param   default_value     default value to use when not present
   @return                    bool value of the setting
*/
bool iptvx_config_get_setting_bool(char* setting_name, bool default_value){
	bool result = default_value;

	int base;
	if (config_lookup_bool(&cfg, setting_name, &base)){
		result = base;
	}

	return result;
}

/* 
   Gets a setting value as string
   @param   setting_name      name of the setting
   @param   default_value     default value to use when not present
   @return                    string value of the setting
*/
char* iptvx_config_get_setting_string(char* setting_name, char* default_value){
	char* result = default_value;

	const char* base;
	if (config_lookup_string(&cfg, setting_name, &base)){
		result = (char*)base;
	}

	return result;
}