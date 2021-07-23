#include <getopt.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dbg.h"
#include "pomodoro.h"

const char *PROG_NAME = "pomodoro_curses";

typedef enum {
    POMODORO_WORK,
    POMODORO_SHORT_REST,
    POMODORO_LONG_REST,
    POMODORO_ERROR = -1
} STATE;

/* 
 * Print a usage message to stderr and exit.
 */
void usage() {
    fprintf(stderr,
            "%s: A simple ncurses-based Pomodoro timer\n"
            "\n"
            "Usage: %s [-h] [OPTIONS]\n"
            "\n"
            "Mandatory arguments to long options are mandatory for short "
            "options too.\n"
            "Note: all durations are in minutes\n"
            "\n"
            "Options:\n"
            "    -h, --help\t\t\tShow this help message and exit\n"
            "\n"
            "    -b, --short-break-length"
                    "\tLength of breaks between work sessions (default 5)\n"
            "    -n, --num-sets\t\tNumber of sets to work through (default 1)\n"
            "    -p, --pomodoros-per-set"
                    "\tNumber of pomodoros (work sessions) per set (default 3)\n"
            "    -s, --session-length\tPomodoro session length (default 25)\n"
            "    -B, --long-break-length\tLong break length (default 30)\n",
            PROG_NAME, PROG_NAME

    );
}

/* 
 * Return a friendly status message to show to the user
 *
 * Parameters:
 *     state: The current state --- working or resting
 * 
 * Return:
 *     If state is valid, a human-friendly string describing the current state.
 *     Else, NULL
 */
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
 *     status_win: pointer to the status window; needed for window calcuations
 *     timer_win: pointer to the timer window; needed for window calcuations
 *     set_num: current set number; needed for status window
 * 
 * Return: 0 on success, -1 on failure
 */
int do_timer_session(Timer *t, int session_length, STATE state,
        WINDOW *status_win, WINDOW *timer_win, int set_num) {
    check(t != NULL, "Got NULL Timer pointer.");
    int time_left = session_length * (SECONDS_PER_MINUTE);
    int hours = session_length / MINUTES_PER_HOUR;
    session_length -= hours * MINUTES_PER_HOUR;
    int minutes = session_length;

    int rc = Timer_set(t, hours, minutes, 0);
    check(rc == 0, "Failed to set main timer.");

    char msg[80];
    char *cur_state_msg = pomodoro_status(state);

    int timer_win_h;
    int timer_win_w;
    getmaxyx(timer_win, timer_win_h, timer_win_w);
    int timer_win_starty = getbegy(timer_win);

    int status_win_h;
    int status_win_w;
    getmaxyx(status_win, status_win_h, status_win_w);
    int status_win_starty = getbegy(status_win);

    wclear(timer_win);
    box(timer_win, 0, 0);
    wrefresh(timer_win);

    wclear(status_win);
    box(status_win, 0, 0);
    wrefresh(status_win);

    sprintf(msg, "%02d:%02d:00", hours, minutes);
    mvwprintw(timer_win, (timer_win_starty + timer_win_h - 1) / 2 - 1,
            (timer_win_w-strlen(msg)) / 2 - 1, msg);
    box(timer_win, 0, 0);
    wrefresh(timer_win);
    mvwprintw(status_win, (status_win_starty + status_win_h) / 2,
            (status_win_w-strlen(cur_state_msg)) / 2, "%s", cur_state_msg);
    box(status_win, 0, 0);
    wrefresh(status_win);
    sprintf(msg, "Current set: %d", set_num);
    mvwprintw(status_win, (status_win_starty + status_win_h) / 2 - 1,
            (status_win_w-strlen(msg)) / 2, "%s", msg);
    box(status_win, 0, 0);
    wrefresh(status_win);

    while ((time_left = Timer_tick(t)) != -1) {
        hours = time_left / (SECONDS_PER_MINUTE * MINUTES_PER_HOUR);
        time_left -= hours * SECONDS_PER_MINUTE * MINUTES_PER_HOUR;
        minutes = time_left / SECONDS_PER_MINUTE;
        time_left -= minutes * SECONDS_PER_MINUTE;
        int seconds = time_left;
        sprintf(msg, "Time left: %02d:%02d:%02d", hours, minutes, seconds);
        mvwprintw(timer_win, (timer_win_starty + timer_win_h - 1) / 2 - 1,
                (timer_win_w-strlen(msg)) / 2, "%s", msg);
        box(timer_win, 0, 0);
        wrefresh(timer_win);
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
 *     status_win: pointer to the status window; needed for window calcuations
 *     timer_win: pointer to the timer window; needed for window calcuations
 *     set_num: current set number; needed for status window
 * 
 * Return: 0 on sucess, -1 on error
 */
int do_pomodoro_set(Timer *t, int work_len, int short_b_len, int long_b_len,
        int sessions_per_set, WINDOW *status_win, WINDOW *timer_win,
        int set_num) {
    check(t != NULL, "Got NULL Timer pointer");

    for (int i = 0; i < sessions_per_set; i++) {
        do_timer_session(t, work_len, POMODORO_WORK, status_win, timer_win,
                set_num);
        clear();
        refresh();
        do_timer_session(t, short_b_len, POMODORO_SHORT_REST,
                status_win, timer_win, set_num);
    }
    clear();
    refresh();
    do_timer_session(t, long_b_len, POMODORO_LONG_REST, status_win, timer_win,
            set_num);
error:
    return -1;
}

/*
 * Destroy an ncurses window that isn't stdscr
 * 
 * Parameters:
 *     win: window to destroy
 * Returns: none
 */
void destroy_win(WINDOW *win) {
    check(win != NULL, "Got NULL Window pointer");
    wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    delwin(win);
error:
    return;
}

/* 
 * Create a new window
 *
 * Parameters:
 *     h: window height
 *     w: window width
 *     y: y coordinate of top-left corner
 *     x: x coordinate of top-left corner
 * Returns:
 *     on success, a pointer to the new window
 *     on failure, NULL
 */
WINDOW *create_window(int h, int w, int y, int x) {
    WINDOW *new_window = newwin(h, w, y, x);
    check(new_window != NULL, "Failed to create window");
    box(new_window, 0, 0);
    wrefresh(new_window);

    return new_window;
error:
    return NULL;
}

int main(int argc, char *argv[]) {

    WINDOW *status_window = NULL;
    WINDOW *timer_window = NULL;
    /* #### program options #### */
    int opt; // variable for getting options with getopt(3)
    int option_index;

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

    static struct option long_options[] = {
        {"short-break-length", required_argument, 0, 'b'},
        {"long-break-length", required_argument, 0, 'B'},
        {"help", no_argument, 0, 'h'},
        {"num-sets", required_argument, 0, 'n'},
        {"pomodoros-per-set", required_argument, 0, 'p'},
        {"session-length", required_argument, 0, 's'}
    };

    while ((opt = getopt_long(argc, argv, "b:hn:p:s:B:", long_options,
            &option_index)) != -1) {
        switch (opt) {
            case 'b':
                short_break_length = atoi(optarg);
                check(short_break_length > 0,
                        "Short break length must be greater than 0");
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'n':
                num_sets = atoi(optarg);
                check(num_sets > 0, "Number of sets must be greater than 0");
                break;
            case 'p':
                pomodoros_per_set = atoi(optarg);
                check(pomodoros_per_set > 0,
                        "Pomodoros per set must be greater than 0");
                break;
            case 's':
                session_length = atoi(optarg);
                check(session_length > 0,
                        "Session length must be greater than 0");
                break;
            case 'B':
                long_break_length = atoi(optarg);
                check(long_break_length > 0,
                        "Long break length must be greater than 0");
                break;
            default:
                usage();
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

    /* #### Window setup #### */
    /* Status window starts at top-left corner */
    int status_window_starty = 0;
    int status_window_startx = 0;

    /* Status window stretches all the way from left to right */
    int status_window_width = col;
    /*
     * Status window is minimal size for centered messages:
     *     2 rows for borders
     *     2 rows for padding
     *     2 rows for messages:
     *         1 row for number of current set
     *         1 row for status message from pomodoro_status()
     */
    int status_window_height = 6;

    status_window = create_window(status_window_height,
            status_window_width, status_window_starty, status_window_startx);
    check(status_window != NULL, "Failed to create status window");

    /* Timer window also extends all the way left to right */
    int timer_window_startx = 0;
    /* Just stick the timer right under the status window */
    int timer_window_starty = status_window_starty + status_window_height;

    int timer_window_width = col;
    /* Timer window takes up whatever rows are left from the status window */
    int timer_window_height = row - status_window_height;

    timer_window = create_window(timer_window_height,
            timer_window_width, timer_window_starty, timer_window_startx);
    check(timer_window != NULL, "Failed to create timer window");


    char *welcome_msg = "Welcome to pomodoro_curses";
    mvwprintw(status_window,
            ((status_window_startx + status_window_height) / 2) - 1,
            (status_window_width-strlen(welcome_msg)) / 2, "%s", welcome_msg);
    wrefresh(status_window);
    sleep(2);
    wclear(status_window);
    wrefresh(status_window);

    Timer *pomodoro_timer = Timer_alloc();
    check(pomodoro_timer != NULL, "Failed to allocate main pomodoro timer.");

    for (int i = 1; i <= num_sets; i++) {
        do_pomodoro_set(pomodoro_timer, session_length, short_break_length,
                long_break_length, pomodoros_per_set, status_window,
                timer_window, i);
    }

    getch();

    Timer_destroy(pomodoro_timer);
    pomodoro_timer = NULL;
    destroy_win(status_window);
    destroy_win(timer_window);
    endwin();
    in_curses_mode = 0;

    return 0;
error:
    if (pomodoro_timer != NULL) {
        Timer_destroy(pomodoro_timer);
        pomodoro_timer = NULL;
    }
    if (status_window != NULL) {
        destroy_win(status_window);
    }
    if (timer_window != NULL) {
        destroy_win(timer_window);
    }
    if (in_curses_mode) {
        endwin();
    }
    return -1;
}
