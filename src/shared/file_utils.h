#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdbool.h>
#include <stdio.h>

/*
* Use strrchr to get pointer to the last '.' character
* Check if '.' is even in the file name by checking if strrchr returned a null pointer 
* Then check if this matches "ext"
* Return true or false accordingly
*/
bool has_extension(const char *filename, const char *ext);

/*
* Opens a file with the given mode and exits the program on failure.
*
* This function wraps fopen() to provide consistent error handling
* If the file cannot be opened, an error message is printed using perror() and the program terminates
*
* Return valid file pointer on success
*/
FILE *open_file_or_exit(const char *filename, const char *mode);

#endif