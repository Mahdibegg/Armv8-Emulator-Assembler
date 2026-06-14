#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include "emulate/state.h"
#include "emulate/decode_struct.h"
#include "shared/types.h"
#include "shared/instruction_fields.h"

/*
 * Takes bits and executes correct data processing type instruction
 * decodes the instruction then performs the operation on the machine state
 */
exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr);

#endif