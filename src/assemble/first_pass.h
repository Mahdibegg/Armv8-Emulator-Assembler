#ifndef FIRST_PASS_H
#define FIRST_PASS_H

#include <stdio.h>
#include "assemble/symbol_table.h"

/*
 * Perform the First pass of assembler
 * Read every line of the input file and build symbol table
 * Only store pairs of label and address
 * Skip over lines that are empty 
 * Increment current address when line is not label
 * Whe line is label then store label alongside current address
 * Return symbol table 
 */
symbol_table_t first_pass(FILE *input);


#endif