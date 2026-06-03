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


int main(void) {

    printf("Running load/store tests...\n\n");

    printf("UNSIGNED OFFSET TESTS --->\n");
    unsigned_offset_str_64_test();

    return 0;
}