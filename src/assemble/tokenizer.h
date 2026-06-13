#ifndef TOKENIZER_H
#define TOKENIZER_H

/*
 * This enum is for distinguishing the token type
 * Useful for later on when filling in separate tokens based on the type
 */
typedef enum {INSTRUCTION, DIRECTIVE, LABEL} tokenized_type_t;

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
 *
 * Meta deta section
 * 
 * Operand_count keeps a track of number of operands the instruction has
 * Line_number keeps a track of line number, so error handling for invalid syntax in further passing becomes easier
 */
typedef struct {
    /* Token type  */
    tokenized_type_t token_type;
    
    /* LABEL */
    token_t label;

    /* INSTRUCTION */
    token_t opcode; /* ONLY FIELD FOR DIRECTIVE */
    tokens_t operands;

    /* META DATA */
    size_t operand_count;
    size_t line_number;
} tokenized_line_t;

/*
 * Returns a pointer reference to a malloc()ed tokenized_line_t struct
 *
 * char *buffer - takes the buffer storing each line from the file via read_line output
 * line_number - stores this as meta data within tokenized_line_t for future error messages during parsing
 */
tokenized_line_t *tokenize(char *buffer, size_t line_number);

/*
 * Free memory allocated by tokenize() for the tokenized_line_t struct
 *
 * tokens_ptr - pointer reference to the tokenized_line_t struct created by tokenize()
 */
void free_tokenized_line(tokenized_line_t *tokens_ptr);

#endif