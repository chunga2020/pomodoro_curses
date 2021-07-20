#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dbg.h"
#include "pomodoro.h"

const char *prog_name = "pomodoro_curses";

typedef enum {
    POMODORO_WORK,
    POMODORO_SHORT_REST,
    POMODORO_LONG_REST,
    POMODORO_ERROR = -1
} STATE;

void usage(char *argv[]) {
    fprintf(stderr,
            "%s: A simple ncurses-based Pomodoro timer\n"
            "\n"
            "Usage: %s [-h] [-b short_break_len] [-n sets] "
            "[-p sessions_per_set] [-s session_length] [-B long_break_len]\n"
            "\n"
            "Note: all durations are in minutes\n"
            "\n"
            "Options:\n"
            "    -h\tShow this help message and exit\n"
            "\n"
            "    -b\tLength of breaks between work sessions (default 5)\n"
            "    -n\tNumber of sets to work through (default 1)\n"
            "    -p\tNumber of pomodoros (work sessions) per set (default 3)\n"
            "    -s\tPomodoro session length (default 25)\n"
            "    -B\tLong break length (default 30)\n",
            prog_name, prog_name

    );
}

char *pomodoro_status(STATE state) {
    switch (state) {
        case POMODORO_WORK:
            return "Working, working, working...";
        case POMODORO_SHORT_REST:
            return "Taking a little break :)";
        case POMODORO_LONG_REST:
            return "Relaxing for a while :D";
        case POMODORO_ERROR:
            return "Something went wrong :(";
        default:
            sentinel("Invalid STATE value");
    }
error:
    return NULL;
}

/* 
 * Do a pomodoro session
 *
 * Parameters:
 *     t: pointer to the Timer to use
 *     session_length: length of this pomodoro, in minutes
 *     state: the type of timer --- working or resting
 *     row: ncurses window row (for printing)
 *     col: ncurses window column (for print)
 * 
 * Return: 0 on success, -1 on failure
 */
int do_timer_session(Timer *t, int session_length, STATE state, int row,
        int col) {
    check(t != NULL, "Got NULL Timer pointer.");
    int time_left = session_length * (SECONDS_PER_MINUTE);
    int hours = session_length / MINUTES_PER_HOUR;
    session_length -= hours * MINUTES_PER_HOUR;
    int minutes = session_length;

    int rc = Timer_set(t, hours, minutes, 0);
    check(rc == 0, "Failed to set main timer.");

    char msg[80];
    char *cur_state_msg = pomodoro_status(state);
    sprintf(msg, "%02d:%02d:00", hours, minutes);
    mvprintw(row / 2, (col-strlen(msg)) / 2, msg, hours, minutes);
    refresh();
    clear();
    refresh();
    mvprintw(row / 2 - 1, (col-strlen(cur_state_msg)) / 2, "%s", cur_state_msg);

    while ((time_left = Timer_tick(t)) != -1) {
        hours = time_left / (SECONDS_PER_MINUTE * MINUTES_PER_HOUR);
        time_left -= hours * SECONDS_PER_MINUTE * MINUTES_PER_HOUR;
        minutes = time_left / SECONDS_PER_MINUTE;
        time_left -= minutes * SECONDS_PER_MINUTE;
        int seconds = time_left;
        sprintf(msg, "Time left: %02d:%02d:%02d", hours, minutes, seconds);
        mvprintw(row / 2, (col-strlen(msg)) / 2, "%s", msg);
        refresh();
    }
    return 0;
error:
    return -1;
}

/* 
 * Do a full pomodoro set
 *
 * Parameters:
 *     t: The Timer to use
 *     work_len: how many minutes work sessions are
 *     short_b_len: how many minutes short (inter-pomodoro) breaks are
 *     long_b_len: how many minutes long (inter-set) breaks are
 *     sessions_per_set: how many pomodoro+short-break reps per set
 *     row: ncurses window row count (for printing)
 *     col: ncurses window column count (for printing)
 * 
 * Return: 0 on sucess, -1 on error
 */
int do_pomodoro_set(Timer *t, int work_len, int short_b_len, int long_b_len,
        int sessions_per_set, int row, int col) {
    check(t != NULL, "Got NULL Timer pointer");

    for (int i = 0; i < sessions_per_set; i++) {
        do_timer_session(t, work_len, POMODORO_WORK, row, col);
        clear();
        refresh();
        do_timer_session(t, short_b_len, POMODORO_SHORT_REST, row, col);
    }
    clear();
    refresh();
    do_timer_session(t, long_b_len, POMODORO_LONG_REST, row, col);
error:
    return -1;
}

int main(int argc, char *argv[]) {

    /* #### program options #### */
    int opt; // variable for getting options with getopt(3)

    // Default pomodoro (work session) length, in minutes
    short int session_length = 25;

    // Default short break length, in minutes
    short int short_break_length = 5;

    // Number of sets to work through
    short int num_sets = 1;

    // Number of sessions before a long break
    short int pomodoros_per_set = 3;

    // Default long break length, in minutes
    short int long_break_length = 30;

    while ((opt = getopt(argc, argv, "b:hn:p:s:B:")) != -1) {
        switch (opt) {
            case 'b':
                short_break_length = atoi(optarg);
                break;
            case 'h':
                usage(argv);
                exit(EXIT_SUCCESS);
            case 'n':
                num_sets = atoi(optarg);
                break;
            case 'p':
                pomodoros_per_set = atoi(optarg);
                break;
            case 's':
                session_length = atoi(optarg);
                break;
            case 'B':
                long_break_length = atoi(optarg);
                break;
            default:
                usage(argv);
                exit(EXIT_FAILURE);
        }
    }

    int row = 0;
    int col = 0;

    /* Has initscr been called? (for error-checking and cleanup purposes) */
    int in_curses_mode = 0;

    initscr(); // start curses mode
    in_curses_mode = 1;
    getmaxyx(stdscr, row, col); // get window dimensions
    cbreak(); // no line-buffered input, but allow signals
    keypad(stdscr, TRUE);
    noecho(); // don't echo on getch()
    curs_set(0); // invisible cursor

    char *welcome_msg = "Welcome to pomodoro_curses";
    mvprintw(row / 2, (col-strlen(welcome_msg)) / 2, "%s", welcome_msg);
    refresh();
    sleep(2);
    clear();
    refresh();

    Timer *pomodoro_timer = Timer_alloc();
    check(pomodoro_timer != NULL, "Failed to allocate main pomodoro timer.");

    do_pomodoro_set(pomodoro_timer, session_length, short_break_length,
            long_break_length, pomodoros_per_set, row, col);

    getch();

    Timer_destroy(pomodoro_timer);
    pomodoro_timer = NULL;
    endwin();
    in_curses_mode = 0;

    return 0;
error:
    if (pomodoro_timer != NULL) {
        Timer_destroy(pomodoro_timer);
        pomodoro_timer = NULL;
    }
    if (in_curses_mode) {
        endwin();
    }
    return -1;
}
