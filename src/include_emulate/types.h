#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define WORD_BITS 32
#define DWORD_BITS 64

// shared types
// typedef sizes: register, byte, instruction size (for readability)

// memory referencing type definitions
typedef uint32_t instr_t;
typedef uint32_t addr_t;

// register sizes
typedef uint64_t reg64_t;
typedef uint32_t reg32_t;

// using clearer names for different bit sizes
typedef bool bit_t;
typedef uint8_t byte_t;
typedef uint32_t word_t;
typedef uint64_t dword_t;

// signed 32/64 bit
typedef int32_t sword_t;
typedef int64_t sdword_t;

#endif