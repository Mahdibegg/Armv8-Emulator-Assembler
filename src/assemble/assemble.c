#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "assemble/assemble.h"

static bool file_type_check( char *input_file,  char *output_file) {
  /*
   * Use strrchr to get pointer to the last '.' character
   * Check if '.' is even in the file name by checking if strrchr returned a null pointer 
   * Then check if this matches ".s" for the input
   * And if this matches ".bin" for the output file
   * Return true or false accordingly
   */

  const char *input_dot = strrchr(input_file, '.');

  const char *outupt_dot = strrchr(output_file, '.');

  if (input_dot == NULL || outupt_dot == NULL) {
    return false;
  }

  /*
   * Use strcmp() to check if ".s" is in input file name
   * Use strcmp() to chekc if ".bin" is in output file name
   */

  return ((strcmp(input_dot, ".s") == 0) && (strcmp(outupt_dot, ".bin") == 0));
}

int main(int argc, char **argv) {

  /*
   * First argument - Input file (.s)
   * Second argument - Output file (.bin)
   * Need to validate number of arguments by checking if argc = 3
   * If arguments are valid then open files and call two pass function 
   * Then close files 
   * Return success or failure
   */

   if (argc != 3) {
    fprintf(stderr, "ERROR: Invalid number of arguments\n");
    return 1;
   }

   /* Check if the first input is a .s file and the second input is a .bin file*/
   if (!file_type_check(argv[1], argv[2])) {
    fprintf(stderr, "ERROR: Invalid file types for input/output\n");
    return 1;
   }

   /*Open the input file in read mode and the output file in write mode*/
   FILE *input = fopen(argv[1], "r");

   if (input == NULL) {
    fprintf(stderr, "ERROR: Failed to open input file in read mode\n");
    return 1;
   }

   FILE *output = fopen(argv[2], "wb");

   if (output == NULL) {
    fprintf(stderr, "ERROR: Failed to open output file in write mode\n");
    return 1;
   }
   
   two_pass(input, output);

   fclose(input);
   fclose(output);

   /*Successful so return 0*/
   return 0;
}
