#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dbg.h"
#include "pomodoro.h"

const char *prog_name = "pomodoro_curses";

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

    initscr(); // start curses mode
    getmaxyx(stdscr, row, col); // get window dimensions
    raw(); // no line-buffered input
    keypad(stdscr, TRUE);
    noecho(); // don't echo on getch()

    getch();

    endwin();

    return 0;
}
