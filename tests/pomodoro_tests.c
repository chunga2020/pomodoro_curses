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

char *all_tests() {
    mu_suite_start();

    mu_run_test(test_Timer_alloc);
    mu_run_test(test_Timer_set_null_ptr);

    return NULL;
}

RUN_TESTS(all_tests);
