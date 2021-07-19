#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dbg.h"
#include "pomodoro.h"


int main(int argc, char *argv[]) {
    /* #### program options #### */
    int opt; // variable for getting options with getopt(3)

    // Default pomodoro (work session) length, in minutes
    short int session_length = 25;

    // Default short break length, in minutes
    short int short_break_length = 5;

    // Number of sessions before a long break
    short int pomodoros_per_set = 3;

    // Default long break length, in minutes
    short int long_break_length = 30;

    while ((opt = getopt(argc, argv, "b:p:s:B:")) != -1) {
        switch (opt) {
            case 'b':
                short_break_length = atoi(optarg);
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
                log_err("Usage message not implemented.");
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
