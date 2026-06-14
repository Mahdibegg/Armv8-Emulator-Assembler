#include <stdio.h>

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
            free(line->data.directive_data.opcode);
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
            free(line->data.directive_data.opcode);
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