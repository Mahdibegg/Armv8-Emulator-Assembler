#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "registers/registers.h"
#include "state.h"

void assert_general_registers_are_zero(const machine_state_t *state);

void assert_special_registers_initialised_except_pc(const machine_state_t *state, dword_t expected_pc);

void assert_only_x_register_changed(const machine_state_t *state, unsigned changed_index, dword_t expected_value);

#endif