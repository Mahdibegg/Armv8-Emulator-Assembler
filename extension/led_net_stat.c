#include "network_status.h"

#define DEVICE_ON 1

int main(void) {
    /* 
     * Initialising before the loop starts
     * 
     * leds: LEDs controller, which will be updated with a new net_stat
     * history: A history data structure which contains an array of samples
     * stats: Represents the averages of the last N samples
     */
    led_controller_t *leds = init_led();
    net_sample_history_t *history = init_history();
    net_analysis_t *stats = init_net_analysis();

    /* One single check for all NULL pointers */
    if (leds == NULL ||
        history == NULL ||
        stats == NULL) {
        fprintf("ERROR: Initialisation failed for program start");
        abort()
    }

    /* Program loop when device starts up */
    while (DEVICE_ON) {
        /*
         * Get sample, push to the history, analyse it
         * Then provide those statistics for a status update
         * And set the LED when status is updated
         */
        net_sample_t sample = net_sample_get();

        net_history_add(history, sample);
        net_stat_analyse(stats, history);

        net_status_t net_stat = net_stat_update(stats);

        net_set_led(leds, net_stat);

        sleep(PROGRAM_WAIT);
    }

    free_controller(leds);
    free_stats(stats)

    return 0;
}