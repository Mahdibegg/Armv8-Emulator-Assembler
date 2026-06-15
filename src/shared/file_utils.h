#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdbool.h>

/*
* Use strrchr to get pointer to the last '.' character
* Check if '.' is even in the file name by checking if strrchr returned a null pointer 
* Then check if this matches "ext"
* Return true or false accordingly
*/
bool has_extension(const char *filename, const char *ext);

#endif