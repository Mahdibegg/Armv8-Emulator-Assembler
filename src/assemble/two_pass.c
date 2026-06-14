#include "assemble/two_pass.h"
#include "shared/types.h"
#include "tokenizer.h"
#include "assemble/symbol_table.h"
#include "assemble/reader.h"
#include "assemble/binary_writer.h"
#include "assemble/encoder.h"

// Fill in 
// static first_pass()

static void second_pass(FILE *input, FILE *output, symbol_table_t st) {
    /* magic number to change */
    char buffer[256];
    size_t line_number = 0;
    addr_t address = 0;

    tokenized_line_t *tokens = init_tokenized_line();

    /*
     * Loop until EOF
     * read_line returns false when EOF is reached
     */
    while (read_line(input, buffer, sizeof(buffer))) {

        /*
         * Skip empty lines
         * read_line normalises empty lines to ""
         */
        if (buffer[0] == '\0') {
            continue;
        }

        /* Clear previous token data */
        clear_tokenized_line(tokens);

        /* Tokenise current line */
        tokenize_line(tokens, buffer, line_number);

        /* Write 32-bit word to output */
        // write_word(out, );

        /* Magic number to change*/
        address += 4;

        line_number++;
    }

    free_tokenized_line(tokens);
}

// Create public function 
// two_pass