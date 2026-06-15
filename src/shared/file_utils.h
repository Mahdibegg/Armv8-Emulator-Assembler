#include <stdbool.h>

/*
* Use strrchr to get pointer to the last '.' character
* Check if '.' is even in the file name by checking if strrchr returned a null pointer 
* Then check if this matches ".s" for the input
* And if this matches ".bin" for the output file
* Return true or false accordingly
*/
bool has_extension(const char *filename, const char *ext);