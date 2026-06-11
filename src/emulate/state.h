#ifndef STATE_H
#define STATE_H

#include <stdbool.h>
#include "shared/types.h"
#include "emulate/memory.h"
#include "emulate/registers.h"

// struct contains all data about the machine
// memory, all registers, halt value
typedef struct {
    memory_t memory;
    gen_regs general_registers;
    spec_reg special_registers;
    bit_t halted;
} machine_state_t;

#endif