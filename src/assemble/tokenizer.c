#include <stdio.h>

#include "tokenizer.h"
#include "../shared/types.h"

tokenized_line_t *init_tokenized_line(void) {

    tokenized_line_t *line;

    line = malloc(sizeof(tokenized_line_t));

    if (line == NULL) {
        fprintf("ERROR: could not allocate tokenized line\n");
        abort();
    }

    /*
     * Initialise fields 
     * Set line number to 0
     * Set Token type to NULL
     */
    
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