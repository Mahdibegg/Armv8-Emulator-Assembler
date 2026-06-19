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

void net_set_led(led_controller_t *leds, net_status_t net_stat) {
    /* 
     * NET_DOS case is handled first since its a priority check
     *
     * DOS attack: Flicker red
     * No network connection: No LED light
     * Unstable network: Yellow LED
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

led_controller_t *init_led(void) {}

void free_controller(led_controller_t *leds) {}

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

/*
 * Open the file usig popen() to treat terminal output as text
 * Ping Google Dns with a single packet count (safe option)
 * Ignore text until "% packet loss" and return packetloss
 * No connection returns 100.0 (Full packet loss)
 */
static double get_packet_loss(void) {
    char net_info[MAX_LINE_LENGTH];

    double packet_loss = 100.0;

    FILE *net_stats_file = popen("ping -c 1 -W 8.8.8.8 2>&1", "r");

    if (net_stats_file == NULL) {
        return packet_loss;
    }

    while (fgets(net_info, sizeof(net_info), net_stats_file) != NULL) {

        char *packet_ptr = strstr(net_info, "% packet loss");
        int transmitted;
        int received;
        double loss;

        if ( sscanf(net_info, "%d packets transmitted, %d received, %lf%% packet loss", &transmitted, &received, &loss ) == 3) {
            packet_loss = loss;
        }

        break;
    }

    pclose(net_stats_file);
    return packet_loss;

}

static unsigned long read_u_long_from_file(const char *file_path) {
    FILE *fp = fopen(file_path, "r");

    unsigned long value = 0;

    if (fp == NULL) {
        perror("File did not open");
        return 0;
    }

    if (fscanf(fp, "%lu", &value) != 1) {
        value = 0;
    }

    fclose(fp)
    return value;
}

static unsigned long get_rx_bytes(void) {
    return read_u_long_from_file(
         "/sys/class/net/wlan0/statistics/rx_bytes"
    );
}

static unsigned long get_tx_bytes(void) {
    return read_u_long_from_file(
         "/sys/class/net/wlan0/statistics/tx_bytes"
    );
}

net_sample_t net_sample_get(void) {
    /* Make the struct and initialise the fields with helper functions */
    net_sample_t sample;

    sample.ping_ms = get_ping();
    sample.packet_loss = get_packet_loss();
    sample.rx_bytes = get_rx_bytes();
    sample.tx_bytes = get_tx_bytes();

    return sample;
}