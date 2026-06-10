#include <stdio.h>
#include <stdlib.h>

#include "symbol_table.h"
#include "types.h"

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
    Pair *data;
    int size;
    int capacity;
}