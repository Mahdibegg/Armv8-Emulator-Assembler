#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registers.h"
 
void init_gen_registers(gen_regs *registers) {
 
    // memset function sets all bytes to 0
    memset(registers->r, 0, sizeof(registers->r));
}

void init_spec_registers(spec_reg *registers) {

    // memset function sets all bytes (incl. bool flags) to 0
    memset(registers, 0, sizeof(spec_reg));
}

reg64_t read_xzr(void) {
    return 0;
}
 
reg32_t read_wzr(void) {
    return 0;
}
