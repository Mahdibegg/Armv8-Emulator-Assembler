#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// shared types
// typedef sizes: register, byte, instruction size (for readability)

typedef uint32_t instr_t;
typedef uint64_t addr_t;

typedef uint64_t reg64_t;
typedef uint32_t reg32_t;

typedef uint8_t byte_t;
typedef uint32_t word_t;
typedef uint64_t dword_t;

#endif