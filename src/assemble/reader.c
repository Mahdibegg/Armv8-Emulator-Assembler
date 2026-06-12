#include <string.h>
#include <stdlib.h>
#include <assert.h>

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