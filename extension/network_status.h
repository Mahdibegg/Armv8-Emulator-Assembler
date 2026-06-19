#ifndef NETWORK_H   
#define NETWORK_H

#include <stdbool.h>

typedef struct {
    double ping_ms;
    double packet_loss;
    unsigned long rx_bytes;
    unsigned long tx_bytes;
    int active_connections;
} net_sample_t;

/*
 * Network_status this function is run in the main function
 * It will prioritise a dos check before giving the status check
 * In a set time interval (since security is prioritised)
 * 
 * No return value, only LED output
 */
void network_status(void);

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

/*
 * This function choses which led to set, using the result of net_stat_update
 */
void net_set_led(net_status_t status);

#endif