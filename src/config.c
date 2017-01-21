#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <libconfig.h>
#include <unistd.h>
#include <glib.h>

config_t cfg;

/* gets the configuration file and also 
	checks if the configuration file exists */
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

/* checks if a local config file is present */
bool iptvx_config_file_exists(){
	bool result = false;

	char* configFile = iptvx_get_config_filename();
	if(configFile[0] != '\0'){
		result = true;
	}

	return result;
}

/* initialises and loads config file */
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

int iptvx_config_get_setting_int(char* setting_name, int default_value){
	int result = default_value;

	int base;
	if (config_lookup_int(&cfg, setting_name, &base)){
		result = base;
	}

	return result;
}

bool iptvx_config_get_setting_bool(char* setting_name, bool default_value){
	bool result = default_value;

	int base;
	if (config_lookup_bool(&cfg, setting_name, &base)){
		result = base;
	}

	return result;
}

char* iptvx_config_get_setting_string(char* setting_name, char* default_value){
	char* result = default_value;

	const char* base;
	if (config_lookup_string(&cfg, setting_name, &base)){
		result = (char*)base;
	}

	return result;
}