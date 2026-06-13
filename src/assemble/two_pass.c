#include "two_pass.h"
#include "stdio.h"

#include "assemble/two_pass.h"

// Fill in 
// static first_pass()

static void second_pass(FILE *in, FILE *out, symbol_table_t symtab) {
    /* magic number to change */
    char buffer[256];
    size_t line_number = 0;
    addr_t address = 0;

    tokenized_line_t *tokens = init_tokenized_buffer();

    /*
     * Loop until EOF
     * read_line returns false when EOF is reached
     */
    while (read_line(in, buffer, sizeof(buffer))) {
        line_number++;
    }

    free_tokenized_buffer(tokens);
}

// Create public function 
// two_pass