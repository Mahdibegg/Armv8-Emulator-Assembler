#ifndef BIT_H
#define BIT_H

#include <stdio.h>
#include <stdlib.h>

#include "types.h"

/* extract bits in range low -> high on words (32 bit) */
dword_t extract_bits(dword_t dword, unsigned low, unsigned high);

/* 
* 2 sign bit extract function (MSB extractors)
* one is a 32 bit version and the other is a 64 bit version
* should use higher calls (extract bit at MSB position for 32,64 bit respectively)
*/

bit_t sign_bit_32(word_t word);

bit_t sign_bit_64(dword_t dword);

/* sign extend N bits to 64 bit */
int64_t sign_extend(dword_t value, unsigned bits);

/* split a 32 bit word into 4 little-endian bytes (least significant first) */
void word_to_bytes_le(word_t word, byte_t bytes[4]);

/* combine 4 little-endian bytes (least significant first) into a 32 bit word */
word_t bytes_to_word_le(const byte_t bytes[4]);

#endif