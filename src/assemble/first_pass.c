#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "assemble/first_pass.h"
#include "assemble/symbol_table.h"


symbol_table_t first_pass(FILE *input) {
    /*
     * Initialise the buffer that holds a line read from the file, will also be passed into tokenizer
     * Initialise line_number at 0, will also be passed into tokenizer for error messsages, to be incremented at the end of the function
     * Read each line of file, tokenize each line and update current address respective to tokenized line type
     * Build symbol table through each iteration respectively.
     */

    int line_number = 0;

    char *raw_line;

    addr_t current_address;
}