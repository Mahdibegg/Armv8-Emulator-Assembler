#ifndef NETWORK_H   
#define NETWORK_H

#include <stdbool.h>
#include <gpiod.h>

#define PROGRAM_WAIT 1

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
free_controller(led_controller_t *leds);

/*
 * Returns struct net_sample_t, so this can be added to the net buffer which holds the last samples 
 */
net_sample_t net_sample_get(void);

/*
 * Add sample to the net_buffer which is an array of samples
 */
void net_buffer_add(net_sample_t net_buffer[], net_sample_t sample);

/*
 * Analyses last 10 samples and builds struct that holds the average ping, packet loss and other information
 */
net_analysis_t net_stat_analyse(const net_sample_t buffer[]);

/*
 * Takes in the analysis and returns enumerated type to indicate the network status, this will be used in a switch case block to chose what led to flash 
 */
net_status_t net_stat_update(net_analysis_t analysis);

#endif