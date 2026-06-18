#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#define PING_LIMIT 100.0
#define MAX_LINE_LENGTH 256

/* Led functions to create static/flicker effect for selected RGB values */
static void static_led_colour(void) // TODO - ADD PARAMETERS
static void flicker_led_colour(void)

/*
 * Open the file using popen() to treat terminal output as text
 * Ping Google DNS with a single packet count (safe option)
 * Ignore text until "time=<ping_time>" and return ping_time
 * No connection returns -1.0
 */
static double get_ping(void) {

    /* Reading line buffer */
    char net_info[MAX_LINE_LENGTH];

    /* 
     * Result to be returned, a default value of -1.0 indicates no connection 
     * Since ping cannot be negative
     */
    double ping_time = -1.0;

    /* Get the file pointer for the terminal output to read */
    FILE *ping_file = popen("ping -c 1 -W 1 8.8.8.8 2>&1", "r");
    
    /* No open file means no connection so no updated ping_time hence return */
    if (ping_file == NULL) {
        return ping_time;
    }

    /* 
     * Use strstr to return pointer to the ping_time
     * Non-null pointer results in an available float for ping_time
     */
    while (fgets(net_info, sizeof(net_info), ping_file) != NULL) {
        /* Pointer to "time=" */
        char *ping_ptr = strstr(net_info, "time=");

        if (ping_ptr != NULL) {
            /* Use sscanf to retrieve ping time in given format */
            sscanf(ping_ptr, "time=%lf", &ping_time);
            break;
        }
    }

    pclose(ping_file);

    return ping_time;
}