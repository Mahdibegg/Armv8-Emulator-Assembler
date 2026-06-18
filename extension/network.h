#ifndef NETWORK_H   
#define NETWORK_H

#include <stdbool.h>

/*
 * network_status -> this function is run in the main function and continuously checks the ping time and check for dos
 * Then flashes LEDs accordingly
 */

void network_status(void);

/*
 * Check DOS fucntion, Return status as bool
 * Open file using popen()  [This actually treats the output in the terminal as a text file, we also run the ping command on Google DNS] 
 * Then read the file line by line until we find number of connections
 * If the number of connections exceed threshold, return true
 * Else false
 */
bool detect_dos(void);

#endif