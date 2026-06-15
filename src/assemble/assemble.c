#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "assemble/assemble.h"
#include "shared/file_utils.h"

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
    return EXIT_FAILURE;
   }

   /* Check if the first input is a .s file and the second input is a .bin file*/
   if (!has_extension(argv[1], ".s") || !has_extension(argv[2], ".bin")) {
    fprintf(stderr, "ERROR: Invalid file types for input/output\n");
    return EXIT_FAILURE;
   }

   /*Open the input file in read mode and the output file in write mode*/
   FILE *input = fopen(argv[1], "r");

   if (input == NULL) {
    fprintf(stderr, "ERROR: Failed to open input file in read mode\n");
    return EXIT_FAILURE;
   }

   FILE *output = fopen(argv[2], "wb");

   if (output == NULL) {
    fprintf(stderr, "ERROR: Failed to open output file in write mode\n");
    fclose(input);
    return EXIT_FAILURE;
   }

   two_pass(input, output);

   fclose(input);
   fclose(output);

   /*Successful so return 0*/
   return EXIT_SUCCESS;
}
