#include "emulate/emulate.h"

int main(int argc, char **argv) {

    // Argument handling
    char *inputfile;
    char *outputfile;

    validate_args(argc, argv, &inputfile, &outputfile);

    // Selecting output
    // IMPORTANT! If an output file exists, file is opened and will close at the end
    // If no output file, using stdout, not recommended to fopen or fclose stdout
    FILE *out = setup_output(outputfile);

    // Initialising machine state
    machine_state_t state;   // dont initialise anything

    // Initialise memory and registers
    init_memory(&state.memory);
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    state.halted = 0;

    // Load input file into state memory
    binary_loader(&state, inputfile);

    // Run FDE cycle until halt
    run_pipeline(&state);

    // Write the results to output
    output_write(&state, out);

    // Closing the output file IF it exists
    // Could close at the end of output_write()? both should be the same
    if (out != stdout) {
        fclose(out);
    }

    return EXIT_SUCCESS;
}
