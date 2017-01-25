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

#ifndef	CONFIG_H
#define CONFIG_H

#include <stdbool.h>

config_t* iptvx_get_config();
bool iptvx_config_init();
bool iptvx_config_file_exists();
char* iptvx_get_config_filename();
char* iptvx_config_get_overlay_app();
int iptvx_config_get_setting_int(char* setting_name, int default_value);
bool iptvx_config_get_setting_bool(char* setting_name, bool default_value);
char* iptvx_config_get_setting_string(char* setting_name, char* default_value);

#endif