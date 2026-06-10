#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

#include "types.h"


/*
 * Symbol Table ADT.

 * Stores label address pairs generated from the first pass of he assembler 
 * Internal implementation: Dynamic array of pairs 
 */

typedef struct symbol_table *symbol_table_t;

/*
 * Create Empty symbol table.
 */
symbol_table_t symbol_table_create(void);

/*
 * Add label address pair to Symbol Table.
 * Return false if already exists.
 */
bool symbol_table_add(symbol_table_t st, const char *label, addr_t address);

/*
 * Check if label is already in the Symbol Table.
 */
bool symbol_table_contains(symbol_table_t st, const char *label); 

/*
 * Returns address associated with label.
 */
addr_t symbol_table_get(symbol_table_t st, const char *label); 

/*
 * Print The symbol_table used for debugging.
 */ 
void symbol_table_print(symbol_table_t st);

/*
 * Frees all memory used by Symbol Table.
 */
void symbol_table_free(symbol_table_t st);

#endif