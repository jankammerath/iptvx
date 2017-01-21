#ifndef	CONFIG_H
#define CONFIG_H

#include <stdbool.h>

bool iptvx_config_init();
bool iptvx_config_file_exists();
char* iptvx_get_config_filename();
char* iptvx_config_get_overlay_app();
int iptvx_config_get_setting_int(char* setting_name, int default_value);
bool iptvx_config_get_setting_bool(char* setting_name, bool default_value);
char* iptvx_config_get_setting_string(char* setting_name, char* default_value);

#endif