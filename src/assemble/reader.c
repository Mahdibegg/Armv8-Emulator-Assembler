#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "assemble/reader.h"

/*
 * Trims leading and trailing whitespace from a line
 *
 * str: string to be modified in place
 */
static void trim_whitespace(char *str) {
    assert(str != NULL);

    char *start = str;

    while (*start == ' ' || *start == '\t') {
        start++;
    }

    memmove(str, start, strlen(start) + 1);

    int len = strlen(str);

    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t')) {
        str[len - 1] = '\0';
        len--;
    }
}

/*
 * Checks if a line contains only whitespace
 *
 * line: input string to check
 *
 * Returns true if the line is empty or whitespace only
 */
static bool is_empty_line(const char *line) {
    assert(line != NULL);

    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] != ' ' && line[i] != '\t') {
            return false;
        }
    }

    return true;
}

bool read_line(FILE *f, char *buffer, size_t size) {
    if (f == NULL) {
        fprintf(stderr, "ERROR: File pointer is NULL");
        abort();
    }

    if (buffer == NULL) {
        fprintf(stderr, "ERROR: Buffer pointer is NULL");
        abort();
    }

    if (fgets(buffer, size, f) == NULL) {
        if (feof(f)){
            buffer[0] = '\0';
            return false;
        }
        fprintf(stderr, "ERROR: File could not be read");
        abort();
    }

    /* Remove newline character if present */
    buffer[strcspn(buffer, "\n")] = '\0';

    /* Normalise whitespace */
    trim_whitespace(buffer);

    /* Skip empty lines by forcing empty string */
    if (is_empty_line(buffer)) {
        buffer[0] = '\0';
    }

    return true;
}