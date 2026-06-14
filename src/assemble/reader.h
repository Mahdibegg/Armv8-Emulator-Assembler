#ifndef READER_H
#define READER_H

#include <stdio.h>
#include <stdbool.h>

/*
 * Reads a single line from the input file
 *
 * f:       pointer to the input file
 * buffer:  destination buffer to store the line
 * size:    maximum number of characters to read
 *
 * Assert the file and buffer is valid,
 * Abort if reading from the file fails
 */
bool read_line(const FILE *f, char *buffer, size_t size);

#endif