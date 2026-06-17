#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "emulate/memory.h"
#include "shared/bit.h"

void init_memory(memory_t *memory) {
    /* memset function sets all bytes to 0 */
    memset(memory->memory, 0, MEMORY_SIZE);
}

word_t read_word(const memory_t *memory, addr_t address) {
    /* validate address size and return value */
    if (address <= MEMORY_SIZE - 4) {
        /* combine the 4 little-endian bytes at address to form a word_t */
        return bytes_to_word_le(&memory->memory[address]);
    } else {
        /* print error message and exit program */
        fprintf(stderr, "Memory read out of bounds - address %" PRIu32 " doesn't exist\n", address);
        exit(EXIT_FAILURE);
    }
}

void write_word(memory_t *memory, addr_t address, word_t value) {
    /* validate address size and write value */
    if (address + 3 < MEMORY_SIZE) {
        /* split value into 4 little-endian bytes written directly into memory */
        word_to_bytes_le(value, &memory->memory[address]);
    } else {
        /* print error message and exit program */
        fprintf(stderr, "Memory write out of bounds - address %" PRIu32 " doesn't exist\n", address);
        exit(EXIT_FAILURE);
    }
}

dword_t read_double_word(const memory_t *memory, addr_t address) {
    dword_t lo = read_word(memory, address);
    dword_t hi = read_word(memory, address + 4);
    return lo | (hi << 32);
}

void write_double_word(memory_t *memory, addr_t address, dword_t value) {
    write_word(memory, address, (word_t) value);
    write_word(memory, address + 4, (word_t) (value >> 32));
}
