#include "assemble/two_pass.h"
#include "shared/types.h"
#include "tokenizer.h"
#include "assemble/symbol_table.h"
#include "assemble/reader.h"
#include "assemble/binary_writer.h"
#include "assemble/encoder.h"

#define MAX_LINE_LENGTH 256
#define NEXT_INSTRUCTION 4

/*
 * First_pass function runs the first pass of a two pass
 * 
 * input: File input that will be read, tokenized and then build the symbol table
 */
static symbol_table_t first_pass(FILE *input) {
    return NULL;
}

/*
 * Second_pass function runs the second pass of a two pass
 * 
 * input: File input that will be read, tokenized, encoded then written
 * output: File output that will be written to, should be a .bin file
 * st: Symbol table pointer that will be used for label lookup
 */
static void second_pass(FILE *input, FILE *output, symbol_table_t st) {
    /* magic number to change */
    char buffer[MAX_LINE_LENGTH];
    size_t line_number = 0;
    addr_t address = 0;

    tokenized_line_t *tokens = init_tokenized_line();

    /*
     * Loop until EOF
     * read_line returns false when EOF is reached
     */
    while (read_line(input, buffer, sizeof(buffer))) {

        /* Empty line set buffer to empty and go to next loop*/
        if (buffer[0] == '\0') {
            continue;
        }

        /* Clear previous token data in buffer called tokens*/
        clear_tokenized_line(tokens);

        tokenize_line(tokens, buffer, line_number);

        /* Write 32-bit word to output */

        /* Magic number to change*/
        address += NEXT_INSTRUCTION;

        line_number++;
    }

    free_tokenized_line(tokens);
}

void two_pass(FILE *input, FILE *output, symbol_table_t st) {
    return NULL;
}