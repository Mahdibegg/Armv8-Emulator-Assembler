#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "network_status.h"

#define PING_LIMIT 100.0
#define MAX_LINE_LENGTH 256

#define GPIO_CHIP "/dev/gpiochip0"

#define RED_PIN 17
#define GREEN_PIN 27
#define BLUE_PIN 22

#define FLICKER_DELAY 0.1

#define ERROR_WAIT

/* 
 * Represents a data sample to be pushed onto buffer + analysis
 */
typedef struct {
    double ping_ms; /* Ping time of the packet sent */
    double packet_loss; /* Packet loss returned by running the ping command */
    unsigned long rx_bytes; /* number of bytes recieved by rpi */
    unsigned long tx_bytes; /* number of bytes transmitted by rpi */
} net_sample_t;

/* 
 * Represents reliable data to conclude a network status result
 */
typedef struct {
    double avg_ping_ms; /* Average ping time over last 10 samples */
    double avg_packet_loss; /* Average packet loss over last 10 samples */
    double avg_rx_rate; /* Average number of bytes recieved by rpi in last 10 samples */
    double avg_tx_rate; /* Average number of bytes transmitted by rpi in last 10 smaples */
} net_analysis_t;

/*
 * Enums to classify network status for LED output
 */
typedef enum {
    NET_DOWN,
    NET_UNSTABLE,
    NET_STABLE,
    NET_DOS
} net_stat;

/* 
 * LED section - initialisation, reference freeing, led setter
 */

/* 
 * Turn off all LEDs
 * 
 * leds: Reference to controller to clear all LEDs
 */
static void clear_leds(led_controller_t *leds) {}

/* 
 * Hold and LED light
 * 
 * colour: The colour of LED that is being switched on
 */
static void static_led_colour(gpiod_line *colour){}

/* 
 * Flicker an LED light 
 * 
 * colour: The colour of LED that is being switched on
 */
static void flicker_led(gpiod_line *colour) {}

/*
 * Get led lines for controller
 * Return true if successful, false if not
 *
 * leds: Reference to controller in order to get
 */
static bool get_led_lines(led_controller_t *leds) {
    /* References to the lines in which LED colours can be outputted */
    leds->red = gpiod_chip_get_line(leds->chip, RED_PIN);
    leds->blue = gpiod_chip_get_line(leds->chip, BLUE_PIN);
    leds->green = gpiod_chip_get_line(leds->chip, GREEN_PIN);

    /* Failed to retrieve line, return false for error handling loop to continue */
    if (leds->red == NULL ||
        leds->yellow == NULL ||
        leds->green == NULL) {
        return false;
    }

    return true;
}

void net_set_led(led_controller_t *leds, net_status_t net_stat) {
    /* 
     * NET_DOS case is handled first since its a priority check
     *
     * DOS attack: Flicker red
     * No network connection: No LED light
     * Unstable network: Blue LED
     * Stable network: Green LED
     */
    switch (net_stat) {
        case NET_DOS:
            flicker_led(leds->red);
            break;
        case NET_DOWN:
            clear_leds(leds);
            break;
        case NET_UNSTABLE:
            static_led_colour(leds->yellow);
            break;
        case NET_STABLE:
            static_led_colour(leds->green);
            break;
    }
}

led_controller_t *init_led(void) {
    /*
     * Error handling here is done in a loop
     * So as this runs from startup, it tries to allocate memory for itself 
     * Rather than print an error and end the service
     *
     * For example if you plug in a rgb pin while the rpi is on 
     * This program just retrieves the update in real time and continues running
     */
    led_controller_t *leds = malloc(sizeof(struct led_controller_t));

    /* Instead of returning an error, continue attempts at allocating memory */
    while (leds == NULL) {
        leds = malloc(sizeof(struct led_controller_t));

        if (leds != NULL) {
            break;
        }

        sleep(ERROR_WAIT);
    }

    /* If you need to free the memory for whatever reason, have initial NULL values for safety */
    leds->chip = NULL;
    leds->red = NULL;
    leds->green = NULL;
    leds->blue = NULL;

    /* Open chip access for chip field in controller */
    leds->chip = gpiod_chip_open(GPIO_CHIP);

    while (leds->chip == NULL) {
        leds->chip = gpiod_chip_open(GPIO_CHIP);

        if (leds->chip != NULL) {
            break;
        }

        sleep(ERROR_WAIT);
    }

    /* 
     * get_led_lines returns false if lines aren't retrieved
     * so while you can't get led lines sleep the program
     */
    while (!get_led_lines(leds)) {
        sleep(ERROR_WAIT;)
    }

    return leds;
}

void free_controller(led_controller_t *leds) {
    /* If there was no controller to begin with, just return */
    if (leds == NULL) {
        return;
    }

    /* Turn off all LED colours */
    clear_leds(leds);

    /* Error free line release, otherwise invalid references are being freed */
    if (leds->red != NULL) {
        gpiod_line_release(leds->red);
    }

    if (leds->yellow != NULL) {
        gpiod_line_release(leds->yellow);
    }

    if (leds->green != NULL) {
        gpiod_line_release(leds->green);
    }

    if (leds->chip != NULL) {
        gpiod_chip_close(leds->chip);
    }
}

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