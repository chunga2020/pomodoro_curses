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
    
    t->hours = hours;
    check(t->hours == hours, "Hours not set correctly. Expected %d, got %d",
            hours, t->hours);

    t->minutes = minutes;
    check(t->minutes == minutes,
            "Minutes not set correctly. Expected %d, got %d", minutes,
            t->minutes);

    t->seconds = seconds;
    check(t->seconds == seconds,
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
