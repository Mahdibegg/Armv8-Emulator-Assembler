#ifndef ENCODER_H
#define ENCODER_H

#include "symbol_table.h"
#include "tokenizer.h"
#include "shared/types.h"

/*
 * Encode function abstracts multiple things, further parse to give numerical meaning to each operand/opcode
 * Then identify the right instruction type, in order to use the right instruction field (so correct fields "encoded")
 * Then assemble the encoded fields into a single word type that is returned
 * This whole process falls under "encoding" the fully tokenized but partially parsed tokenized_line_t
 * 
 * st: Symbol table that will be used for lookup across fully processed symbol table
 * tokens: A tokenized + partially parsed struct, tokens is the result of tokenize_line
 */
word_t encode(symbol_table_t st, const tokenized_line_t tokens);

#endif