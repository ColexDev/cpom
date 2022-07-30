#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "../include/cpom.h"

int
main(int argc, char* argv[])
{
    initscr();
    start_color();
    init_colors();
    noecho();
    cbreak();
    curs_set(0);

    while (1) {
        cpom();
    }

    endwin();
}
