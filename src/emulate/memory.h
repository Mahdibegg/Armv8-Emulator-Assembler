#ifndef MEMORY_H
#define MEMORY_H

#include "shared/types.h"
#include "shared/bit.h"

// REQUIRED functions to be implemented in memory.c

// initialise all bytes in memory to 0
void init_memory(memory_t *memory);

// write a word (32 bits), read returns 32 bit (word_t)
void write_word(memory_t *memory, addr_t address, word_t value);

// read_double_word reads a 64-bit value as two little-endian words:
// the low word at address, the high word at address + 4
dword_t read_double_word(const memory_t *memory, addr_t address);

// write_double_word writes a 64-bit value as two little-endian words:
// the low word at address, the high word at address + 4
void write_double_word(memory_t *memory, addr_t address, dword_t value);

#endif