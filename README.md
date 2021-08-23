# pomodoro_curses

A simple pomodoro timer written with the Ncurses library

## Dependencies
* GNU `make`
* `libinih`
* `ncurses`

## Features
- [x]  `ncurses` interface
- [x] Separate timer and status windows
    - [x] User-friendly status messages to indicate whether working or on break
- [x] Command-line flags to control program settings
- [x] Configuration file to persistently store preferred settings
- [x] Basic bell indicator of beginnings and ends of time periods
- [ ] Better sound playback to indicate the beginnings and ends of time periods
    - [ ] Sound for the start of a work session
    - [ ] Sound for the end of a work session (beginning of a break)
       - [ ] Sound for the beginning of a short break
       - [ ] Sound for the beginning of a long break
- [x] `man` page documenting the program
    - [x] Installation of `man` page in an appropriate location to be found by
      `man`

## Installation
1. Ensure dependencies are installed.
2. `make`
    - This will compile the program and run all unit tests
3. `make install`
    - Currently only installs per user, so elevated privileges are not needed

## Notes

* `src/dbg.h` and `src/minunit.h` come from [Zed Shaw's *Learn C The Hard
Way*](https://learncodethehardway.org/c/) book. As noted in the first response
to [this forum
thread](https://forum.learncodethehardway.com/t/dbg-h-licence/2178), people are
freely encouraged to use this code as needed.
* The bell indicator currently requires the terminal's visual bell to be
enabled. Audible bells currently do not seem to work, at least on the system on
which this program is developed. This also means that the terminal must be
visible onscreen.
