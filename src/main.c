#include <ncurses.h>
#include <string.h>
#include <unistd.h>

#include "pomodoro.h"


int main() {
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
