#ifndef POMODORO_H
#define POMODORO_H

/* Timer tick length, in seconds */
#define TIMER_PULSE 1

#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60

typedef struct {
    int seconds;
} Timer;

/*
 * Allocates memory for a Timer object.
 * 
 * Parameters: none
 * 
 * Returns:
 *     on success, a pointer to the new Timer
 *     on failure, NULL
 */
Timer *Timer_alloc();

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
 *     on success, 0
 *     on failure, -1
 */
int Timer_set(Timer *t, int hours, int minutes, int seconds);

/* 
 * Destroy a Timer object
 *
 * Parameters:
 *     t: the Timer to destroy
 * Returns: none
 */
void Timer_destroy(Timer *t);

/*
 * Make a timer count down
 *
 * Parameters:
 *     t: The timer to count down
 * 
 * Returns:
 *     on success, the number of seconds remaining
 *     on failure, -1
 */
int Timer_tick(Timer *t);

#endif
