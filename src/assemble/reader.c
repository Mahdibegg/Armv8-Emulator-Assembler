#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "reader.h"

void read_line(FILE *f, char *buffer, size_t size) {

    assert(f != NULL);
    assert(buffer != NULL);

    if (fgets(buffer, size, f) == NULL) {
        abort();
    }

    /*
     * Remove newline character if present.
     */
    buffer[strcspn(buffer, "\n")] = '\0';
}

/*
 * Checks if a line contains only whitespace.
 *
 * line: input string to check
 *
 * Returns true if the line is empty or whitespace only.
 */
static bool is_empty_line(const char *line) {

    for (int i = 0; line[i] != '\0'; i++) {
        if (line[i] != ' ' && line[i] != '\t') {
            return false;
        }
    }

    return true;
}