#include "assemble/two_pass.h"
#include "shared/types.h"
#include "assemble/tokenizer.h"
#include "assemble/symbol_table.h"
#include "assemble/reader.h"
#include "assemble/binary_writer.h"
#include "assemble/encoder.h"

#include <string.h>

#define MAX_LINE_LENGTH 256
#define NEXT_INSTRUCTION 4

/*
 * First_pass function runs the first pass of a two pass
 * 
 * input: File input that will be read, tokenized and then build the symbol table
 */ // NEED TO CHANGE CONST
static symbol_table_t first_pass(const FILE *input) {
    /*
     * Check if Input file is null 
     * Initialise line number, current address and line buffer, symbol table and tokenized line buffer
     * WHILE Loop with readline function to read each line 
     * For each line, tokenize line, check token type and update the current address appropriately or add to symbol table if it is a label
     * Return Symbol Table
     */

    
    if (input == NULL) {
        fprintf(stderr, "ERROR: Input file could not be opened");
        abort();
    }
    
     
    size_t line_number = 0;

    addr_t current_addr = 0;

    char raw_line[MAX_LINE_LENGTH];

    tokenized_line_t *tokenized_line = init_tokenized_line();

    symbol_table_t st = symbol_table_create();

    while (read_line(input, raw_line, MAX_LINE_LENGTH)) {

        /*
         * Tokenize the raw line and store in tokenized_line
         * Pass in line_number + 1 for consistency for error messages 
         */
        tokenize_line(tokenized_line, raw_line, line_number+1);

        /* Check the token type and upadate current address appropriately */
        if (tokenized_line->token_type == LABEL) {
            /* 
             * Since the line is of type LABEL, we want to add the Labelname alongise the current address into the symbol table
             * if add returns false then duplicate is found in this line, return error plus abort
             */
            if(!symbol_table_add(st, tokenized_line->data.label_data.label, current_addr)) {
                fprintf(stderr, "ERROR: Duplicate label found on line: %zu\n", line_number + 1);
                abort();
            }
        } else if (tokenized_line->token_type == EMPTY) {
            clear_tokenized_line(tokenized_line);
            line_number++;
            continue;
        } else {
            /* If the line is a directive or instruction then we simply increment the current address */
            current_addr+= NEXT_INSTRUCTION;
        }

        /*
         * Need to clear the tokenized line for the next iteration
         * Increment line number
         */

        clear_tokenized_line(tokenized_line);
        line_number++;
    }

    /*
     * Free the tokenized line 
     * Return symbol table
     */

    free_tokenized_line(tokenized_line);
    
    return st;
}

/*
 * Second_pass function runs the second pass of a two pass
 * 
 * input: File input that will be read, tokenized, encoded then written
 * output: File output that will be written to, should be a .bin file
 * st: Symbol table pointer that will be used for label lookup
 */
static void second_pass(const FILE *input, FILE *output, const symbol_table_t st) {
    /* magic number to change */
    char buffer[MAX_LINE_LENGTH];
    size_t line_number = 0;
    addr_t address = 0;

    tokenized_line_t *tokens = init_tokenized_line();

    /*
     * Loop until end of file to which read_line returns false when EOF is reached
     */
    while (read_line(input, buffer, sizeof(buffer))) {

        /* Empty line set buffer to empty and go to next loop*/
        if (buffer[0] == '\0') {
            continue;
        }

        /* Clear previous token data in buffer called tokens*/
        clear_tokenized_line(tokens);

        tokenize_line(tokens, buffer, line_number);

        /* Only allowing directives and instructions to be further parsed and encoded */
        switch (tokens->token_type) {
            /* Encode function abstracts encoding process for both directive and instruction tokens */
            case DIRECTIVE:
            case INSTRUCTION:
                word_t encoded_value = encode(st, *tokens);
                binary_writer(output, encoded_value);
                break;
            /* Break LABEL and EMPTY case, continue to next line */
            case LABEL:
                break;
            case EMPTY:
                break;
            default:
                fprintf(stderr, "ERROR: Unknown token type parsed for encoding at %zu\n",
                    line_number
                );
                abort();
        }

        /* Magic number to change*/
        address += NEXT_INSTRUCTION;

        line_number++;
    }

    free_tokenized_line(tokens);
}

void two_pass(const FILE *input, FILE *output) {
    return NULL;
}