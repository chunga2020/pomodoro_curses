#ifndef POMODORO_H
#define POMODORO_H

/* Timer tick length, in seconds */
#define TIMER_PULSE 1

#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60

typedef struct {
    int seconds;
} Timer;

Timer *Timer_alloc();
int Timer_set(Timer *t, int hours, int minutes, int seconds);
void Timer_destroy(Timer *t);

int Timer_tick(Timer *t);

#endif
