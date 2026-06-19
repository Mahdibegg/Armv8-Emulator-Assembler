#ifndef NETWORK_H   
#define NETWORK_H

#include <stdbool.h>
#include <gpiod.h>

#define SAMPLE_HISTORY_SIZE 10

#define PROGRAM_WAIT 1

/* 
 * Represents full GPIO chip for access
 */
typedef struct {
    struct gpiod_chip *chip; /* Open GPIO device */

    struct gpiod_line *red; /* Reserved for DOS detection */
    struct gpiod_line *blue; /* Reserved for UNSTABLE  */
    struct gpiod_line *green; /* Reserved for STABLE */
} led_controller_t;

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
} net_status_t;

/*
 * Represent the sampling history (like a partial data structure)
 */
typedef struct {
    net_sample_t array[SAMPLE_HISTORY_SIZE]; /* Array to store all samples in the last N frames */
    size_t next_index; /* Next position to push the sample onto */

    net_sample_t popped; /* Previously removed sample (where the last one was pushed) */
} net_sample_history_t;

/*
 * Enums to classify network interface type 
 */
typedef enum {
    NET_IFACE_NONE,
    NET_IFACE_WLAN0,
    NET_IFACE_ETH0
} net_interface_t;

/*
 * Update LED colour after obtaining new status
 *
 * leds: Reference to global controller, allow LED to update colour
 * net_stat: Returned status of network, this will choose LED option
 */
void net_set_led(led_controller_t *leds, net_status_t net_stat);

/* 
 * Led functions to create static/flicker effect for selected RGB values
 */
led_controller_t *init_led(void);

/*
 * Controller references freed from memory
 *
 * leds: Reference to LEDs that have to be freed before you exit program
 */
void free_controller(led_controller_t *leds);

/*
 * Initialise the history data structure to store sample history
 */
net_sample_history_t *init_history(void);

/*
 * Free the history data structure to avoid memory leaks
 */
void free_history(net_sample_history_t *history);

/*
 * Initialise the analysis history of previous N samples
 */
net_analysis_t *init_net_analysis(void);

/*
 * Free the analysis data structure to avoid memory leaks
 */
void free_stats(net_analysis_t *stats);

/*
 * Returns struct net_sample_t, so this can be added to the net buffer which holds the last samples 
 */
net_sample_t net_sample_get(void);

/*
 * Add sample to the net_buffer which is an array of samples
 * 
 * history: A struct that contains the history, represented by an array
 * the next index to be added is within history and the previously popped
 * sample is stored
 * sample: New sample to be placed at next_index (the field in history)
 */
void net_history_add(net_sample_history_t *history, const net_sample_t sample);

/*
 * Analyse recently popped and pushed sample onto history
 * 
 * stats: Reference to analysed stats that would be updated
 * history: History contains the items to be analysed, such as popped and pushed values
 */
void net_stat_analyse(net_analysis_t *stats, const net_sample_history_t *history);

/*
 * Takes in the analysis and returns enumerated type to indicate the network status, this will be used in a switch case block to chose what led to flash 
 */
net_status_t net_stat_update(const net_analysis_t *stats);

#endif