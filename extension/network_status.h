#ifndef NETWORK_H   
#define NETWORK_H

#include <stdbool.h>
#include <gpiod.h>

/* 
 * Represents full GPIO chip for access
 * Stores all references to GPIO pins using gpio module
 */
typedef struct {
    struct gpiod_chip *chip; /* Open GPIO device */

    struct gpiod_line *red; /* Reserved for DOS detection */
    struct gpiod_line *yellow; /* Reserved for UNSTABLE  */
    struct gpiod_line *green; /* Reserved for STABLE */
} led_controller_t;

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
led_controller_t *init_led(void) {}

#endif