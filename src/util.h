#ifndef	UTIL_H
#define UTIL_H

bool util_file_exists(char* fileName);
GString* util_download_string(char* url);
char* str_replace(const char* orig_str, const char* old_token, const char* new_token);

#endif