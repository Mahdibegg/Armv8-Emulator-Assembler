#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "network_status.h"

#define MAX_LINE_LENGTH 256

#define GPIO_CHIP "/dev/gpiochip0"

#define RED_PIN 17
#define GREEN_PIN 27
#define BLUE_PIN 22

#define DNS "8.8.8.8"

#define ERROR_WAIT 3

#define PING_STABLE_MAX 80.0

/* 
 * LED section - initialisation, reference freeing, led setter
 */

/* 
 * Turn off all LEDs
 * 
 * leds: Reference to controller to clear all LEDs
 */
static void clear_leds(led_controller_t *leds) {
    /*
     * All NULL checks otherwise program error would occur, ending the service
     * Since possibly one colour would be cleared
     * If the leds controller reference is gone, do not abort the program, a return is enough
     */
    if (leds == NULL || leds->request == NULL) {
        return;
    }

    gpiod_line_request_set_value(
        leds->request, leds->red, GPIOD_LINE_VALUE_INACTIVE
    );
    gpiod_line_request_set_value(
        leds->request, leds->blue, GPIOD_LINE_VALUE_INACTIVE
    );
    gpiod_line_request_set_value(
        leds->request, leds->green, GPIOD_LINE_VALUE_INACTIVE
    );
}

/* 
 * Hold an LED light
 * 
 * colour: The colour of LED that is being switched on
 */
static void static_led_colour(led_controller_t *leds, unsigned int colour) {
    /*
     * NULL reference check to the colour argument
     * Nothing returned just to keep the service running
     * It just means no LED will be displayed
     */
    if (leds == NULL || leds->request == NULL) {
        return;
    }

    clear_leds(leds);
    gpiod_line_request_set_value(
        leds->request, colour, GPIOD_LINE_VALUE_ACTIVE
    );
}

/*
 * Get led lines for controller
 * Return true if successful, false if not
 *
 * leds: Reference to controller in order to get
 */
static bool get_led_lines(led_controller_t *leds) {
    /*
     * Build an array of the three GPIO pin offsets to configure them together
     * Order matches red, blue, green as stored in the controller
     */
    const unsigned int offsets[] = {
        leds->red,
        leds->blue,
        leds->green
    };

    /*
     * Allocate a settings object to define shared behaviour for all three lines
     * NULL means allocation failed so bail out early
     */
    struct gpiod_line_settings *settings = gpiod_line_settings_new();

    if (settings == NULL) {
        return false;
    }

    /*
     * Set all three lines as outputs, starting in the inactive (off) state
     */
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    /*
     * Allocate the line config and free settings on failure to avoid a leak
     */
    struct gpiod_line_config *line_config = gpiod_line_config_new();

    if (line_config == NULL) {
        gpiod_line_settings_free(settings);
        return false;
    }

    /*
     * Apply the shared settings to all three pin offsets at once
     * Free both config objects on failure before returning
     */
    if (gpiod_line_config_add_line_settings(
            line_config, offsets, 3, settings) < 0) {
        gpiod_line_config_free(line_config);
        gpiod_line_settings_free(settings);
        return false;
    }

    /*
     * Allocate a request config to label this consumer in the kernel
     * Free line config and settings on failure to avoid leaks
     */
    struct gpiod_request_config *request_config = gpiod_request_config_new();

    if (request_config == NULL) {
        gpiod_line_config_free(line_config);
        gpiod_line_settings_free(settings);
        return false;
    }

    gpiod_request_config_set_consumer(request_config, "network_led");

    /*
     * Submit the request to the chip, storing the handle in leds->request
     * This reserves all three lines for exclusive use by this program
     */
    leds->request = gpiod_chip_request_lines(
        leds->chip, request_config, line_config
    );

    /*
     * All config objects are no longer needed regardless of success or failure
     */
    gpiod_request_config_free(request_config);
    gpiod_line_config_free(line_config);
    gpiod_line_settings_free(settings);

    /* Non-null request means all three lines were successfully reserved */
    return leds->request != NULL;
}

/*
 * Helper functions for parsing sample data

 * For getting ping and packet loss data
 * 
 * Open the file using popen() to treat terminal output as text
 * Ping Google DNS with a single packet count (safe option)
 */

/*
 * Ignore text until "time=<ping_time>" and return ping_time
 * No connection returns -1.0
 */
static double get_ping(void) {
    char command[MAX_LINE_LENGTH];

    snprintf(command, sizeof(command), "ping -c 1 %s", DNS);

    /* Get the file pointer for the terminal output to read */
    FILE *ping_file  = popen(command, "r");

    /* Reading line buffer */
    char net_info[MAX_LINE_LENGTH];

    /* 
     * Result to be returned, a default value of -1.0 indicates no connection 
     * Since ping cannot be negative
     */
    double ping_time = -1.0;

    /* No open file means no connection so no updated ping_time hence return */
    if (ping_file == NULL) {
        return ping_time;
    }

    /* 
     * Use strstr to return pointer to the ping_time
     * Non-null pointer results in an available float for ping_time
     */
    while (fgets(net_info, sizeof(net_info), ping_file) != NULL) {
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
 * Get active network interface
 *
 * Checks which network interface the Raspberry Pi is currently using to reach the DNS address.
 *
 * Use the command:
 * ip route get 8.8.8.8
 *
 * Command normally contains "dev wlan0" if WiFi is being used or "dev eth0" if Ethernet is being used.
 *
 * Returns NET_IFACE_WLAN0 if WiFi is being used.
 * Returns NET_IFACE_ETH0 if Ethernet is being used.
 * Returns NET_IFACE_NONE if no supported interface is found.
 */
static net_interface_t get_interface(void) {
    char command[MAX_LINE_LENGTH];

    snprintf(command, sizeof(command), "ip route get %s", DNS);

    /*
     * Open file (terminal as txt file) with following command to get information in interface type
     */
    FILE *interface_info_file = popen(command, "r");

    /*
     * initialise fixed size buffer to store information from file
     */
    char buffer[MAX_LINE_LENGTH];

    /*
     * Null pointer check, if null that means connection could not be established as file is not opened so return No network interface    
     */
    if (interface_info_file == NULL) {
        perror("Error: File failed to open");
        return NET_IFACE_NONE;
    }

    /*
     * If fgets returns null then it was unsuccessful read so return no interface
     */
    if (fgets(buffer, sizeof(buffer), interface_info_file) == NULL) {
        pclose(interface_info_file);
        return NET_IFACE_NONE;
    }

    /*
     *  Close file because read has happended
     */
    pclose(interface_info_file);

    /*
     * If statement checks to see which interface is read 
     */
    if (strstr(buffer, "dev wlan0") != NULL) {
        return NET_IFACE_WLAN0;
    }

    if (strstr(buffer, "dev eth0") != NULL) {
        return NET_IFACE_ETH0;
    }

    /* If neither of the two interfaces was found, return none */
    return NET_IFACE_NONE;
}

/*
 * Implementation section
 */

void net_set_led(led_controller_t *leds, net_status_t net_stat) {
    /* 
     * No network connection: No LED light
     * Unstable network: Red LED
     * Stable network: Green LED
     */
    switch (net_stat) {
        case NET_DOWN:
            clear_leds(leds);
            break;
        case NET_UNSTABLE:
            static_led_colour(leds, leds->blue);
            break;
        case NET_STABLE:
            static_led_colour(leds, leds->green);
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
    led_controller_t *leds = malloc(sizeof(*leds));

    /* Instead of returning an error, continue attempts at allocating memory */
    if (leds == NULL) {
        return NULL;
    }

    /* If you need to free the memory for whatever reason, have initial NULL values for safety */
    leds->chip = NULL;
    leds->request = NULL;
    leds->red = RED_PIN;
    leds->green = GREEN_PIN;
    leds->blue = BLUE_PIN;

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
        sleep(ERROR_WAIT);
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
    if (leds->request != NULL) {
        gpiod_line_request_release(leds->request);
    }

    if (leds->chip != NULL) {
        gpiod_chip_close(leds->chip);
    }

    free(leds);
}

/*
 * Initialise history struct 
 * 
 * Allocate memory to the struct and initialise next_index to 0 and popped to NULL
 * Check for null pointer and handle appropriately
 */
net_sample_history_t *init_history(void) {

    net_sample_history_t *history = malloc(sizeof(net_sample_history_t));

    if (history == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < SAMPLE_HISTORY_SIZE; i++) {
        history->array[i] = (net_sample_t) {
            .ping_ms = 0.0,
            .iface = NET_IFACE_NONE
        };
    }

    history->next_index = 0;
    history->popped = (net_sample_t) {
        .ping_ms = 0.0,
        .iface = NET_IFACE_NONE
    };

    return history;
}

/*
 * Free history data structure
 * 
 * Takes pointer to history struct as a parameter
 * Only needs to free the struct, does not need to free any of the fields
 */
void free_history(net_sample_history_t *history) {
    if (history == NULL) {
        return;
    }

    free(history);
}

/*
 * Initialise analysis struct 
 * 
 * Allocate memory to the sturct 
 * Initialise all fields to 0
 */
net_analysis_t *init_net_analysis(void) {

    net_analysis_t *stats = malloc(sizeof(net_analysis_t));

    if (stats == NULL) {
        return NULL;
    }

    stats->avg_ping_ms = 0.0;
    stats->iface = NET_IFACE_NONE;

    return stats;
}

/*
 * Free the analysis struct 
 * 
 * Takes pointer to stats that needs to be freed as a parameter
 * Only needs to free the struct, does not need to free any of the fields
 */
void free_stats(net_analysis_t *stats) {

    if (stats == NULL) {
        return;
    }

    free(stats);
}

net_sample_t net_sample_get(void) {
    net_interface_t iface = get_interface();

    net_sample_t sample = {
        .ping_ms = -1.0,
        .iface = iface
    };

    if (iface != NET_IFACE_NONE) {
        sample.ping_ms = get_ping();
    }

    return sample;
}

void net_history_add(net_sample_history_t *history, const net_sample_t sample) {
    /* No buffer to add the new sample to, so quit */
    if (history == NULL) {
        return;
    }

    /*
     * No need to assert size to end program
     * Next_index is the only indexing variable to modify the buffer
     * And it is being checked against the max capacity 
     */

    /*
     * Next_index is a circular pointer on the array
     * When it reaches the end, reset the index to the beginning
     * Continue to increment it 
     */
    history->popped = history->array[history->next_index];
    history->array[history->next_index] = sample;
    history->next_index = (history->next_index + 1) % SAMPLE_HISTORY_SIZE;
}

void net_stat_analyse(net_analysis_t *stats, const net_sample_history_t *history) {
    if (stats == NULL || history == NULL) {
        return;
    }

    /*
     * This relies on net_stat_analyse being called straight after
     * net_history_add, same as it is in the main loop, since popped and
     * next_index are only valid for the sample that was just pushed
     */
    net_sample_t popped = history->popped;

    /* Obtain index of the sample that was pushed */
    size_t new_index = (history->next_index == 0)
        ? SAMPLE_HISTORY_SIZE - 1 : history->next_index - 1;
    net_sample_t new = history->array[new_index];

    /*
     * Update average by removing the popped sample fraction
     * And then adding new sample fractions
     * This preserves complexity as you dont have to loop over entire history
     */
    stats->avg_ping_ms +=
        (new.ping_ms - popped.ping_ms) / SAMPLE_HISTORY_SIZE;

    stats->iface = new.iface;
}

net_status_t net_stat_update(const net_analysis_t *stats) {
    if (stats == NULL) {
        return NET_DOWN;
    }

    /* Get rid of no iface connection as soon as possible */
    if (stats->iface == NET_IFACE_NONE) {
        return NET_DOWN;
    }

    if (stats->avg_ping_ms < 0.0) {
        return NET_DOWN;
    }

    if (stats->avg_ping_ms <= PING_STABLE_MAX) {
        return NET_STABLE;
    }

    return NET_UNSTABLE;
}
