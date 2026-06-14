#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "shared/types.h"

/*
 * This enum is for distinguishing the token type
 * Useful for later on when filling in separate tokens based on the type
 */
typedef enum {INSTRUCTION, DIRECTIVE, LABEL, EMPTY} tokenized_type_t;

/* 
 * Tokenized_instr_t is a struct that sums up all the tokens and has a token type
 *
 * Token_type identifies which type the token is for further parsing to be made easier 
 * 
 * Tokens section
 * 
 * Label is the token that belongs to a LABEL type
 * Opcode belongs to DIRECTIVE and INSTRUCTION token_types
 * Operands belongs to the INSTRUCTION token_types, which is a list of operands the instruction takes
 * Operand_count keeps a track of number of operands the instruction has
 * Line_number keeps a track of line number, so error handling for invalid syntax in further passing becomes easier
 */
typedef struct {
    tokenized_type_t token_type;
    size_t line_number;

    union {
        /* LABEL */
        struct {
            token_t label;
        } label_data;

        /* DIRECTIVE */
        struct {
            token_t opcode;
        } directive_data;

        /* INSTRUCTION */
        struct {
            token_t opcode;
            tokens_t operands;
            size_t operand_count;
        } instruction_data;

    } data;
} tokenized_line_t;

/*
 * Initialise the tokenized_lint_t struct at the beginning of the assembling process
 */
tokenized_line_t *init_tokenized_line(void);

/*
 * Clearing the tokenized_buffer fields for every new line
 *
 * tokenized_line_buffer: reference to the tokenized line buffer to clear per line you write
 */
void clear_tokenized_line(tokenized_line_t *line);

/*
 * Returns a pointer reference to a malloc()ed tokenized_line_t struct
 *
 * toknized_line_buffer: reference to the tokenized line buffer the result is written to
 * char *buffer: takes the buffer storing each line from the file via read_line output
 * line_number: stores this as meta data within tokenized_line_t for future error messages during parsing
 */
void tokenize_line(tokenized_line_t *line, char *buffer, size_t line_number);

/*
 * Free memory allocated by tokenize() for the tokenized_line_t struct
 *
 * tokens_ptr: pointer reference to the tokenized_line_t struct created by tokenize()
 */
void free_tokenized_line(tokenized_line_t *line);

#endif