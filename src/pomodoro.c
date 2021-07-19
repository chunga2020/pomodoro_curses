// For malloc suite
#include <stdlib.h>
// For sleep(3)
#include <unistd.h>

#include "dbg.h"
#include "pomodoro.h"

/*
 * Allocates memory for a Timer object.
 * 
 * Parameters: none
 * 
 * Returns: a pointer to the new timer on success; NULL on failure
 */
Timer *Timer_alloc() {
    Timer *t = (Timer *)malloc(sizeof(Timer));

    check(t != NULL, "Timer allocation failed.");

    return t;
error:
    return NULL;
}

/*
 * Sets a Timer to the specified time.
 *
 * Parameters:
 *     hours: the number of hours to put on the timer; must not be negative
 *     minutes: the number of minutes to put on the timer; must be in the 
 *              interval [0, 59]
 *     seconds: the number of seconds to put on the timer; must be in the
 *              interval [0, 59]
 * Returns:
 *     0 on success
 *     -1 on failure
 */
int Timer_set(Timer *t, int hours, int minutes, int seconds) {
    check(t != NULL, "Got NULL Timer pointer.");
    check(hours >= 0, "Cannot have negative 'hours' value '%d'.", hours);
    check(0 <= minutes && minutes <= 59,
            "Got bad 'minutes' value '%d'. Must be [0, 59]", minutes);
    check(0 <= seconds && seconds <= 59,
            "Got bad 'seconds' value '%d'. Must be [0, 59]", seconds);
    
    int total_seconds = (SECONDS_PER_MINUTE * MINUTES_PER_HOUR * hours)
            + (SECONDS_PER_MINUTE * minutes)
            + seconds;
    t->seconds = total_seconds;
    
    check(t->seconds == total_seconds,
            "Seconds not set correctly. Expected %d, got %d", seconds,
            t->seconds);

    return 0;
error:
    return -1;
}

/* 
 * Destroy a Timer object
 *
 * Parameters:
 *     t: the Timer to destroy
 * Returns: none
 */
void Timer_destroy(Timer *t) {
    check(t != NULL, "Got NULL Timer pointer.");
    free(t);
error:
    return;
}

/*
 * Make a timer count down
 *
 * Parameters:
 *     t: The timer to count down
 * 
 * Returns: on success, the number of seconds remaining; on failure, -1
 */
int Timer_tick(Timer *t) {
    check(t != NULL, "Got NULL Timer pointer");
    int time_remaining = t->seconds;
    check(time_remaining > 0,
            "Time remaining cannot be negative. H: %d, M: %02d, S: %02d",
            t->seconds / (SECONDS_PER_MINUTE * MINUTES_PER_HOUR),
            t->seconds / SECONDS_PER_MINUTE,
            t->seconds);
    sleep(TIMER_PULSE);
    t->seconds--;
    time_remaining = t->seconds;
    
    return time_remaining;
error:
    return -1;
}
