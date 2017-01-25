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
   
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libconfig.h>
#include <unistd.h>
#include <glib.h>

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

	/* check if file is in app directory */
	char* localconfig = "cfg/iptvx.conf";
	if(access(localconfig, F_OK) != -1) {
		/* there is a local config file */
		result = localconfig;
	}else{
		printf("No local config file found ('cfg/iptvx.conf')\n");
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
			/* config is good */
			result = true;
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
			char* appDir;
			char buff[PATH_MAX+1];

    		appDir = getcwd(buff,PATH_MAX+1);
    		if(appDir != NULL) {
        		result = g_strjoin("/",appDir,appFile,NULL);
    		}
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