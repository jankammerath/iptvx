#include <argp.h>

const char *argp_program_version = "iptvx 0.1 alpha";
const char *argp_program_bug_address = "<dev@iptvx.org>";
static char args_doc[] = "VIDEO_FILE HTML_FILE";
static char doc[] = "iptvx -- An IPTV client and server system";
static struct argp_option options[] =
{
  {0}
};

struct arguments
{
  char *input_video_file;
  char *input_html_file;
  bool sufficient;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) { 
    struct arguments *arguments = state->input; 
  
    switch (key) {
        case ARGP_KEY_NO_ARGS:
        	arguments->sufficient = false; 
            argp_usage(state); 
            break; 
        case ARGP_KEY_ARG: 
        	if (state->arg_num == 0){
        		arguments->input_video_file = arg;
        		arguments->sufficient = false;
        	}if (state->arg_num == 1){
        		arguments->input_html_file = arg;
        		arguments->sufficient = true;
        	}
            break; 
        case ARGP_KEY_END:
        	if(arguments->sufficient != true){
        		argp_usage(state);
        	}
        	break;
        default: 
            return ARGP_ERR_UNKNOWN; 
    } 
    return 0; 
} 

static struct argp argp = {options, parse_opt, args_doc, doc};

struct arguments iptvx_parse_args(int argc, char *argv[]){
	struct arguments arguments;
	argp_parse (&argp, argc, argv, 0, 0, &arguments);
	return arguments;
}