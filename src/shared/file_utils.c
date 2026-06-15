#include "file_utils.h"
#include <string.h>

bool has_extension(const char *filename, const char *ext) {
    if (!filename || !ext) {
        return false;
    }

    const char *dot = strrchr(filename, '.');
    if (!dot) return false;

    return strcmp(dot, ext) == 0;
}