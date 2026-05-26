#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "../memory/memory.h"

// initialising all memory bytes to 0 test

void init_memory_test(void) {

    memory_t memory;

    //initialise memory 
    init_memory(&memory);

    //check each byte to see if it is set to 0
    for(addr_t i = 0; i < MEMORY_SIZE; i++) {
        assert(memory.memory[i] == 0);
    }

    printf("All bytes of memory set to 0: PASSED\n");
}

// read/write tests
void read_write_word_test(void) {

    memory_t memory;

    //initialise memory
    init_memory(&memory);

    //Testing address 0
    write_word(&memory, 0 , 0x12345678);

    assert(read_word(&memory, 0) == 0x12345678);

    //Testing another address
    write_word(&memory, 12, 0xABCDEF12);

    assert(read_word(&memory, 12) == 0xABCDEF12);

    //Testing zero value
    write_word(&memory, 0, 0x00000000);
    
    assert(read_word(&memory, 0) == 0x00000000);

    printf("Read and write words to memory: PASSED\n");
}

// Memory locations are independent tests
void memory_locations_are_independent_test(void) {

    memory_t memory;

    //initialising memory
    init_memory(&memory);

    //consecutive writes to memory shouldnt affect other addresses
    write_word(&memory, 0, 0x12345678);
    write_word(&memory, 4, 0xABCDEF12);

    assert(read_word(&memory, 0) == 0x12345678);
    assert(read_word(&memory, 4) == 0xABCDEF12);

    printf("Memory locations are independent: PASSED\n");
}

// write near the end of memory test
void write_near_end_of_memory_test(void) {

    memory_t memory;

    //initialising memory 
    init_memory(&memory);

    //Get the last possible address to write at, since a word is 4 bytes we have:
    addr_t last_possible_address = MEMORY_SIZE - 4;

    //Write and read from last possible address test
    write_word(&memory, last_possible_address, 0x12345678);

    assert(read_word(&memory, last_possible_address) == 0x12345678);

    printf("Write near end of memory test: PASSED\n");
}

//Running all tests together 
int main(void) {

    init_memory_test();

    read_write_word_test();

    memory_locations_are_independent_test();

    write_near_end_of_memory_test();

    printf("\nAll memory tests passed\n");

    return 0;
}