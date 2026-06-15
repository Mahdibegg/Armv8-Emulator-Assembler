#include "assemble/two_pass.h"
#include "shared/types.h"
#include "tokenizer.h"
#include "assemble/symbol_table.h"
#include "assemble/reader.h"
#include "assemble/binary_writer.h"
#include "assemble/encoder.h"

#include <string.h>

#define NEXT_INSTRUCTION 4

/*
 * First_pass function runs the first pass of a two pass
 * 
 * input: File input that will be read, tokenized and then build the symbol table
 */
static symbol_table_t first_pass(FILE *input) {
    /*
     * Check if input file is null 
     * WHILE Loop with readline function to read each line 
     * For each line, tokenize line, check token type and update the current address appropriately or add to symbol table if it is a label
     * Return Symbol Table
     */
    
    if (input == NULL) {
        fprintf(stderr, "ERROR: Input file could not be opened");
        abort();
    }
     
    /* Initialise required variables for looping through file */
    char buffer[MAX_LINE_LENGTH];
    size_t line_number = 1;
    addr_t current_addr = 0;
    
    tokenized_line_t *tokenized_line = init_tokenized_line();
    symbol_table_t st = symbol_table_create();

    /*
     * Loop until end of file to which read_line returns false when EOF is reached
     */
    while (read_line(input, buffer, MAX_LINE_LENGTH)) {

        /*
         * Tokenize the raw line and store in tokenized_line
         * Pass in line_number + 1 for consistency for error messages 
         */
        tokenize_line(tokenized_line, buffer, line_number);

        /* Check the token type and upadate current address appropriately */
        switch (tokenized_line->token_type) {
            case LABEL:
                /* 
                 * Since the line is of type LABEL, we want to add the Labelname alongise the current address into the symbol table
                 * if add returns false then duplicate is found in this line, return error plus abort
                 */
                if(!symbol_table_add(st, tokenized_line->data.label_data.label, current_addr)) {
                    fprintf(stderr, "ERROR: Duplicate label found on line: %zu\n", line_number + 1);
                    abort();
                }
                break;

            case EMPTY:
                clear_tokenized_line(tokenized_line);
                line_number++;
                continue;

            case DIRECTIVE:
            case INSTRUCTION:
                /* If the line is a directive or instruction then we simply increment the current address */
                current_addr+= NEXT_INSTRUCTION;
                break;
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
     * And return reference to symbol table
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
static void second_pass(FILE *input, FILE *output, const symbol_table_t st) {
    /*
     * Check for null input file
     * Reading each line during the loop
     * Then verify its not an empty read (empty line)
     * Tokenize the line and encode directives/instructions through a switch case
     */

    if (input == NULL) {
        fprintf(stderr, "ERROR: Input file could not be opened");
        abort();
    }

    /* Initialise required variables for looping through file */
    char buffer[MAX_LINE_LENGTH];
    size_t line_number = 1;
    addr_t current_addr = 0;

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
                word_t encoded_value = encode(st, tokens, current_addr);
                binary_writer(output, encoded_value);

                /* Forward to the next address */
                current_addr += NEXT_INSTRUCTION;
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

        line_number++;
    }

    free_tokenized_line(tokens);
}

void two_pass(FILE *input, FILE *output) {
    /*
     * Initialises the symbol table
     * Runs the first pass by passing the symbol table and the input file reference
     * Rewinds the file pointer to point to the first line in the file
     * Runs the second pass by passing a read only reference to the symbol table, the input file reference and the output file reference
     * Frees the memory allocated by the symbol table
     */

    symbol_table_t *st = first_pass(input);

    rewind(input);

    second_pass(input, output, st);

    symbol_table_free(st);
}