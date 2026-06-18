#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>


/*
 * Blueprint:
 * Open the file using popen() [This actually treats the output in the terminal as a text file, we also run the ping command on Google DNS] 
 * Then read the file line by line until we find "time=" we then read the floating point number in this line, giving us the ping time in ms
 * Then compare this time to the threshold that we set (100ms)
 * If the ping time is larger than 100 then we flash the yellow led, also, if we cannot get an internet connection then we also flash yellow led 
 * If the ping time is less than 100 we flash the green led 
 * We sleep for a set time and then repeat all of the checks  
 */

 #define PING_LIMIT 100.00

 /*
  * Function prototype for flashing the RGBS
  */
 void flash_yellow(void);
 void flash_green(void);

 double get_ping(void) {
    /* Initialise the ping time and the information buffer which is supposed to the store the lines from the terminal output */
    char response[256]
 }