#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define WORD_BITS 32
#define DWORD_BITS 64

/*
 * Shared types between emulate and assemble 
 * Typedef sizes: register, byte, instruction size (for readability)
 */

/*
 * Memory referencing type definitions
 */
typedef uint32_t instr_t;
typedef uint32_t addr_t;

/*
 * Register sizes
 */
typedef uint64_t reg64_t;
typedef uint32_t reg32_t;

/*
 * Clearer names for different bit sizes
 */
typedef bool bit_t;
typedef uint8_t byte_t;
typedef uint32_t word_t;
typedef uint64_t dword_t;

/*
 * Signed 32/64 bit
 */
typedef int32_t sword_t;
typedef int64_t sdword_t;

/*
 * Types shared for tokens
 */
typedef char *token_t;
typedef char **tokens_t;

#endif