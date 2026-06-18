#ifndef NETWORK_H   
#define NETWORK_H

#include <stdbool.h>

/*
 * Get ping function, Return ping as double 
 * Open file using popen() [This actually treats the output in the terminal as a text file, we also run the ping command on Google DNS] 
 * Then read the file line by line until we find "time=" we then read the floating point number in this line, giving us the ping time in ms
 * Then compare this time to the threshold that we set (100ms)
 * If the ping time is larger than 100 then we flash the yellow led, also, if we cannot get an internet connection then we also flash yellow led 
 * If the ping time is less than 100 we flash the green led 
 * We sleep for a set time and then repeat all of the checks 
 */
double get_ping(void);

/*
 * Check DOS fucntion, Return status as bool
 * Open file using popen()  [This actually treats the output in the terminal as a text file, we also run the ping command on Google DNS] 
 * Then read the file line by line until we find number of connections
 * If the number of connections exceed threshold, return true
 * Else false
 */
bool detect_dos(void);

#endif