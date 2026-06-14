#ifndef MEMORY_H
#define MEMORY_H

#include "shared/types.h"
#define MEMORY_SIZE (2 * 1024 * 1024)

/* Memory package */
typedef struct {
    byte_t memory[MEMORY_SIZE];
} memory_t;

/* Required functions to be implemented in memory.c */

/*
 * Initialise all bytes in memory to 0
 * zeroes the entire memory array
 */
void init_memory(memory_t *memory);

/*
 * Read/write a word (32 bits), read returns 32 bit (word_t)
 * read_word returns the word stored at address
 * write_word stores value at address
 */
word_t read_word(const memory_t *memory, addr_t address);
void write_word(memory_t *memory, addr_t address, word_t value);

/*
 * Read a 64-bit value as two little-endian words
 * the low word at address, the high word at address + 4
 */
dword_t read_double_word(const memory_t *memory, addr_t address);

/*
 * Write a 64-bit value as two little-endian words
 * the low word at address, the high word at address + 4
 */
void write_double_word(memory_t *memory, addr_t address, dword_t value);

#endif
