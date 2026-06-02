#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "IO/io.h"
#include "memory/memory.h"
#include "registers/registers.h"
#include "state.h"


// TEST 1.1: validate_args (valid input, no output)
void test_validate_args_valid_input() {
    char *input;
    char *output;

    char *argv[] = {"./emulate", "test.bin"};
    int argc = 2;

    validate_args(argc, argv, &input, &output);

    assert(strcmp(input, "test.bin") == 0);
    assert(output == NULL);

    printf("validate_args (valid input, no output) PASSED\n");
}

// TEST 1.2: validate_args (valid input, valid output)
void test_validate_args_valid_output() {
    char *input;
    char *output;

    char *argv[] = {"./emulate", "test.bin", "out.out"};
    validate_args(3, argv, &input, &output);

    assert(strcmp(input, "test.bin") == 0);
    assert(strcmp(output, "out.out") == 0);

    printf("validate_args (valid input, valid output) PASSED\n");
}

// TEST 1.3: validate_args (invalid input)
void test_validate_args_invalid_input() {
    char *input;
    char *output;

    char *argv[] = {"./emulate", "bad.txt"};  // WRONG extension
    int argc = 2;

    printf("validate_args (invalid input) [should EXIT]\n");

    validate_args(argc, argv, &input, &output);

    // If this line runs, test failed
    printf("\nERROR: validate_args did NOT exit\n");
}

// TEST 1.4: validate_args (invalid output)
void test_validate_args_invalid_output(){
    char *input;
    char *output;

    char *argv[] = {"./emulate", "test.bin", "bad.txt"};  // WRONG extension
    int argc = 3;

    printf("validate_args (invalid output) [should EXIT]\n");

    validate_args(argc, argv, &input, &output);

    // If this line runs, test failed
    printf("\nERROR: validate_args did NOT exit\n");
}

// TEST 1.5: validate_args (too many arguments)
void test_validate_args_too_many_args(){
    char *input;
    char *output;

    char *argv[] = {"./emulate", "test.bin", "out.out", "extra"};  // too many
    int argc = 4;

    printf("validate_args (too many args) [should EXIT]\n");

    validate_args(argc, argv, &input, &output);

    // If this line runs, test failed
    printf("\nERROR: validate_args did NOT exit\n");
}

// TEST 1.6: validate_args (too few arguments)
void test_validate_args_too_few_args(){
    char *input;
    char *output;

    char *argv[] = {"./emulate"};  // too few
    int argc = 1;

    printf("validate_args (too few args) [should EXIT]\n");

    validate_args(argc, argv, &input, &output);

    // If this line runs, test failed
    printf("\nERROR: validate_args did NOT exit\n");
}

// TEST 2.1: setup_output (no output)
void test_setup_output_null() {
    FILE *out = setup_output(NULL);

    assert(out == stdout);

    printf("setup_output (no output) PASSED\n");
}

// TEST 2.2: setup_output (valid output)
void test_setup_output_valid() {
    FILE *out = setup_output("test.out");

    assert(out != NULL);

    fprintf(out, "hello world\n");
    fclose(out);

    printf("setup_output (valid output) PASSED\n");
}

// TEST 2.3: setup_output (invalid path)

void test_setup_output_invalid() {
    printf("setup_output (invalid path) (should EXIT)\n");

    // This directory does not exist, fopen will fail
    setup_output("nonexistent_dir/test.out");

    // If execution reaches here, FAILURE
    printf("\nERROR: setup_output did NOT exit\n");
}

// TEST 3.1: binary_loader (valid path)
void test_binary_loader_valid() {

    machine_state_t state;
    init_memory(&state.memory);

    // temporary file
    FILE *f = fopen("test.bin", "wb");

    byte_t test_bytes[4] = {0x22, 0x08, 0x00, 0x91};
    fwrite(test_bytes, sizeof(byte_t), 4, f);
    fclose(f);

    binary_loader(&state, "test.bin");

    // check memory correctly loaded
    assert(state.memory.memory[0] == 0x22);
    assert(state.memory.memory[1] == 0x08);
    assert(state.memory.memory[2] == 0x00);
    assert(state.memory.memory[3] == 0x91);

    printf("binary_loader (valid path) PASSED\n");
}

// TEST 3.2: binary_loader (invalid file)
void test_binary_loader_invalid() {
    printf("binary_loader (invalid file) (should EXIT)\n");

    machine_state_t state;

    // invalid file path, fopen fails
    binary_loader(&state, "nonexistent_file.bin");

    // If reached, FAIL
    printf("ERROR: binary_loader did NOT exit\n");
}

// TEST 3.3: binary_loader with an empty file
void test_binary_loader_empty(){
    printf("binary_loader (empty file) (should EXIT)\n");

    machine_state_t state;

    // temporary file
    FILE *f = fopen("test.bin", "wb");
    fclose(f);

    binary_loader(&state, "test.bin");

    printf("ERROR: binary_loader did NOT exit\n");
}

// TEST 4.1: output_write
void test_output_write() {

    machine_state_t state;

    // initialise components
    init_memory(&state.memory);
    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);

    // set memory word
    write_word(&state.memory, 0, 0x91000822);

    // set some registers
    state.general_registers.r[1] = 0x1234;
    state.special_registers.pc = 8;

    // set PSTATE flags
    state.special_registers.psr.n_flag = 0;
    state.special_registers.psr.z_flag = 1;
    state.special_registers.psr.c_flag = 0;
    state.special_registers.psr.v_flag = 0;

    printf("output_write test output:\n\n");

    output_write(&state, stdout);

    printf("\nEnd output\n");

    printf("output_write EXECUTED (manual check required)\n");
}

// MAIN TEST RUNNER
int main() {

    // Tests that should pass 
    
    test_validate_args_valid_input();
    test_validate_args_valid_output();
    
    test_setup_output_null();
    test_setup_output_valid();
    
    test_binary_loader_valid();
    
    test_output_write();
    
    // Tests that should fail
    //test_validate_args_invalid_input();
    //test_validate_args_invalid_output();
    //test_validate_args_too_many_args();
    //test_validate_args_too_few_args();
    //test_setup_output_invalid();
    //test_binary_loader_invalid();
    //test_binary_loader_empty();

    printf("\nAll IO tests completed\n");
    return 0;
}
