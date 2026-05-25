#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "memory.h"

// type for separating bytes of a word
typedef struct {
    byte_t bytes[4];
} four_byte_arr_t;

// helper function to extract bytes from value
static four_byte_arr_t extract_bytes(word_t value) {

    // little endian format extraction using shifts
    four_byte_arr_t result;
    result.bytes[0] = (byte_t) (value & 0xFF);
    result.bytes[1] = (byte_t) ((value >> 8) & 0xFF);
    result.bytes[2] = (byte_t) ((value >> 16) & 0xFF);
    result.bytes[3] = (byte_t) ((value >> 24) & 0xFF);
    
    return result;
}

void init_memory(memory_t *memory) {
    
    // memset function sets all bytes to 0
    memset(memory->memory, 0, MEMORY_SIZE);
}

word_t read_word(const memory_t *memory, addr_t address) {
    
    // validate address size and return value
    if (address + 3 < MEMORY_SIZE) {
        
        // mem variable shorthand as const (no change)
        const byte_t *mem = memory->memory;

        // combine 4 bytes to form a word_t
        word_t value = (word_t) mem[address] | 
            (word_t) mem[address + 1] << 8 | 
            (word_t) mem[address+2] << 16 |
            (word_t) mem[address+3] << 24;
    } else {
        
        // print error message and exit program
        fprintf(stderr, "Memory read out of bounds - address " PRIu32 " doesn't exist\n", address);
        exit(EXIT_FAILURE);
    }
}

void write_word(memory_t *memory, addr_t address, word_t value) {
    
    // validate address size and write value 
    if (address + 3 < MEMORY_SIZE) {

        // using extract_bytes to subset 32 bits in 4 bytes
        four_byte_arr_t divided_bytes = extract_bytes(value);
        byte_t *val_bytes = divided_bytes.bytes;
        
        // writing value into memory in 4 separate bytes
        for (int i = address; i < address + 4; i++) {
            memory->memory[i] = val_bytes[i-address];
        }
    } else {
        
        // print error message and exit program
        fprintf(stderr, "Memory write out of bounds - address " PRIu32 " doesn't exist\n", address);
        exit(EXIT_FAILURE);
    }
}