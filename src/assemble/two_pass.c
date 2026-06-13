#include "assemble/two_pass.h"
#include "shared/types.h"
#include "tokenizer.h"
#include "assemble/symbol_table.h"
#include "assemble/reader.h"
#include "assemble/binary_writer.h"
#include "assemble/encoder.h"

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

        /*
         * Skip empty lines
         * read_line normalises empty lines to ""
         */
        if (buffer[0] == '\0') {
            continue;
        }

        /* Clear previous token data */
        clear_tokenized_buffer(tokens);

        /* Tokenise current line */
        tokenized_buffer(tokens, buffer, line_number);

        /*
         * LABEL:
         * Skip labels in second pass
         */
        if (tokens->token_type == LABEL) {
            continue;
        }

        uint32_t word = 0;

        /*
         * DIRECTIVE:
         * Stub for now, TODO 
         */
        if (tokens->token_type == DIRECTIVE) {
            word = 0;
        }

        /*
         * INSTRUCTION:
         * Stub for now, TODO
         */
        if (tokens->token_type == INSTRUCTION) {
            word = 0;
        }

        /* Write 32-bit word to output */
        write_word(out, word);

        /* magic number to change*/
        address += 4;

        line_number++;
    }

    free_tokenized_buffer(tokens);
}

// Create public function 
// two_pass