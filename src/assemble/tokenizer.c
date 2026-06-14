#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble/tokenizer.h"
#include "shared/types.h"

#define MAX_OPERANDS 7

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
     * Null pointer check on buffer and silent return (since nothing to tokenize)
     * Check first word and assign token type accordingly 
     * Then assign the fields of relevent struct within union by splitting string into opcode and operands
     * ":" at end of line indidcates label, "." indicates directive, otherwise instruction
     */

    if (buffer == NULL) {
        return;
    }
    
    char *colon_ptr = strchr(buffer, ':');

    if (buffer[0] == '\0') {
        /* This check for empty line */

        line->token_type = EMPTY;
        line->line_number = line_number;
        return;
    } else if (buffer[0] == '.') {
        /* Check for directive */

        line->token_type = DIRECTIVE;
        line->line_number = line_number;

        
        token_t token = strtok(buffer, " ");
        /* Token = .int, as this is the first word in the string */

        /* Token = next word in line which is the value */
        token = strtok(NULL, " ");

        if (token == NULL) {
            fprintf(stderr, "ERROR: Invalid directive on line: %zu\n", line_number);
            abort();
        }
        /* Allocate memory to value field */
        line->data.directive_data.value = malloc(strlen(token)+1);

        if (line->data.directive_data.value == NULL) {
            fprintf(stderr, "ERROR: Could not allocate directive value\n");
            abort();
        }
        strcpy(line->data.directive_data.value, token);
        return;
    } else if (colon_ptr != NULL) {
        /* Check for label in line */

        line->token_type = LABEL;
        line->line_number = line_number;

        /* Replace : with \0 to end the line so the buffer is now just label name */
        *colon_ptr = '\0';
        
        /* Allocate memory to label */
        line->data.label_data.label = malloc(strlen(buffer) +  1);
        if (line->data.label_data.label == NULL) {
            fprintf(stderr, "Could not allocate label");
            abort();
        }
        
        strcpy(line->data.label_data.label, buffer);
        return;
    } else {
        /* Line is not directive, empty or label, so must be an instruction */

        line->token_type = INSTRUCTION;
        line->line_number = line_number;
        line->data.instruction_data.operand_count = 0;


        /* Allocate memory to the operands array */
        line->data.instruction_data.operands = malloc(sizeof(token_t) * MAX_OPERANDS);
        if (line->data.instruction_data.operands == NULL) {
            fprintf(stderr, "Could not allocate operands");
            abort();
        }

        /* First token/word is going to be opcode */
        token_t opcode = strtok(buffer, " ");
        if (opcode == NULL) {
            fprintf(stderr, "ERROR: Invalid instruction type on line number: %zu", line_number);
            abort();
        }
        

        line->data.instruction_data.opcode = malloc(strlen(opcode) +1);
        if (line->data.instruction_data.opcode == NULL) {
            fprintf(stderr, "Could not allocate opcode for instruction");
            abort();
        }
        strcpy(line->data.instruction_data.opcode, opcode);

        /* Next token is first operand */
        token_t token = strtok(NULL, ", ");
        while (token != NULL) {

            line->data.instruction_data.operands[line->data.instruction_data.operand_count] = malloc(strlen(token) +1);
            
            if (line->data.instruction_data.operands[line->data.instruction_data.operand_count] == NULL) {
                fprintf(stderr, "Could not allocate operand");
                abort();
            }

            strcpy(line->data.instruction_data.operands[line->data.instruction_data.operand_count], token);
            
            line->data.instruction_data.operand_count++;
            token = strtok(NULL, ", ");
        }

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