#ifndef	UTIL_H
#define UTIL_H

bool util_file_exists(char* fileName);
void file_put_contents(GString* file, GString* content);
GString* file_get_contents(GString* file);
GString* util_download_string(char* url);
char* str_replace(const char* orig_str, const char* old_token, const char* new_token);

#endif