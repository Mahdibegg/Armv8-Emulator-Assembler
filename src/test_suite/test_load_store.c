#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "instruction_types/load_store/load_store.h"
#include "state.h"
#include "registers/registers.h"
#include "memory/memory.h"
#include "types.h"

#define BLUE "\033[34m"
#define WHITE "\033[0m"


// UNSIGNED OFFSET TESTS

// TEST 1.1: unsigned_offset_str_64
// Testing that a 64-bit STR writes the whole doubleword at Xn + (imm12 * 8)
static void unsigned_offset_str_64_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
    init_memory(&state.memory);

    write_x_register(&state.general_registers, 1, 0x1000);
    write_x_register(&state.general_registers, 2, 0xDEADBEEFCAFEBABE);

    /*

       Single data transfer STR (unsigned offset, 64-bit):
       bit 31 = 1, sf = 1, fixed 111, U = 1, L = 0 (store)
       imm12 = 2 (uoffset = 2 * 8 = 16), xn = 1, rt = 2
       Address = 0x1000 + 16 = 0x1010, stores X2 as a doubleword

    */

    decoded_instr_t instr;
    instr.type = INSTR_LOAD_STORE;
    instr.instr = (0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) |
                  (0x1 << 24) | (0x0 << 22) | (2 << 10) | (1 << 5) | 2;

    exec_result_t result = execute_load_store(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_word(&state.memory, 0x1010) == 0xCAFEBABE);
    assert(read_word(&state.memory, 0x1014) == 0xDEADBEEF);

    // load/store must not touch PC or the condition flags
    assert(read_pc(&state.special_registers) == 0x0);
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);

    printf("Unsigned offset STR 64-bit: PASSED\n");
}

// TEST 1.2: unsigned_offset_ldr_64
// Testing that a 64-bit LDR reads the whole doubleword at Xn + (imm12 * 8)
static void unsigned_offset_ldr_64_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
    init_memory(&state.memory);

    write_x_register(&state.general_registers, 1, 0x1000);
    write_word(&state.memory, 0x1010, 0xCAFEBABE);
    write_word(&state.memory, 0x1014, 0xDEADBEEF);

    /*

       Single data transfer LDR (unsigned offset, 64-bit):
       bit 31 = 1, sf = 1, fixed 111, U = 1, L = 1 (load)
       imm12 = 2 (uoffset = 16), xn = 1, rt = 3
       Address = 0x1010, loads doubleword into X3

    */

    decoded_instr_t instr;
    instr.type = INSTR_LOAD_STORE;
    instr.instr = (0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) |
                  (0x1 << 24) | (0x1 << 22) | (2 << 10) | (1 << 5) | 3;

    exec_result_t result = execute_load_store(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 3) == 0xDEADBEEFCAFEBABE);

    // load/store must not touch PC or the condition flags
    assert(read_pc(&state.special_registers) == 0x0);
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);

    printf("Unsigned offset LDR 64-bit: PASSED\n");
}

// TEST 1.3: unsigned_offset_ldr_32
// Testing that a 32-bit LDR loads a word and zero-extends it into the X register
static void unsigned_offset_ldr_32_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
    init_memory(&state.memory);

    write_x_register(&state.general_registers, 1, 0x2000);
    write_word(&state.memory, 0x2000, 0x11223344);

    /*

       Single data transfer LDR (unsigned offset, 32-bit):
       bit 31 = 1, sf = 0, fixed 111, U = 1, L = 1 (load)
       imm12 = 0, xn = 1, rt = 4
       Loads word 0x11223344 into W4, upper 32 bits of X4 cleared

    */

    decoded_instr_t instr;
    instr.type = INSTR_LOAD_STORE;
    instr.instr = (0x1 << 31) | (0x0 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) |
                  (0x1 << 24) | (0x1 << 22) | (0 << 10) | (1 << 5) | 4;

    exec_result_t result = execute_load_store(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 4) == 0x0000000011223344);

    // load/store must not touch PC or the condition flags
    assert(read_pc(&state.special_registers) == 0x0);
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);

    printf("Unsigned offset LDR 32-bit (zero-extends): PASSED\n");
}

// TEST 1.4: unsigned_offset_str_32
// Testing that a 32-bit STR writes only the low word and leaves the next word untouched
static void unsigned_offset_str_32_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
    init_memory(&state.memory);

    write_x_register(&state.general_registers, 1, 0x3000);
    write_x_register(&state.general_registers, 2, 0xAABBCCDD11223344);

    /*

       Single data transfer STR (unsigned offset, 32-bit):
       bit 31 = 1, sf = 0, fixed 111, U = 1, L = 0 (store)
       imm12 = 0, xn = 1, rt = 2
       Stores only the low word of X2 (0x11223344) at 0x3000,
       leaving 0x3004 untouched

    */

    decoded_instr_t instr;
    instr.type = INSTR_LOAD_STORE;
    instr.instr = (0x1 << 31) | (0x0 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) |
                  (0x1 << 24) | (0x0 << 22) | (0 << 10) | (1 << 5) | 2;

    exec_result_t result = execute_load_store(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_word(&state.memory, 0x3000) == 0x11223344);
    assert(read_word(&state.memory, 0x3004) == 0x0);

    // load/store must not touch PC or the condition flags
    assert(read_pc(&state.special_registers) == 0x0);
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);

    printf("Unsigned offset STR 32-bit (low word only): PASSED\n");
}

// TEST 1.5: unsigned_offset_ldr_32_scaled
// Testing that a 32-bit LDR scales imm12 by 4 (not 8) when forming the address
static void unsigned_offset_ldr_32_scaled_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
    init_memory(&state.memory);

    write_x_register(&state.general_registers, 1, 0x2000);
    write_word(&state.memory, 0x200C, 0x55667788);

    /*

       Single data transfer LDR (unsigned offset, 32-bit) with non-zero imm12:
       bit 31 = 1, sf = 0, fixed 111, U = 1, L = 1 (load)
       imm12 = 3 (uoffset = 3 * 4 = 12), xn = 1, rt = 5
       Address = 0x2000 + 12 = 0x200C (would be wrong if scaled by 8)

    */

    decoded_instr_t instr;
    instr.type = INSTR_LOAD_STORE;
    instr.instr = (0x1 << 31) | (0x0 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) |
                  (0x1 << 24) | (0x1 << 22) | (3 << 10) | (1 << 5) | 5;

    exec_result_t result = execute_load_store(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 5) == 0x55667788);

    // load/store must not touch PC or the condition flags
    assert(read_pc(&state.special_registers) == 0x0);
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);

    printf("Unsigned offset LDR 32-bit (scaled by 4): PASSED\n");
}

// TEST 2.1: pre_index_ldr_64
// Testing that a pre-indexed LDR writes the updated base back to Xn before transferring
static void pre_index_ldr_64_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
    init_memory(&state.memory);

    write_x_register(&state.general_registers, 1, 0x4000);
    write_word(&state.memory, 0x4008, 0x05060708);
    write_word(&state.memory, 0x400C, 0x01020304);

    /*

       Single data transfer LDR (pre-index, 64-bit):
       bit 31 = 1, sf = 1, fixed 111, U = 0, L = 1 (load)
       bit 21 = 0, simm9 = 8, I = 1 (pre), bit 10 = 1, xn = 1, rt = 5
       Address = 0x4000 + 8 = 0x4008, X1 written back to 0x4008

    */

    decoded_instr_t instr;
    instr.type = INSTR_LOAD_STORE;
    instr.instr = (0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) |
                  (0x0 << 24) | (0x1 << 22) | (8 << 12) | (0x1 << 11) | (0x1 << 10) | (1 << 5) | 5;

    exec_result_t result = execute_load_store(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 5) == 0x0102030405060708);
    assert(read_x_register(&state.general_registers, 1) == 0x4008);

    // load/store must not touch PC or the condition flags
    assert(read_pc(&state.special_registers) == 0x0);
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);

    printf("Pre-index LDR 64-bit (write-back): PASSED\n");
}

// TEST 2.2: post_index_ldr_64
// Testing that a post-indexed LDR transfers at the old base, then updates Xn
static void post_index_ldr_64_test(void) {

    machine_state_t state;

    init_gen_registers(&state.general_registers);
    init_spec_registers(&state.special_registers);
    init_memory(&state.memory);

    write_x_register(&state.general_registers, 1, 0x5000);
    write_word(&state.memory, 0x5000, 0x33334444);
    write_word(&state.memory, 0x5004, 0x11112222);

    /*

       Single data transfer LDR (post-index, 64-bit):
       bit 31 = 1, sf = 1, fixed 111, U = 0, L = 1 (load)
       bit 21 = 0, simm9 = 16, I = 0 (post), bit 10 = 1, xn = 1, rt = 6
       Address = 0x5000 (old base), then X1 written back to 0x5010

    */

    decoded_instr_t instr;
    instr.type = INSTR_LOAD_STORE;
    instr.instr = (0x1 << 31) | (0x1 << 30) | (0x1 << 29) | (0x1 << 28) | (0x1 << 27) |
                  (0x0 << 24) | (0x1 << 22) | (16 << 12) | (0x0 << 11) | (0x1 << 10) | (1 << 5) | 6;

    exec_result_t result = execute_load_store(&state, instr);

    assert(result == EXEC_NEXT);
    assert(read_x_register(&state.general_registers, 6) == 0x1111222233334444);
    assert(read_x_register(&state.general_registers, 1) == 0x5010);

    // load/store must not touch PC or the condition flags
    assert(read_pc(&state.special_registers) == 0x0);
    assert(state.special_registers.psr.z_flag == true);
    assert(state.special_registers.psr.n_flag == false);
    assert(state.special_registers.psr.c_flag == false);
    assert(state.special_registers.psr.v_flag == false);

    printf("Post-index LDR 64-bit (write-back): PASSED\n");
}

int main(void) {

    printf("Running load/store tests...\n\n");

    printf("UNSIGNED OFFSET TESTS --->\n");
    unsigned_offset_str_64_test();
    unsigned_offset_ldr_64_test();
    unsigned_offset_ldr_32_test();
    unsigned_offset_str_32_test();
    unsigned_offset_ldr_32_scaled_test();

    printf("\nINDEXED TESTS --->\n");
    pre_index_ldr_64_test();
    post_index_ldr_64_test();

    return 0;
}