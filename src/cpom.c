#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

#include "../config.h"
#include "../include/cpom.h"

#define DEFAULT   -1
#define GREEN      1
#define GRAY       2
#define COLOR_GRAY 8 /* ncurses color macros end at 7 */

#define NOTIFY()                                                                \
    char buf[256];                                                              \
    snprintf(buf, 256, "mpv %s --volume=%d > /dev/null 2>&1 &", SOUND, VOLUME); \
    system(buf);                                                                \

#define QUIT() \
    endwin(); \
    exit(1);  \

#define WORK        countdown_timer(intervals[0] * 60, names[0]);
#define SHORT_BREAK countdown_timer(intervals[2] * 60, names[2]);
#define LONG_WORK   WORK; SHORT_BREAK; WORK; SHORT_BREAK; WORK; SHORT_BREAK; WORK;

char keybindings[] = {'w',    'l',            's',           'q'   };
char* names[]      = {"WORK", "WORK + BREAK", "SHORT BREAK", "QUIT"};
int intervals[]    = {25,      115,           5,             0     }; /* IN MINUTES */

char* spinner[4]   = {"\\", "|", "/", "-"};

unsigned int total_time_spent_sec = 0;
unsigned int total_time_spent_min = 0;

void
init_colors(void)
{
    use_default_colors();
    init_color(COLOR_GRAY, 255, 255, 255);
    init_pair(GREEN, COLOR_GREEN, DEFAULT);
    init_pair(GRAY, COLOR_GRAY, DEFAULT);
}

void display_total_time(void)
{
    while (total_time_spent_sec >= 60) {
        total_time_spent_min += 1;
        total_time_spent_sec -= 60;
    }

    mvprintw(6, 1, "Time Spent This Session: ");
    attron(COLOR_PAIR(GRAY));
    mvprintw(6, 26, "%dm %ds", total_time_spent_min, total_time_spent_sec);
    attroff(COLOR_PAIR(GRAY));
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
    display_total_time();
}

int
pause_timer(void)
{
    nodelay(stdscr, FALSE);

    mvprintw(3, 1, "Time Paused...");
    mvprintw(4, 1, "[c] Continue");
    mvprintw(5, 1, "[s] Skip");
    mvprintw(6, 1, "[q] Quit");

    refresh();

    switch (getch()) {
    case 'c':
        break;
    case 's':
        return 1;
    case 'q':
        QUIT();
    }

    nodelay(stdscr, TRUE);
    clear();
    return 0;
}

void
countdown_timer(unsigned int time_in_sec, char* type)
{
    nodelay(stdscr, TRUE);
    unsigned int min        = 0;
    unsigned int sec        = 0;
    unsigned int msec       = 0;
    unsigned int total_time = 0;
    unsigned int total_secs = 0;
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
        msec       = count_time - start_time;
        total_secs = (msec / CLOCKS_PER_SEC);
        sec        = (total_secs - (min * 60));
        min        = ((msec / CLOCKS_PER_SEC) / 60);

        time_left = time_in_sec - total_secs;

        attron(COLOR_PAIR(GREEN));
        attron(A_BOLD);
        mvprintw(1, 1, "[%s] ", spinner[time_left % 4]);
        attroff(A_BOLD);
        attroff(COLOR_PAIR(GREEN));

        mvprintw(1, 5, "%dm %ds\n", time_left / 60, time_left % 60);
        refresh();

        switch (getch()) {
        case 'p':
            if (pause_timer()) {
                clear();
                refresh();
                if (strcmp(type, "SHORT BREAK") != 0) {
                    total_time_spent_sec += total_secs;
                }
                return;
            }
            break;
        case 'q':
            QUIT();
        }
    } while (time_left > 0);

    if (strcmp(type, "SHORT BREAK") != 0) {
        total_time_spent_sec += total_secs;
    }

    clear();
    refresh();

    NOTIFY();
}

void
cpom(void)
{
    nodelay(stdscr, TRUE);
    display_main_menu();

    /* FIX: Figure out how to use keybindings array for this switch */
    switch (getch()) {
    case 'w':
        WORK;
        break;
    case 's':
        SHORT_BREAK;
        break;
    case 'l':
        LONG_WORK;
        break;
    case 't':
        countdown_timer(5, "TESTING");
        break;
    case 'q':
        QUIT();
    default:
        break;
    }
}
