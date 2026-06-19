#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "network_status.h"

#define PING_LIMIT 100.0
#define MAX_LINE_LENGTH 256
#define SAMPLE_HISTORY_SIZE 10

#define GPIO_CHIP "/dev/gpiochip0"

#define RED_PIN 17
#define GREEN_PIN 27
#define BLUE_PIN 22

#define FLICKER_DELAY 0.1
#define DNS "8.8.8.8"

#define ERROR_WAIT 3

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
} net_stat;

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
    if (leds == NULL) {
        return;
    }

    if (leds->red != NULL) {
        gpiod_line_set_value(leds->red, 0);
    }

    if (leds->blue != NULL) {
        gpiod_line_set_value(leds->blue, 0);
    }

    if (leds->green != NULL) {
        gpiod_line_set_value(leds->green, 0);
    }
}

/* 
 * Hold an LED light
 * 
 * colour: The colour of LED that is being switched on
 */
static void static_led_colour(struct gpiod_line *colour) {
    /*
     * NULL reference check to the colour argument
     * Nothing returned just to keep the service running
     * It just means no LED will be displayed
     */
    if (colour == NULL) {
        return;
    }

    gpiod_line_set_value(colour, 1);
}

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
        leds->blue == NULL ||
        leds->green == NULL) {
        return false;
    }

    return true;
}

/*
 * Helper functions for parsing sample data

 * For getting ping and packet loss data
 * 
 * Open the file using popen() to treat terminal output as text
 * Ping Google DNS with a single packet count (safe option)
 * 
 * For getting rx, tx bytes
 * 
 * Accessing the stat files and returning rx and tx bytes respectively
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
 * Ignore text until "% packet loss" and return packetloss
 * No connection returns 100.0 (Full packet loss)
 */
static double get_packet_loss(void) {
    char command[MAX_LINE_LENGTH];

    snprintf(command, sizeof(command), "ping -c 1 %s", DNS);

    FILE *net_stats_file = popen(command, "r");

    char net_info[MAX_LINE_LENGTH];

    double packet_loss = 100.0;

    if (net_stats_file == NULL) {
        return packet_loss;
    }

    while (fgets(net_info, sizeof(net_info), net_stats_file) != NULL) {
        char *packet_ptr = strstr(net_info, "% packet loss");
        int transmitted;
        int received;
        double loss;

        if (sscanf(net_info, "%d packets transmitted, %d received, %lf%% packet loss", &transmitted, &received, &loss ) == 3) {
            packet_loss = loss;
        }

        break;
    }

    pclose(net_stats_file);
    return packet_loss;

}

/*
 * Read unsigned long value from file
 *
 * read network statistic files:
 * /sys/class/net/wlan0/statistics/rx_bytes
 * /sys/class/net/wlan0/statistics/tx_bytes
 *
 * These files contain one unsigned long value.
 *
 * file_path: Path to the file that stores the unsigned long value
 *
 * Returns the unsigned long value read from the file.
 * Returns 0 if the file cannot be opened or read.
 */
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

    snprintf(command, sizeof(command), "ping -c 1 %s", DNS);

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
        pclose(fp);
        return NET_IFACE_NONE;
    }

    /*
     *  Close file because read has happended
     */
    pclsoe(interface_info_file);

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
 * Get received bytes for active interface
 *
 * RX means received data.
 * Reads the total number of bytes received by the selected network interface.
 *
 * iface: Enum value representing the network interface being used
 *
 * Returns RX bytes for wlan0 or eth0.
 * Returns 0 if the interface is unsupported or not available.
 */
static unsigned long get_rx_bytes(net_interface_t iface) {
    switch (iface) {
        case NET_IFACE_WLAN0:
            return read_u_long_from_file(
                "/sys/class/net/wlan0/statistics/rx_bytes"
            );
        
        case NET_IFACE_ETH0:
            return read_u_long_from_file(
                "/sys/class/net/eth0/statistics/rx_bytes"
            );
        default:
            return 0;
    }
}

/*
 * Get transmitted bytes for active interface
 *
 * TX means transmitted data.
 * Reads the total number of bytes transmitted by the selected network interface.
 *
 * iface: Enum value representing the network interface being used
 *
 * Returns TX bytes for wlan0 or eth0.
 * Returns 0 if the interface is unsupported or not available.
 */
static unsigned long get_tx_bytes(net_interface_t iface) {
    switch (iface) {
        case NET_IFACE_WLAN0:
            return read_u_long_from_file(
                "/sys/class/net/wlan0/statistics/tx_bytes"
            );
        case NET_IFACE_ETH0:
            return read_u_long_from_file(
                "/sys/class/net/eth0/statistics/tx_bytes"
            );
        default:
            return 0;
    }
}

/*
 * Implementation section
 */

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
            static_led_colour(leds->red);
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
 * Initialise history struct 
 * 
 * Allocate memory to the struct
 * Check for null pointer and handle appropriately
 * 
 * Initialise fields
 */
net_sample_history_t *init_history(void) {

    net_sample_history_t *history = malloc(sizeof(net_sample_history_t));

    if (history == NULL) {
        return NULL;
    }

    history->next_index = 0;
    history->popped = NULL;
}

/*
 * Free history data structure
 * 
 * Takes pointer to history struct as a parameter
 * 
 * Only needs to free the struct, does not need to free any of the fields
 */
void free_history(net_sample_history_t *history) {
    if (history == NULL) {
        return 
    }

    free(history);
}

/*
 * Initialise analysis struct 
 * 
 * Allocate memory to the sturct 
 * Check for null pointer and hanlde appropriately
 * 
 * Initialise fields
 */
net_analysis_t *init_net_analysis(void) {

    net_analysis_t *analysis = malloc(sizeof(net_analysis_t));

    if (analysis == NULL) {
        return NULL;
    }

    analysis->avg_ping_ms = 0.0;
    analysis->avg_packet_loss = 0.0;
    analysis->avg_rx_rate = 0.0;
    analysis->avg_tx_rate = 0.0;
}

net_sample_t net_sample_get(net_interface_t iface) {
    /* Make the struct and initialise the fields with helper functions */
    net_sample_t sample = {
        .ping_ms = get_ping();
        .packet_loss = get_packet_loss();
        .rx_bytes = get_rx_bytes(iface);
        .tx_bytes = get_tx_bytes(iface);
    }

    return sample;
}

void net_history_add(net_sample_history_t *history, const net_sample_t sample) {
    /* No buffer to add the new sample to, so quit */
    if (history == NULL || history->array == NULL) {
        return;
    }

    /*
     * No need to assert size to end program
     * Next_index is the only indexing variable to modify the buffer
     * And it is being checked against the max capacity 
     */

    /* Comparing size against max buffer size which is sample history size*/
    size_t history_size = sizeof(history->array) / sizeof(sample);

    /*
     * Next_index is a circular pointer on the array
     * When it reaches the end, reset the index to the beginning
     * Continue to increment it 
     */
    if (*next_index >= SAMPLE_HISTORY_SIZE - 1) {
        /* 
         * Update history values
         * next_index reset to front of array
         * popped is updated to value to be removed
         * array at new index contains new sample
         */
        history->next_index = 0;
        history->popped = history->array[history->next_index];
    } else {
        history->popped = NULL;
    }
    history->array[history->next_index] = sample;
    history->next_index++;
}

void net_stat_analyse(net_analysis_t *stats, const net_sample_history_t *history) {
    if (stats == NULL || history == NULL) {
        return;
    }

    net_sample_t popped = history->popped;

    if (popped == NULL) {
        popped = {
            .ping_ms = 0;
            .packet_loss = 0;
            .rx_bytes = 0;
            .tx_bytes = 0;
        }
    }
    net_sample_t new = history->array[history->next_index - 1];

    /* Recalculating the mean by updating a fraction of the mean from the stats reference */
    double ping_update = (new.ping_ms - popped.ping_ms)/SAMPLE_HISTORY_SIZE;
    stats->avg_ping_ms = stats->avg_ping_ms + ping_update;
}

net_status_t net_stat_update(net_analysis_t *stats) {}