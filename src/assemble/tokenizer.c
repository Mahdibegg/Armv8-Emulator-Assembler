#include <stdio.h>

#include "tokenizer.h"
#include "../shared/types.h"

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