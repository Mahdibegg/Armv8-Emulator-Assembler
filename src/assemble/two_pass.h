#ifndef TWO_PASS_H
#define TWO_PASS_H

#include <stdio.h>

#include "assemble/symbol_table.h"

/*
 * Two_pass function runs both the first_pass and the second_pass
 * This abstracts the first_pass and second_pass, which become backend in .c (static)
 * 
 * input: File input that will be read, tokenized, encoded then written
 * output: File output that will be written to, should be a .bin file
 */
void two_pass(const FILE *input, FILE *output);

#endif