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
     * Freeing all fields without token type case checking
     * Since fields are initialised with NULL, hence no free() errors 
     */

    /* Free label and opcode tokens */
    free(tokens_ptr->label);
    free(tokens_ptr->opcode);
    

    /* Free each token within the tokens (array of strings) */
    for (size_t i = 0; i < tokens_ptr->operand_count; i++) {
        free(tokens_ptr[i]);
    }

    /* Free pointer to operands string */
    free(tokens_ptr->operands);

    /* Free reference to actual struct */
    free(tokens_ptr);
}