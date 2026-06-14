#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble/tokenizer.h"
#include "shared/types.h"

tokenized_line_t *init_tokenized_line(void) {

    tokenized_line_t *line;

    line = malloc(sizeof(tokenized_line_t));

    if (line == NULL) {
        fprintf(stderr, "ERROR: Could not allocate tokenized line\n");
        abort();
    }

    /*
     * Initialise fields 
     * 
     * Set line number to 0
     * Set Token type to EMPTY
     */
     
    line->line_number = 0;
    line->token_type = EMPTY;

    return line;
}

void clear_tokenized_line(tokenized_line_t *line) {

    /* First check if line is NULL */
    if (line = NULL) {
        fprintf(stderr, "ERROR: Tokenized line is NULL\n");
        abort();
    }

    /*
     * Need to Free the data according to the token type
     */
    switch (line->token_type) {
        case LABEL:
            /* Free label in label_data struct */
            free(line->data.label_data.label);
            break;
        
        case DIRECTIVE:
            /* Free opcode */
            free(line->data.directive_data.value);
            break;

        case INSTRUCTION:
            /* Free array of operands and opcode */
            free(line->data.instruction_data.opcode);

            for (size_t i = 0; i < line->data.instruction_data.operand_count; i++) {
                /* Free each operand in the array */
                free(line->data.instruction_data.operands[i]);
            }

            free(line->data.instruction_data.operands);
            break;

        case EMPTY:
            /* Nothing to free so simply break */
            break;
    }

    /*
     * Initialise fields
     */
    line->line_number = 0;
    line->token_type = EMPTY;
}

void tokenize_line(tokenized_line_t *line, char *buffer, size_t line_number) {
    /*
     * Nul pointer check on buffer and silent return (since nothing to tokenize)
     * Check first word and assign token type accordingly 
     * Then assign the fields of relevent struct within union by splitting string into opcode and operands
     * ":" at end of line indidcates label, "." indicates directive, otherwise instruction
     */

    if (buffer == NULL) {
        return;
    }
    
    char *colon_ptr = strchr(buffer, ':');

    if (buffer[0] == '\0') {
        line->token_type = EMPTY;
        line->line_number = line_number;
        return;
    } else if (buffer[0] == '.') {
        line->token_type = DIRECTIVE;
        line->line_number = line_number;

        /* Token = .int*/
        token_t token = strtok(buffer, " ");
        token = strtok(NULL, " ");

        if (token == NULL) {
            fprintf(stderr, "ERROR: Invalid directive line");
            abort();
        }
        line->data.directive_data.value = malloc(strlen(token)+1);

        if (line->data.directive_data.value == NULL) {
            fprintf(stderr, "ERROR: Could not allocate directive value\n");
            abort();
        }
        strcpy(line->data.directive_data.value, token);
        return;
    }
}

void free_tokenized_line(tokenized_line_t *line) {
    /* No error since this pointer does not reference any memory */
    if (line == NULL) {
        return;
    }

    /* 
     * Freeing any malloc()ed pointer references within the struct
     *
     * Freeing fields by case checking the token_type
     * Since the .data would not be set for different unions, hence it would be illogical to free unused memory
     */

    switch (line->token_type) {
        case LABEL:
            free(line->data.label_data.label);
            break;

        case DIRECTIVE:
            free(line->data.directive_data.value);
            break;

        case INSTRUCTION:
            free(line->data.instruction_data.opcode);

            for (size_t i = 0; i < line->data.instruction_data.operand_count; i++) {
                free(line->data.instruction_data.operands[i]);
            }

            free(line->data.instruction_data.operands);
            break;
    }
    
    free(line);
}