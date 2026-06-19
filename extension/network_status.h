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

#endif