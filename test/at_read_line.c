#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "assemble/reader.h"

/* Helper function to write a temporary test file */
static void create_test_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    assert(f != NULL);

    fprintf(f, "   hello world   \n");     /* leading/trailing whitespace */
    fprintf(f, "\t\tfoo bar\n");           /* tabs at beginning */
    fprintf(f, "baz\t\t\n");               /* tabs at end */
    fprintf(f, "   \t   \n");              /* whitespace-only line */
    fprintf(f, "\n");                      /* empty line */
    fprintf(f, "last line\n");             /* normal line */

    fclose(f);
}
