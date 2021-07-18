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
