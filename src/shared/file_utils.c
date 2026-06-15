#include "file_utils.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

bool has_extension(const char *filename, const char *ext) {
    if (!filename || !ext) {
        return false;
    }

    const char *dot = strrchr(filename, '.');
    if (!dot) return false;

    return strcmp(dot, ext) == 0;
}

FILE *open_file_or_exit(const char *filename, const char *mode) {
    FILE *f = fopen(filename, mode);
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s' with mode '%s' (%s)\n", filename, mode, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return f;
}