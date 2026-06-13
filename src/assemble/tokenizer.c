#include <stdio.h>

#include "tokenizer.h"
#include "../shared/types.h"

void free_tokenized_line(tokenized_line_t *tokens_ptr) {
    /* No error since this pointer does not reference any memory */
    if (tokens_ptr == NULL) {
        return;
    }

    /* 
     * Freeing any malloc()ed pointer references within the struct
     *
     * Freeing fields by case checking the token_type
     * Since the .data would not be set for different unions, hence it would be illogical to free unused memory
     */

    switch (tokens_ptr->token_type) {
        case LABEL:
            free(tokens_ptr->data.label_data.label);
            break;

        case DIRECTIVE:
            free(tokens_ptr->data.directive_data.opcode);
            break;

        case INSTRUCTION:
            free(tokens_ptr->data.instruction_data.opcode);

            for (size_t i = 0; i < tokens_ptr->data.instruction_data.operand_count; i++) {
                free(tokens_ptr->data.instruction_data.operands[i]);
            }

            free(tokens_ptr->data.instruction_data.operands);
            break;
    }
    
    free(tokens_ptr);
}