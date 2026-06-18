#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#define PING_LIMIT 100.00
#define MAX_LINE_LENGTH 256

/*
 * Function prototype for flashing the RGBS
 */
static void flash_blue(void);
static void flash_green(void);

static double get_ping(void) {
    /*
    * Blueprint:
    * Open the file using popen() [This actually treats the output in the terminal as a text file, we also run the ping command on Google DNS] 
    * Then read the file line by line until we find "time=" we then read the floating point number in this line, giving us the ping time in ms
    * Then return the ping time that we found.
    * If the file does not open and we have not connection then we return -1.0
    */

    /* 
     * Initialise the ping time and the information buffer which is supposed to the store the lines from the terminal output 
     * Initialise ping time with -1.0 which indicates no connection
     */
    char net_info[MAX_LINE_LENGTH];
    double ping_time = -1.0;

    /* Get the file pointer for the file (terminal) that we want to read (the name is the command and the mode is r for reading) */
    FILE *ping_file = popen("ping -c 1 -W 1 8.8.8.8 2>&1", "r");
    
    /* Check if the file opened, if it didn't then there is no connectoin so we return -1 to indicate no connection */

    if (ping_file == NULL) {
        return -1.0;
    }

    /* 
     * We need to read from the ping file until we find time= in the line somewhere, we can use strstr to return the pointer to this part of the line
     * If a non null pointer is returned then we can read the floating point number after "time=" which gives us the ping time( then we can break )
     */

    while (fgets(net_info, sizeof(net_info), ping_file) != NULL) {
        /* Now we get the pointer to "time=" and check if it is null */
        char *ping_ptr = strstr(net_info, "time=");

        if (ping_ptr != NULL) {
            /*
             * Want to read the part just after and store it into ping time
             * so use sscanf
             */
            sscanf(ping_ptr, "time %lf", &ping_time);
            break;
        }
    }

    /*
     * Use pclose to close the file 
     * Return the ping time
     */
    pclose(ping_file);

    return ping_time;
}

/*
 * Static helper function for checking if ther is no network connection
 * Makes use of get_ping()
 */

static bool is_connection(void) {
    /* True if ping is -1 otherwise false */
    return (get_ping() != -1);
}