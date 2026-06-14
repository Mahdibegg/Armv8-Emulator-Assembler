#ifndef BIT_H
#define BIT_H

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#define MEMORY_SIZE (2 * 1024 * 1024)

// extract bits in range low -> high on words (32 bit)
dword_t extract_bits(dword_t dword, unsigned low, unsigned high);

/* 

2 sign bit extract function (MSB extractors)
one is a 32 bit version and the other is a 64 bit version
should use higher calls (extract bit at MSB position for 32,64 bit respectively)

*/

bit_t sign_bit_32(word_t word);

bit_t sign_bit_64(dword_t dword);

// memory package
typedef struct {
    byte_t memory[MEMORY_SIZE];
} memory_t;

// read a word (32 bits), read returns 32 bit (word_t)
word_t read_word(const memory_t *memory, addr_t address);

// sign extend N bits to 64 bit
int64_t sign_extend(dword_t value, unsigned bits);

#endif