#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

#include "../config.h"

#define DEFAULT   -1
#define GREEN      1
#define GRAY       2
#define COLOR_GRAY 8 /* ncurses color macros end at 7 */

#define NOTIFY()                                                                \
    char buf[256];                                                              \
    snprintf(buf, 256, "mpv %s --volume=%d > /dev/null 2>&1 &", SOUND, VOLUME); \
    system(buf);                                                                \

#define END() \
    endwin(); \
    exit(1);  \

#define WORK        countdown_timer(intervals[0] * 60, names[0]);
#define SHORT_BREAK countdown_timer(intervals[2] * 60, names[2]);
#define LONG_BREAK  countdown_timer(intervals[3] * 60, names[3]);

char keybindings[] = {'h',    'j',            'k',           'k',          'q'};
char* names[]      = {"WORK", "WORK + BREAK", "SHORT BREAK", "LONG BREAK", "QUIT"};
int intervals[]    = {25,     115,             5,             15,           0}; /* IN MINUTES */

char* spinner[4]   = {"\\", "|", "/", "-"};

void
init_colors(void)
{
    use_default_colors();
    init_color(COLOR_GRAY, 255, 255, 255);
    init_pair(GREEN, COLOR_GREEN, DEFAULT);
    init_pair(GRAY, COLOR_GRAY, DEFAULT);
}

void
display_main_menu(void)
{
    attron(COLOR_PAIR(GREEN));
    attron(A_BOLD);
    mvprintw(0, 1, "CPOM");
    attroff(A_BOLD);
    attroff(COLOR_PAIR(GREEN));

    int num_of_bindings = sizeof keybindings / sizeof keybindings[0];

    for (int i = 0; i < num_of_bindings; i++) {
        int line_len = 1;

        attron(COLOR_PAIR(GREEN));
        mvprintw(i + 1, line_len, "[%c]", keybindings[i]);
        attroff(COLOR_PAIR(GREEN));
        line_len += 4;

        mvprintw(i + 1, line_len, "%s", names[i]);
        line_len += strlen(names[i]) + 1;

        if (intervals[i]) {
            attron(COLOR_PAIR(GRAY));
            mvprintw(i + 1, line_len, "(%dm)", intervals[i]);
            attroff(COLOR_PAIR(GRAY));
        }
    }
}

void
pause_timer(void)
{
    nodelay(stdscr, FALSE);

    mvprintw(3, 1, "Time Paused...");
    mvprintw(4, 1, "[c] Continue");
    mvprintw(5, 1, "[q] Quit");

    refresh();

    switch (getch()) {
    case 'c':
        break;
    case 'q':
        END();
    }

    nodelay(stdscr, TRUE);
    clear();
}

void
countdown_timer(unsigned int time_in_sec, char* type)
{
    nodelay(stdscr, TRUE);
    unsigned int min        = 0;
    unsigned int sec        = 0;
    unsigned int msec       = 0;
    unsigned int total_time = 0;
    unsigned int time_left  = 0;

    clock_t start_time, count_time;

    start_time = clock();
    clear();
    refresh();
    do {
        attron(COLOR_PAIR(GREEN));
        mvprintw(0, 1, "%s", type);
        attroff(COLOR_PAIR(GREEN));
        count_time = clock();
        msec = count_time - start_time;
        sec  = (msec / CLOCKS_PER_SEC) - (min * 60);
        min  = (msec / CLOCKS_PER_SEC) / 60;

        time_left = time_in_sec - sec;

        attron(COLOR_PAIR(GREEN));
        attron(A_BOLD);
        mvprintw(1, 1, "[%s] ", spinner[time_left % 4]);
        attroff(A_BOLD);
        attroff(COLOR_PAIR(GREEN));

        mvprintw(1, 5, "%dm %ds\n", time_left / 60, time_left % 60);
        refresh();

        switch (getch()) {
        case 'p':
            pause_timer();
            break;
        case 'q':
            END();
        }
    } while (time_left > 0);
    clear();
    refresh();

    NOTIFY();
}

void
cpom(void)
{
    nodelay(stdscr, TRUE);
    display_main_menu();

    switch (getch()) {
    case 'h':
        WORK;
        break;
    case 'j':
        WORK;
        SHORT_BREAK;
        WORK;
        SHORT_BREAK;
        WORK;
        SHORT_BREAK;
        WORK;
        break;
    case 'k':
        SHORT_BREAK;
        break;
    case 'l':
        LONG_BREAK;
        break;
    case 't':
        countdown_timer(5, "TESTING");
        countdown_timer(6, "TESTING2");
        break;
    case 'q':
        END();
    default:
        break;
    }
}
