#ifndef	CONFIG_H
#define CONFIG_H

#include <stdbool.h>

bool iptvx_config_init();
bool iptvx_config_file_exists();
char* iptvx_get_config_filename();
char* iptvx_config_get_overlay_app();

#endif