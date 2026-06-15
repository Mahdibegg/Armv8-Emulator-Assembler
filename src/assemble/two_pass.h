#ifndef TWO_PASS_H
#define TWO_PASS_H

#include <stdio.h>

#include "symbol_table.h"

/*
 * Two_pass function runs both the first_pass and the second_pass
 * This abstracts the first_pass and second_pass, which become backend in .c (static)
 * 
 * input: File input that will be read, tokenized, encoded then written
 * output: File output that will be written to, should be a .bin file
 * st: Symbol table pointer that will be used for label lookup
 */
void two_pass(FILE *input, FILE *output);

#endif