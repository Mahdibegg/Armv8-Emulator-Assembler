#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "symbol_table.h"
#include "types.h"

#define INITIAL_CAPACITY 2

 /*
  * Pair consists of label and address
  * label is string and address is addr_t
  */
 typedef struct {
    char *label;
    addr_t address;
 } Pair;

/*
 * ADT: Symbol Table (Implemented as Dynamic array of pairs)
 * Fields: Size, Capacity, Pairs(array of pairs)
 * Pair is its own struct 
 */
struct symbol_table {
    Pair *data;   // This is a pointer the array of pairs
    int size;     // Number of pairs in array
    int capacity; // Maximum number of pairs in array
};

/*
 * Implementing Functions of Symbol Table
 */

symbol_table_t symbol_table_create(void) {
    // Allocate memory for the symbol table 
    symbol_table_t st = malloc(sizeof(struct symbol_table));

    //Check if the allocation was successfull
    if (st == NULL) {
        // Exit program
        fprintf(stderr, "ERROR: Allocation of memory to symbol table was unsuccessful");
        abort();
    }
    // Defensive programming assertion check
    assert(st != NULL);

    // now allocate memory for the data
    st->data = malloc(sizeof(Pair) * INITIAL_CAPACITY);

    // check if allocation was successfull
    if (st->data == NULL) {
        fprintf(stderr, "ERROR: Allocation of memory to symbol table data was unsuccessful");
        // Free the memory allocated to Symbol Table then abort
        free(st);
        abort();
    }

    // Defensive programming assertion check
    assert(st->data != NULL);

    // Initialise the other fields
    st->size = 0; 
    st->capacity = INITIAL_CAPACITY;

    return st;
}

bool symbol_table_add(symbol_table_t st, const char *label, addr_t address) {
    // we can call the function to see if the label is already in the symbol table if so we will not add this pair
    if (symbol_table_contains(st, label)) {
        return false;
    }
    // we want to check for a resize at the start and then we want to add if safe 
    if (resize_needed(st)) {
        // call grow function and then add 
        // CALL GROW
        // Add pair  
        // we want to add on to the end of the array so what we do is add at index size onto the array
        st->data[st->size].label = label;
        st->data[st->size].address = address;
        // update size
        st->size++;
        return true;
    } else {
        // we can simply add the pair
        st->data[st->size].label = label;
        st->data[st->size].address = address;
        // update size
        st->size++;
        return true;
    }
}

