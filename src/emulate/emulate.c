#include "emulate/emulate.h"

int main(int argc, char **argv) {
    /* argument handling */
    char *inputfile;
    char *outputfile;

    validate_args(argc, argv, &inputfile, &outputfile);

    /* initialising machine state */
    machine_state_t state;   /* dont initialise anything */

    /* initialise memory and registers */
    init_memory(&state.memory);
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    state.halted = 0;

    /* load input file into state memory */
    binary_loader(&state, inputfile);

    /*
     * selecting output
     * IMPORTANT! if an output file exists, file is opened and will close at the end
     * if no output file, using stdout, not recommended to fopen or fclose stdout
     */
    FILE *out = setup_output(outputfile);

    /* run FDE cycle until halt */
    run_pipeline(&state);

    /* write the results to output */
    output_write(&state, out);

    /*
     * closing the output file IF it exists
     * could close at the end of output_write()? both should be the same
     */
    if (out != stdout) {
        fclose(out);
    }

    return EXIT_SUCCESS;
}