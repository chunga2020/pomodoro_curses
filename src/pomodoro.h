#ifndef POMODORO_H
#define POMODORO_H

/* Timer tick length, in seconds */
#define TIMER_PULSE 1

typedef struct {
    int hours;
    int minutes;
    int seconds;
} Timer;

Timer *Timer_alloc();
int Timer_set(Timer *t, int hours, int minutes, int seconds);
void Timer_destroy(Timer *t);

long int Timer_tick(Timer *t);

#endif
