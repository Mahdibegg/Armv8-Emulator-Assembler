#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registers.h"

void init_gen_registers(gen_regs *registers) {

    // memset function sets all bytes to 0
    memset(registers->r, 0, REG_NUM * sizeof(reg64_t));
}

void init_spec_registers(spec_reg *registers) {

    // memset function sets all bytes (incl. bool flags) to 0
    memset(registers, 0, sizeof(spec_reg));

    // Z flag is initially set on CPU startup
    registers->psr.z_flag = true;
}

reg64_t read_x_register(const gen_regs *registers, unsigned index) {

    // validate register index and return value
    if (index == REG_NUM) {

        // zero register is reserved
        return 0;
    } else if (index < REG_NUM) {

        return registers->r[index];
    } else {

        // print error message and exit program
        fprintf(stderr, "Register read out of bounds (index X%u does not exist)\n", index);
        exit(EXIT_FAILURE);
    }
}

void write_x_register(gen_regs *registers, unsigned index, dword_t value) {

    // validate register index and write value
    if (index < REG_NUM) {

        registers->r[index] = (reg64_t) value;
    } else if (index ==  REG_NUM) {
        return;
    } else {

        // print error message and exit program
        fprintf(stderr, "Register write out of bounds (index X%u does not exist)\n", index);
        exit(EXIT_FAILURE);
    }
}

reg32_t read_w_register(const gen_regs *registers, unsigned index) {

    // validate register index and return value
    if (index == REG_NUM) {

        // zero register is reserved
        return 0;
    } else if (index < REG_NUM) {

        // mask off upper 32 bits to extract lower word of 64 bit register
        return (reg32_t) (registers->r[index] & 0xFFFFFFFF);
    } else {

        // print error message and exit program
        fprintf(stderr, "Register read out of bounds (index W%u does not exist)\n", index);
        exit(EXIT_FAILURE);
    }
}

void write_w_register(gen_regs *registers, unsigned index, word_t value) {

    // validate register index and write value
    if (index < REG_NUM) {

        // cast zero-extends to 64 bit, clearing upper 32 bits as required
        registers->r[index] = (reg64_t) value;
    } else if (index == REG_NUM) {
        return;
    } else {

        // print error message and exit program
        fprintf(stderr, "Register write out of bounds (index W%u does not exist)\n", index);
        exit(EXIT_FAILURE);
    }
}

reg64_t read_pc(const spec_reg *registers) {

    return registers->pc;
}

void write_pc(spec_reg *registers, dword_t value) {

    registers->pc = (reg64_t) value;
}

// reg64_t read_xzr(void) {

//     return 0;
// }

// reg32_t read_wzr(void) {
    
//     return 0;
// }

void write_pstate(spec_reg *registers, bit_t n, bit_t z, bit_t c, bit_t v) {

    registers->psr.n_flag = n;
    registers->psr.z_flag = z;    
    registers->psr.c_flag = c;
    registers->psr.v_flag = v;
}