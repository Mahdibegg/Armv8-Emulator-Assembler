#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>

#include "assemble/symbol_table.h"
#include "shared/types.h"

#define INITIAL_CAPACITY 16U

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
 * Fields: Size, Capacity, Pairs (array of pairs)
 * Pair is its own struct 
 */
struct symbol_table {
    Pair *data;   /* This is a pointer the array of pairs */ 
    size_t size;     /* Number of pairs in array */
    size_t capacity; /* Maximum number of pairs in array */
};

/*
 * Implementing public functions of Symbol Table
 */

symbol_table_t symbol_table_create(void) {
    /* Allocate memory for the symbol table  */
    symbol_table_t st = malloc(sizeof(struct symbol_table));

    /* Check if the allocation was successfull */
    if (st == NULL) {
        /* Exit program */
        fprintf(stderr, "ERROR: Allocation of memory to symbol table was unsuccessful\n");
        abort();
    }

    /* Now allocate memory for the data */
    st->data = malloc(sizeof(Pair) * INITIAL_CAPACITY);

    /* check if allocation was successfull */
    if (st->data == NULL) {
        fprintf(stderr, "ERROR: Allocation of memory to symbol table data was unsuccessful\n");
        /* Free the memory allocated to Symbol Table then abort */
        free(st);
        abort();
    }

    /* Defensive programming assertion check */
    assert(st->data != NULL);

    /* Initialise the other fields */
    st->size = 0; 
    st->capacity = INITIAL_CAPACITY;

    return st;
}

/*
 * Helper function for symbol_table_add
 *
 * Returns true or false based on whether a a resize is needed.
 */
static bool resize_needed(symbol_table_t st) {
    return (st->size  >= st->capacity);
}

/*
 * Helper function for symbol_table_add
 *
 * Grows Symbol Table if Resize was required.
 */
static void symbol_table_grow(symbol_table_t st) {
    /* First Get the new capacity */
    size_t new_capacity = st->capacity * 2;

    Pair *temp_data = realloc(st->data, sizeof(Pair) * new_capacity);

    /* Check if reallocation was successful */
    if (temp_data == NULL) {
        /* abort */ 
        fprintf(stderr, "ERROR: Reallocation of data was unsuccessful\n");
        abort();
    }

    /* now it is safe to reassign data */
    st->data = temp_data;
    st->capacity = new_capacity;
}

bool symbol_table_add(symbol_table_t st, const char *label, addr_t address) {
    /* we can call the function to see if the label is already in the symbol table if so we will not add this pair */
    if (symbol_table_contains(st, label)) {
        return false;
    }
    /*  we want to check for a resize at the start and then we want to add if safe */ 
    if (resize_needed(st)) {
        /* call grow function */
        symbol_table_grow(st);
    } 

    /* 
     * we can simply add the pair 
     * we need to copy the string into label instead of assigning it the pointer value 
     */ 
    st->data[st->size].label = malloc(strlen(label) + 1);
    if (st->data[st->size].label == NULL) {
        fprintf(stderr, "ERROR: Allocation of memory for label was unsuccessful\n");
        abort();
    }
    /* if it was successful we can copy the label string into the initialised label pointer now */
    strcpy(st->data[st->size].label, label);
    st->data[st->size].address = address;
    /* update size */
    st->size++;
    
    return true;
}

bool symbol_table_contains(symbol_table_t st, const char *label) {
    /*  We have to iterate through the list until we find the label, or until we reach the end which is up to size-1 index */
    for (size_t i = 0; i < st->size; i++) {
        if (strcmp(st->data[i].label, label) == 0) {
            return true;
        }
    }

    return false;
}

addr_t symbol_table_get(symbol_table_t st, const char *label) {
    /* iterate through symbol table, find the pair and return the address */
    for (size_t i = 0; i < st->size; i++) {
        if (strcmp(st->data[i].label, label) == 0) {
            return st->data[i].address;
        }
    }
    /* otherwise throw error */ 
    fprintf(stderr, "ERROR: Label %s is not in the symbol table\n", label);
    abort();
}

void symbol_table_print(symbol_table_t st, FILE *out) {
    /* first check if the out file is valid */ 
    if (out == NULL) {
        fprintf(stderr, "ERROR: Provided file is invalid\n");
        abort();
    }

    /* Iterate through symbol table and print each Pair to the output file */ 
    for (size_t i = 0; i < st->size; i++) {
        fprintf(out, "(%s, 0x%" PRIx32 ")\n",
             st->data[i].label, st->data[i].address);
    }
}

void symbol_table_free(symbol_table_t st) {
    /*  Defensive programming check (Making sure that st exists) */
    if (st == NULL) {
        fprintf(stderr, "ERROR: symbol table does not exist (in an attempt to free symbol table)\n");
        abort();
    }

    /* to free the symbol table you first need to free meory allocated to each label */ 
    for (size_t i=0; i < st->size; i++) {
        free(st->data[i].label);
    }

    /* Now we can free the memory allocatead to data and then the memory allocated to symbol table */ 
    free(st->data);
    free(st);
}