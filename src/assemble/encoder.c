#include "encoder.h"
#include "shared/instruction_fields.h"

/*
 * Assemble_directive will assemble the directive token type
 * 
 * tokens: Returns the directive value from the struct union
 */
static word_t assemble_directive(const tokenized_line_t tokens) {
    return tokens.data.directive_data.value;
}

word_t encode(symbol_table_t st, const tokenized_line_t tokens) {
    /* Value to be written to .bin file */
    word_t encoded_value = 0;

    /* 
     * Identify DIRECTIVE, LABEL, INSTRUCTION, EMPTY token types 
     * Assembling of parsed tokens are separated, since their assembly is different
     * Before INSTRUCTIONS are assembled, they must be further parsed
     */
    switch (tokens.token_type) {
        case DIRECTIVE:
            encoded_value = assemble_directive(tokens);
            break;
        case INSTRUCTION:
            // - TODO -------------
            /* Alias handling function */

            /* Identify instruction type before selecting correct struct to fill fields in */

            /* Build field for correct struct */

            /* Take build field result to re-assign encoded_value using an instruction_assembler */
            break;
    }    

    return encoded_value;
}