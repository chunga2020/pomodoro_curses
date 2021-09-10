#include <getopt.h>
#include <ini.h>
#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dbg.h"
#include "pomodoro.h"

/* #### Useful constants #### */

/* Maximum filepath length, not including NUL terminator */
const int MAXPATH = 255;

const char *PROG_NAME = "pomodoro_curses";

/* #### Useful typedefs #### */

typedef enum {
    ALERT_UNSET = 0,
    ALERT_BEEP = 1,
    ALERT_FLASH = 2
} ALERT_TYPE;

/* Code for config parsing */
typedef struct {
    int short_break_length;
    int long_break_length;
    int set_count;
    int pomodoros_per_set;
    int work_length;
    ALERT_TYPE alert_type;
} configuration;

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
            "    -a, --alert-type TYPE"
                    "\tType of alert to use. Choose 'beep' or 'flash'\n"
            "    -b, --short-break-length N"
                    "\tLength of breaks between work sessions (default 5)\n"
            "    -c, --config-file CONFIG\tPath to config file to use\n"
            "    -d\t\t\t\tDump to stdout values from config file and exit. May\n"
            "\t\t\t\tbe combined with -c to dump a custom config\n"
            "    -n, --num-sets N\t\tNumber of sets to work through (default 1)\n"
            "    -p, --pomodoros-per-set N"
                    "\tNumber of pomodoros (work sessions) per set (default 3)\n"
            "    -s, --session-length N\tPomodoro session length (default 25)\n"
            "    -B, --long-break-length N\tLong break length (default 30)\n",
            PROG_NAME, PROG_NAME

    );
}

/* 
 * Alert the user to the end of a timer session
 *
 * Parameters:
 *     type: the type of alert to use
 * 
 * Returns:
 *     On success, 0
 *     On failure, -1. Also emits a message to stderr
 */
int alert_user(ALERT_TYPE type) {
    check(type == ALERT_BEEP || type == ALERT_FLASH,
            "Invalid alert type %d. Choose ALERT_BEEP or ALERT_FLASH", type);
    int rc = 0;
    switch (type) {
        case ALERT_BEEP:
            rc = beep();
            break;
        case ALERT_FLASH:
            rc = flash();
            break;
        default:
            sentinel("I shouldn't run");
            break;
    }
    check(rc == OK, "Alert failed");
    return 0;
error:
    return -1;
}

/*
 * Dump the contents of a config file to stdout.
 *
 * Parameters:
 *     configptr: the pointer to the configuration struct to read
 *     filepath: the path to the file the config came from (for printing only)
 *
 * Returns:
 *     On success, 0
 *     On failure, -1
 */
int dump_config(configuration *configptr, char *filepath) {
    check(configptr != NULL, "Got NULL configuration pointer!");
    check(filepath != NULL, "Got NULL config path");
    check(strncmp(filepath, "", MAXPATH) != 0, "Got empty config path");
    printf("Current configuration from %s:\n", filepath);
    printf("\tAlert type: %s\n",
            configptr->alert_type == ALERT_BEEP ? "audible" : "visual");
    printf("\tShort break length: %d minutes\n", configptr->short_break_length);
    printf("\tLong break length: %d minutes\n", configptr->long_break_length);
    printf("\tWork session length: %d minutes\n", configptr->work_length);
    printf("\tPomodoros per set: %d\n", configptr->pomodoros_per_set);
    printf("\tNumber of sets: %d\n", configptr->set_count);

    return 0;
error:
    return -1;
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
 *     status_win: pointer to the status window; needed for window calculations
 *     timer_win: pointer to the timer window; needed for window calculations
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

    int status_win_h;
    int status_win_w;
    getmaxyx(status_win, status_win_h, status_win_w);

    wclear(timer_win);
    box(timer_win, 0, 0);
    wrefresh(timer_win);

    wclear(status_win);
    box(status_win, 0, 0);
    wrefresh(status_win);

    sprintf(msg, "%02d:%02d:00", hours, minutes);
    mvwprintw(timer_win, timer_win_h / 2 - 1,
            (timer_win_w-strlen(msg)) / 2 - 1, msg);
    box(timer_win, 0, 0);
    wrefresh(timer_win);
    mvwprintw(status_win, status_win_h / 2,
            (status_win_w-strlen(cur_state_msg)) / 2, "%s", cur_state_msg);
    box(status_win, 0, 0);
    wrefresh(status_win);
    sprintf(msg, "Current set: %d", set_num);
    mvwprintw(status_win, status_win_h / 2 - 1,
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
        mvwprintw(timer_win, timer_win_h / 2 - 1,
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
 *     status_win: pointer to the status window; needed for window calculations
 *     timer_win: pointer to the timer window; needed for window calculations
 *     set_num: current set number; needed for status window
 *     alert_type: the type of alert to use
 * 
 * Return: 0 on sucess, -1 on error
 */
int do_pomodoro_set(Timer *t, int work_len, int short_b_len, int long_b_len,
        int sessions_per_set, WINDOW *status_win, WINDOW *timer_win,
        int set_num, ALERT_TYPE type) {
    int rc = 0;
    check(t != NULL, "Got NULL Timer pointer");

    for (int i = 0; i < sessions_per_set; i++) {
        do_timer_session(t, work_len, POMODORO_WORK, status_win, timer_win,
                set_num);
        clear();
        refresh();
        rc = alert_user(type);
        check(rc == 0, "Terminal alert failure!");
        do_timer_session(t, short_b_len, POMODORO_SHORT_REST,
                status_win, timer_win, set_num);
        rc = alert_user(type);
        check(rc == 0, "Terminal alert failure!");
    }
    clear();
    refresh();
    do_timer_session(t, long_b_len, POMODORO_LONG_REST, status_win, timer_win,
            set_num);
    return rc;
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

static int handler(void *user, const char *section, const char *name,
        const char *value) {
    configuration *pconfig = (configuration *)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("timer", "short_break_length")) {
        pconfig->short_break_length = atoi(value);
    } else if (MATCH("timer", "long_break_length")) {
        pconfig->long_break_length = atoi(value);
    } else if (MATCH("timer", "set_count")) {
        pconfig->set_count = atoi(value);
    } else if (MATCH("timer", "pomodoros_per_set")) {
        pconfig->pomodoros_per_set = atoi(value);
    } else if (MATCH("timer", "work_length")) {
        pconfig->work_length = atoi(value);
    } else if (MATCH("timer", "alert_type")) {
        if (strncmp(value, "beep", 16) == 0) {
            pconfig->alert_type = ALERT_BEEP;
        } else if (strncmp(value, "flash", 16) == 0) {
            pconfig->alert_type = ALERT_FLASH;
        } else {
            sentinel("Bad alert type %s. Choose 'beep' or 'flash'.", value);
        }
    } else {
        sentinel("Bad value in config: %s[%s]", section, name);
    }
    return 1;
error:
    return 0;
}

int main(int argc, char *argv[]) {

    configuration config = {.long_break_length = 0, .pomodoros_per_set = 0,
            .set_count = 0, .short_break_length = 0, .work_length = 0,
            .alert_type = ALERT_UNSET };
    configuration explicit_config = {.long_break_length = 0,
            .pomodoros_per_set = 0, .set_count = 0, .short_break_length = 0,
            .work_length = 0, .alert_type = ALERT_UNSET };

    Timer *pomodoro_timer = NULL;
    WINDOW *status_window = NULL;
    WINDOW *timer_window = NULL;
    /* #### program options #### */
    int opt; // variable for getting options with getopt(3)
    int option_index;

    char *home = getenv("HOME");
    check(home != NULL, "HOME environment variable doesn't exist");
    char *default_config_path = strncat(home, "/.config/", MAXPATH);
    default_config_path = strncat(default_config_path, PROG_NAME, MAXPATH);
    default_config_path = strncat(default_config_path, "/config.ini", MAXPATH);
    check (strnlen(default_config_path, MAXPATH) < MAXPATH,
            "Config path too long");
    
    /* For -c option */
    char *config_file = NULL;

    // Default alert type
    ALERT_TYPE alert_type = ALERT_BEEP;

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

    int rc = ini_parse(default_config_path, handler, &config);
    check(rc != -1, "Error opening default config file '%s'",
            default_config_path);
    check(rc != -2, "ini_parse memory error");
    short_break_length = config.short_break_length;
    long_break_length = config.long_break_length;
    num_sets = config.set_count;
    pomodoros_per_set = config.pomodoros_per_set;
    session_length = config.work_length;
    alert_type = config.alert_type;
    check(rc == 0, "Failed to parse default config file");

    static struct option long_options[] = {
        {"alert-type", required_argument, 0, 'a'},
        {"short-break-length", required_argument, 0, 'b'},
        {"long-break-length", required_argument, 0, 'B'},
        {"config-file", required_argument, 0, 'c'},
        {"dump-config", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"num-sets", required_argument, 0, 'n'},
        {"pomodoros-per-set", required_argument, 0, 'p'},
        {"session-length", required_argument, 0, 's'}
    };

    bool use_custom_config_file = false;
    bool do_config_dump = false;

    while ((opt = getopt_long(argc, argv, "a:b:c:dhn:p:s:B:", long_options,
            &option_index)) != -1) {
        switch (opt) {
            case 'a':
                if (strncmp(optarg, "beep", 16) == 0) {
                    explicit_config.alert_type = ALERT_BEEP;
                } else if (strncmp(optarg, "flash", 16) == 0) {
                    explicit_config.alert_type = ALERT_FLASH;
                } else {
                    explicit_config.alert_type = ALERT_UNSET;
                    log_err("Bad alert type '%s'. Choose 'beep' or 'flash'\n",
                            optarg);
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;
            case 'b':
                explicit_config.short_break_length = atoi(optarg);
                check(short_break_length > 0,
                        "Short break length must be greater than 0");
                break;
            case 'c':
                use_custom_config_file = true;
                config_file = strndup(optarg, MAXPATH);
                rc = ini_parse(config_file, handler, &config);
                check(rc != -1, "Error opening config file '%s'", config_file);
                check(rc != -2, "ini_parse memory error");
                short_break_length = config.short_break_length;
                long_break_length = config.long_break_length;
                num_sets = config.set_count;
                pomodoros_per_set = config.pomodoros_per_set;
                session_length = config.work_length;
                break;
            case 'd':
                do_config_dump = true;
                break;
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 'n':
                explicit_config.set_count = atoi(optarg);
                check(num_sets > 0, "Number of sets must be greater than 0");
                break;
            case 'p':
                explicit_config.pomodoros_per_set = atoi(optarg);
                check(pomodoros_per_set > 0,
                        "Pomodoros per set must be greater than 0");
                break;
            case 's':
                explicit_config.work_length = atoi(optarg);
                check(session_length > 0,
                        "Session length must be greater than 0");
                break;
            case 'B':
                explicit_config.long_break_length = atoi(optarg);
                check(long_break_length > 0,
                        "Long break length must be greater than 0");
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    /*
     * If any command-line args were passed, we need to see if config values
     * need to be overwritten with the correspopnding values from the command
     * line.
     */
    if (explicit_config.alert_type != ALERT_UNSET) {
        alert_type = explicit_config.alert_type;
    }
    if (explicit_config.short_break_length != 0
            && explicit_config.short_break_length != config.short_break_length)
    {
        short_break_length = explicit_config.short_break_length;
    }
    if (explicit_config.long_break_length != 0
            && explicit_config.long_break_length != config.long_break_length) {
        long_break_length = explicit_config.long_break_length;
        
    }
    if (explicit_config.set_count != 0
            && explicit_config.set_count != config.set_count) {
        num_sets = explicit_config.set_count;
    }

    if (explicit_config.pomodoros_per_set != 0
            && explicit_config.pomodoros_per_set != config.pomodoros_per_set)
    {
        pomodoros_per_set = explicit_config.pomodoros_per_set;
    }

    if (explicit_config.work_length != 0
            && explicit_config.work_length != config.work_length) {
        session_length = explicit_config.work_length;
    }

    if (do_config_dump) {
        if (use_custom_config_file) {
            rc = dump_config(&config, config_file);
            check(rc == 0, "dump_config failure");
        } else {
            rc = dump_config(&config, default_config_path);
            check(rc == 0, "dump_config failure");
        }
        exit(EXIT_SUCCESS);
    }

    int row = 0;
    int col = 0;

    /* Has initscr been called? (for error-checking and cleanup purposes) */
    int in_curses_mode = 0;

    initscr(); // start curses mode
    in_curses_mode = 1;
    getmaxyx(stdscr, row, col); // get window dimensions
    check(row >= 10, "Terminal must be >=10 rows tall");
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

    pomodoro_timer = Timer_alloc();
    check(pomodoro_timer != NULL, "Failed to allocate main pomodoro timer.");

    for (int i = 1; i <= num_sets; i++) {
        rc = do_pomodoro_set(pomodoro_timer, session_length, short_break_length,
                long_break_length, pomodoros_per_set, status_window,
                timer_window, i, alert_type);
        check(rc == 0, "Pomodoro set error");
        rc = alert_user(alert_type);
        check(rc == OK, "Could not beep after set");
    }

    getch();

    Timer_destroy(pomodoro_timer);
    pomodoro_timer = NULL;
    destroy_win(status_window);
    destroy_win(timer_window);
    endwin();
    in_curses_mode = 0;
    free(config_file);
    config_file = NULL;

    return 0;
error:
    if (config_file != NULL) {
        free(config_file); // needed because this string came from strndup()
    }
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
