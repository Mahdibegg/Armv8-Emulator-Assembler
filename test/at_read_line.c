#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "assemble/reader.h"

#define BUFFER_SIZE 256

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

int main(void) {
    const char *filename = "test_input.txt";
    create_test_file(filename);

    FILE *f = fopen(filename, "r");
    assert(f != NULL);

    char buffer[BUFFER_SIZE];

    /* TEST 1 */
    assert(read_line(f, buffer, BUFFER_SIZE) == true);
    assert(strcmp(buffer, "hello world") == 0);

    /* TEST 2 */
    assert(read_line(f, buffer, BUFFER_SIZE) == true);
    assert(strcmp(buffer, "foo bar") == 0);

    /* TEST 3 */
    assert(read_line(f, buffer, BUFFER_SIZE) == true);
    assert(strcmp(buffer, "baz") == 0);

    /* TEST 4: whitespace-only line -> should become empty string */
    assert(read_line(f, buffer, BUFFER_SIZE) == true);
    assert(strcmp(buffer, "") == 0);

    /* TEST 5: empty line -> should become empty string */
    assert(read_line(f, buffer, BUFFER_SIZE) == true);
    assert(strcmp(buffer, "") == 0);
}
