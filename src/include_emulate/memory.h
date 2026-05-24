#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"
#define MEMORY_SIZE (2 * 1024 * 1024)

// memory package
typedef struct {
    byte_t memory[MEMORY_SIZE];
} memory_t;

// REQUIRED functions to be implemented in memory.c

// initialise all bytes in memory to 0
void init_memory(memory_t *memory);

// read/write a word (32 bits), read returns 32 bit (word_t)
word_t read_word(const memory_t *memory, addr_t address);
void write_word(memory_t *memory, addr_t address, word_t value);

#endif