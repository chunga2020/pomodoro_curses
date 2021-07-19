#include "dbg.h"
#include "minunit.h"
#include "pomodoro.h"

char *test_Timer_alloc() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");
    Timer_destroy(t);
    return NULL;
}

char *test_Timer_set_null_ptr() {
    Timer *t = NULL;
    int rc = Timer_set(t, 0, 0, 0);
    mu_assert(rc == -1,
            "With a NULL Timer pointer, expected rc -1, got rc %d",
            rc);

    return NULL;
}

char *test_Timer_set_negative_hours() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");

    int hours = -2;
    int rc = Timer_set(t, hours, 0, 0);
    mu_assert(rc == -1,
            "With hours set to %d, expected retcode -1, got retcode %d",
            hours, rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_set_valid_hours() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "timer_alloc failed.");

    int hours = 2;
    int rc = Timer_set(t, hours, 0, 0);
    mu_assert(rc == 0,
            "With hours set to %d, expected rc 0, got rc %d",
            hours, rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_set_negative_minutes() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");

    int minutes = -2;
    int rc = Timer_set(t, 0, minutes, 0);
    mu_assert(rc == -1,
            "With minutes set to %d, expected rc -1, got rc %d",
            minutes, rc);
    
    Timer_destroy(t);
    return NULL;
}

char *test_Timer_set_valid_minutes() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed");

    int minutes = 10;
    int rc = Timer_set(t, 0, minutes, 0);
    mu_assert(rc == 0,
            "With minutes set to %d, expected rc 0, got rc %d",
            minutes, rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_set_minutes_too_large() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");

    int minutes = 60;
    int rc = Timer_set(t, 0, minutes, 0);
    mu_assert(rc == -1,
            "With minutes set to %d, expected rc -1, got rc %d",
            minutes, rc);
    
    Timer_destroy(t);
    return NULL;
}

char *test_Timer_set_negative_seconds() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");

    int seconds = -2;
    int rc = Timer_set(t, 0, 0, seconds);
    mu_assert(rc == -1,
            "With seconds set to %d, expected rc -1, got rc %d",
            seconds, rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_set_valid_seconds() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");

    int seconds = 10;
    int rc = Timer_set(t, 0, 0, seconds);
    mu_assert(rc == 0,
            "With seconds set to %d, expected rc 0, got rc %d",
            seconds, rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_set_seconds_too_large() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");

    int seconds = 60;
    int rc = Timer_set(t, 0, 0, seconds);
    mu_assert(rc == -1,
            "With seconds set to %d, expected rc -1, got rc %d",
            seconds, rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_tick_null_ptr() {
    Timer *t = NULL;

    long int rc = Timer_tick(t);
    mu_assert(rc == -1, "With a NULL Timer ptr, expected rc -1, got %ld",
            rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_tick_negative_seconds() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");
    long int rc = Timer_set(t, 0, 0, 0);
    t->seconds = -1;

    rc = Timer_tick(t);
    mu_assert(rc == -1, "With a negative second value, expected rc -1, got %ld",
            rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_tick_decrements() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");
    int seconds = 5;
    long int rc = Timer_set(t, 0, 0, seconds);
    mu_assert(t->seconds == 5, "Timer_set failed.");
    rc = Timer_tick(t);
    mu_assert(rc == 4, "With initial value 00:00:05, expected 4 s, got %ld",
            rc);

    Timer_destroy(t);
    return NULL;
}

char *test_Timer_tick_stops_at_zero() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");
    int seconds = 5;
    long int rc = Timer_set(t, 0, 0, seconds);
    mu_assert(t->seconds == seconds, "Timer_set failed");
    while ((rc = Timer_tick(t))) {
        log_info("Seconds remaining: %ld", rc);
    }

    Timer_destroy(t);
    return NULL;
}

char *all_tests() {
    mu_suite_start();

    mu_run_test(test_Timer_alloc);
    mu_run_test(test_Timer_set_null_ptr);
    mu_run_test(test_Timer_set_negative_hours);
    mu_run_test(test_Timer_set_negative_minutes);
    mu_run_test(test_Timer_set_negative_seconds);
    mu_run_test(test_Timer_set_valid_hours);
    mu_run_test(test_Timer_set_valid_minutes);
    mu_run_test(test_Timer_set_valid_seconds);
    mu_run_test(test_Timer_set_minutes_too_large);
    mu_run_test(test_Timer_set_seconds_too_large);

    mu_run_test(test_Timer_tick_null_ptr);
    mu_run_test(test_Timer_tick_negative_seconds);
    mu_run_test(test_Timer_tick_decrements);
    mu_run_test(test_Timer_tick_stops_at_zero);

    return NULL;
}

RUN_TESTS(all_tests);
