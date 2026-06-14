#ifndef TWO_PASS_H
#define TWO_PASS_H

#include "symbol_table.h"

/*
 * First_pass function runs the first pass of a two pass
 * 
 * input: File input that will be read, tokenized and then build the symbol table
 */
symbol_table_t first_pass(FILE *input);

/*
 * Second_pass function runs the second pass of a two pass
 * 
 * input: File input that will be read, tokenized, encoded then written
 * output: File output that will be written to, should be a .bin file
 * st: Symbol table pointer that will be used for label lookup
 */
void second_pass(FILE *input, FILE *output, const symbol_table_t st);
 
/*
 * Two_pass function runs both the first_pass and the second_pass
 * This abstracts the first_pass and second_pass, which become backend
 * 
 * input: File input that will be read, tokenized, encoded then written
 * output: File output that will be written to, should be a .bin file
 * st: Symbol table pointer that will be used for label lookup
 */
void two_pass(FILE *input, FILE *output, symbol_table_t st);

#endif