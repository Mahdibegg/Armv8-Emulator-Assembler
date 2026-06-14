#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include "emulate/state.h"
#include "shared/decode.h"
#include "shared/types.h"
#include "shared/instruction_fields.h"

// takes bits and executes correct data processing type instruction
exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr);

#endif