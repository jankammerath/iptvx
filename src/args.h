#ifndef	ARGS_H
#define ARGS_H

#include <stdbool.h>

struct arguments
{
  char *input_video_file;
  char *input_html_file;
  bool sufficient;
};

extern struct arguments iptvx_parse_args(int argc, char *argv[]);

#endif