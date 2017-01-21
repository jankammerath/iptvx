#include <argp.h>
#include <stdbool.h>

const char *argp_program_version = "iptvx 0.2 alpha";
const char *argp_program_bug_address = "<dev@iptvx.org>";
static char args_doc[] = "";
static char doc[] = "iptvx -- An IPTV client and server system";
static struct argp_option options[] =
{
  {0}
};

struct arguments{ 
  char* configFile;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) { 
  //struct arguments *arguments = state->input; 
  switch (key) {
    case ARGP_KEY_NO_ARGS:
      break; 
    case ARGP_KEY_END:
    	argp_usage(state);
    	break;
    default: 
      return ARGP_ERR_UNKNOWN; 
  } 
  return 0; 
} 

static struct argp argp = {options, parse_opt, args_doc, doc};

struct arguments iptvx_parse_args(int argc, char *argv[]){
	struct arguments arguments;
	//argp_parse (&argp, argc, argv, 0, 0, &arguments);
	return arguments;
}