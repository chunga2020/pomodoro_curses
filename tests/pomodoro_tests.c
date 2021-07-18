#include "minunit.h"
#include "pomodoro.h"

char *test_Timer_alloc() {
    Timer *t = Timer_alloc();
    mu_assert(t != NULL, "Timer_alloc failed.");
    Timer_destroy(t);
    return NULL;
}

char *all_tests() {
    mu_suite_start();

    mu_run_test(test_Timer_alloc);

    return NULL;
}

RUN_TESTS(all_tests);
