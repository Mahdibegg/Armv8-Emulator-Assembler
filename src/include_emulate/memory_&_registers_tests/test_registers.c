#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "../registers/registers.h"

// initialising general purpose register tests 
void init_gen_registers_test(void) {

     //Testing general registers
    gen_regs g_regs;

    // initialising the general purpose registers 
    init_gen_registers(&g_regs);

    // check if each register is equal to 0
    for (unsigned i = 0; i < REG_NUM; i++) {
        assert(g_regs.r[i] == 0);
    }

    printf("all general registers were initialised to 0 : PASSED\n");
}


// initialising special register test
void init_spec_registers_test(void) {

    printf("Testing init_spec_registers....\n");

    spec_reg special;

    // initialise the special registers
    init_spec_registers(&special);

    //checking all the flag values only z_flag should be true 
    assert(special.pc == 0);
    assert(special.psr.z_flag == true);
    assert(special.psr.c_flag == false);
    assert(special.psr.n_flag == false);
    assert(special.psr.v_flag == false);
    printf("The special registers were initialised correctly : PASSED\n");
}


// read write x registers test
void read_write_x_registers_test(void) {

    printf("Testing read and write registers...\n");

    gen_regs g_regs;
    // initialise the general purpose registers 
    init_gen_registers(&g_regs);


    // write to and read from each register 
    for(unsigned i = 0; i < REG_NUM; i++) {
        dword_t value = (dword_t)(i + 100);

        write_x_register(&g_regs, i, value);

        reg64_t result = read_x_register(&g_regs,i);

        assert(result == value);
    }

    // also test some larger values 

    write_x_register(&g_regs, 1, 0xFFFFFFFFFFFFFFFF);
    assert(read_x_register(&g_regs, 1) == 0xFFFFFFFFFFFFFFFF);

    write_x_register(&g_regs, 10, 0x123456789ABCDEF0);
    assert(read_x_register(&g_regs, 10) == 0x123456789ABCDEF0);

    printf("read/write x registers : PASSED\n");
}

// read write 32 registers test
void read_write_32_registers_test(void) {

    printf("Testing read and write registers...\n");

    gen_regs g_regs;

    // initialise the general purpose registers 
    init_gen_registers(&g_regs);

    // write to and read from each registers 
    for(unsigned i = 0; i < REG_NUM; i++) {
        word_t value = (word_t)(i+100);

        write_w_register(&g_regs, i, value);

        reg32_t result = read_w_register(&g_regs, i);

        assert(result == value);

    }

    // test for some larger values 

    write_w_register(&g_regs, 1, 0xFFFFFFFF);
    assert(read_w_register(&g_regs, 1) == 0xFFFFFFFF);

    write_w_register(&g_regs, 10, 0x12345678);
    assert(read_w_register(&g_regs, 10) == 0x12345678);

    printf("read/write 32 registesr : PASSED\n");
}

// read write PC register tests
void read_write_PC_register_test(void) {

    printf("Testign read and write to PC...\n");

    spec_reg special;

    //initialise the special registers first 
    init_spec_registers(&special);

    // writing to and reading from the PC 
    write_pc(&special,  0x1000);
    assert(read_pc(&special) ==  0x1000);

    write_pc(&special,  0xABCDEF1234567890);
    assert(read_pc(&special) ==  0xABCDEF1234567890);

    printf("read/write to PC register : PASSED\n");

}

// reading zero register tests
void read_zero_registers_test(void) {

    printf("Testing read xzr and wzr...\n");

    assert(read_xzr == 0);
    assert(read_wzr == 0);

    printf("zero registers hold correct value : PASSED\n");
}


// running all tests together
int main(void) {

    printf("Running Register Tests...\n");

    init_gen_registers_test();

    init_spec_registers_test();

    read_write_32_registers_test();

    read_write_x_registers_test();

    read_write_PC_register_test();

    read_zero_registers_test();

    printf("\nAll register tests passed\n");

    return 0;   
}