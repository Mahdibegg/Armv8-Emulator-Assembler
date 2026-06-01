#ifndef BIT_H
#define BIT_H

#include <stdio.h>
#include <stdlib.h>

#include "types.h"

// extract bits in range low -> high on words (32 bit)
word_t extract_bits(instr_t instruction, unsigned low, unsigned high);

// sign extend N bits to 64 bit
int64_t sign_extend(dword_t value, unsigned bits);

#endif