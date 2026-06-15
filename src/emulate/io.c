#include "emulate/io.h"
#include "shared/types.h"
#include "emulate/memory.h"
#include "shared/file_utils.h"

/*
 * Helper function for checking file format
 * used to output error if suffix is not correct
 */
static int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return 0;

    /* string lengths and suffix lengths for comparison */
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);

    if (lensuffix > lenstr) return 0;

    /* move along string until reach where suffix should be, and compare */
    return strcmp(str + (lenstr - lensuffix), suffix) == 0;
}

/*
 * Validate arguments passed in
 * length of arguments
 */
void validate_args(int argc, char **argv, char **input, char **output) {
    /*
     * check number of arguments
     * expected format: ./emulate <input .bin> <output .out>, max arguments 2, min arguments 1
     */
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: ./emulate <input.bin> OR ./emulate <input.bin> <output.out>\n");
        exit(EXIT_FAILURE);
    }

    /* assign input file */
    *input = argv[1];

    /* assign optional output file */
    if (argc == 3) {
        *output = argv[2];
    } else {
        /* ignore if no second command line argument passed */
        *output = NULL;
    }

    /* validate input file extension using helper function ends_with */
    if (!has_extension(*input, ".bin")) {
        fprintf(stderr, "File input error: input file must have .bin extension\nUse: ./emulate <filename>.bin\n");
        exit(EXIT_FAILURE);
    }

    /* validate output file extension (if not provided short circuit) */
    if (*output && !has_extension(*output, ".out")) {
        fprintf(stderr, "File output error: output file must have .out extension\nUse: ./emulate <filename>.bin <filename>.out\n");
        exit(EXIT_FAILURE);
    }
}

/* Select whether to output to stdout or to an output file */
FILE *setup_output(char *outputfile) {
    /* if no output file is provided -> use stdout to output to terminal */
    if (outputfile == NULL) {
        return stdout;
    }

    /* attempt to open the output file, in order to check whether it exists */
    FILE *out = fopen(outputfile, "w");

    /* fail if unsuccessful (if it doesn't exist then exit the program) */
    if (out == NULL) {
        perror("File error: file cannot be opened\n");
        exit(EXIT_FAILURE);
    }

    return out;
}

/* Load the binary input file into state (specifically the memory field, since instructions will be fetched) */
void binary_loader(machine_state_t *state, char *inputfile) {
    /* attempt to open file to check existence */
    FILE *file = fopen(inputfile, "rb");

    /* if the file does not exit the program to prevent further crashes */
    if (file == NULL) {
        perror("Error opening input file\n");
        exit(EXIT_FAILURE);
    }

    /* read bytes into the memory array, capped at memory size */
    size_t bytes_read = fread(state->memory.memory, sizeof(byte_t), MEMORY_SIZE, file);

    /* closed file after reading */
    fclose(file);

    /* fail if nothing was read to state */
    if (bytes_read == 0) {
        fprintf(stderr, "Error: failed to read input file\n");
        exit(EXIT_FAILURE);
    }
}

/* Write to given output stream (command line output since no provided .out file) */
void output_write(machine_state_t *state, FILE *out) {
    /* registers */
    fprintf(out, "Registers :\n");

    /* format register as X[2 digit number], contents as 16 digit hex value padded with 0's */
    for (int i = 0; i < 31; i++) {
        fprintf(out,
            "X%02d = %016lx\n", i, state->general_registers.r[i]
        );
    }

    /* PC, formatted as 16 digit hex value padded with 0's */
    fprintf(out,
        "PC = %016lx\n",
        state->special_registers.pc
    );

    /* PSTATE, formatted as combination of 4 characters */
    fprintf(out,
        "PSTATE : %c%c%c%c\n",

        state->special_registers.psr.n_flag ? 'N' : '-',  /* negative flag */
        state->special_registers.psr.z_flag ? 'Z' : '-',  /* zero flag */
        state->special_registers.psr.c_flag ? 'C' : '-',  /* carry flag */
        state->special_registers.psr.v_flag ? 'V' : '-'   /* overflow flag */
    );

    /* memory, address and value formatted as 8 digit hex values */
    fprintf(out, "Non-zero memory:\n");

    for (addr_t addr = 0; addr < MEMORY_SIZE; addr += 4) {
        word_t value = read_word(&state->memory, addr);

        if (value != 0) {
            fprintf(out,
                "0x%08x: 0x%08x\n", addr, value
            );
        }
    }
}
