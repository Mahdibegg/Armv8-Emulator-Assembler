#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include "state.h"
#include "pipeline/decode_struct.h"

exec_result_t execute_data_processing(machine_state_t *state, decoded_instr_t instr);

#endif