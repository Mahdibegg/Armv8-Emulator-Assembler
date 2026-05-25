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

reg64_t read_pc(const spec_reg *registers) {
    return registers->pc;
}
 
void write_pc(spec_reg *registers, dword_t value) {
    registers->pc = (reg64_t) value;
}

reg64_t read_xzr(void) {
    return 0;
}
 
reg32_t read_wzr(void) {
    return 0;
}
