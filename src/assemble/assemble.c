#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <assemble/assemble.h>

static bool file_type_check( char *input_file,  char *output_file) {
  return false;
}

int main(int argc, char **argv) {

  /*
   * First argument - Input file (.s)
   * Second argument - Output file (.bin)
   * Need to validate number of arguments by checking if argc = 3
   * If arguments are valid then call two pass function 
   * Then close files 
   * Return success or failure
   */

   if (argc != 3) {
    fprintf(stderr, "ERROR: Invalid number of arguments\n");
    return 0;
   }

   /* Check if the first input is a .s file and the second input is a .bin file*/

  return EXIT_SUCCESS;
}
