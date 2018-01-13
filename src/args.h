/*

   Copyright 2018   Jan Kammerath

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

#ifndef	ARGS_H
#define ARGS_H

#include <stdbool.h>

/* struct containing arguments */
struct arguments{ 
  char* configFile;
  bool daemon;
  bool test;
};

/*
  Parses the arguments
  @param    argc      argument count
  @param    argv      char array with arguments
  @return             a struct with argument information
*/
extern struct arguments iptvx_parse_args(int argc, char *argv[]);

#endif