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

#include <argp.h>
#include <stdbool.h>

const char *argp_program_version = "iptvx 0.5 beta";
const char *argp_program_bug_address = "<dev@iptvx.org>";
static char args_doc[] = "";
static char doc[] = "iptvx -- An IPTV player and recorder";
static struct argp_option options[] = { 
  {"config", 'c', "FILE", 0, "Define the config file to use" },
  { 0 }
};

struct arguments{ 
  char* configFile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) { 
  struct arguments *arguments = state->input; 
  switch (key) {
    case 'c':
      arguments->configFile = arg;
      break;
    case ARGP_KEY_NO_ARGS:
      break; 
    case ARGP_KEY_END:
    	/* print application banner */
      printf("%s\n\n"
        "You are using version '%s'\n"
        "Please file bugs in the issue tracking:\n%s\n\n",
        doc,argp_program_version,
        "https://github.com/jankammerath/iptvx/issues");
    	break;
    default: 
      return ARGP_ERR_UNKNOWN; 
  } 
  return 0; 
} 

static struct argp argp = {options, parse_opt, args_doc, doc};

/*
  Parses the arguments
  @param    argc      argument count
  @param    argv      char array with arguments
  @return             a struct with argument information
*/
struct arguments iptvx_parse_args(int argc, char *argv[]){
	struct arguments arguments;
	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	return arguments;
}