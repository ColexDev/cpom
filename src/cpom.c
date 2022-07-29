#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>

#include "../config.h"

#define GREEN 1
#define GRAY 2
#define COLOR_GRAY 8
#define DEFAULT -1

#define notify      \
    char buf[256];  \
    snprintf(buf, "mpv %s --volume %d > /dev/null 2>&1 &", SOUND, VOLUME); \
    system(buf);    \

char keybindings[] = {'h',    'j',         'k',           'k',          'q'};
char* names[]      = {"WORK", "LONG WORK", "SHORT BREAK", "LONG BREAK", "QUIT"};
int intervals[]    = {25,     115,         5,             15,           0}; /* IN MINUTES */

char* spinner[4] = {"\\", "|", "/", "-"};

void init_colors(void)
{
    use_default_colors();
    init_color(COLOR_GRAY, 255, 255, 255);
    init_pair(GREEN, COLOR_GREEN, DEFAULT);
    init_pair(GRAY, COLOR_GRAY, DEFAULT);
}

void display_main_menu(void)
{
    attron(COLOR_PAIR(GREEN));
    attron(A_BOLD);
    mvprintw(0, 1, "CPOM");
    attroff(A_BOLD);
    attroff(COLOR_PAIR(GREEN));

    for (int i = 0; i < sizeof(keybindings) / sizeof(keybindings[0]); i++) {
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

void countdown_timer(unsigned int time_in_sec, char* type)
{
    unsigned int min  = 0;
    unsigned int sec  = 0;
    unsigned int msec = 0;
    unsigned int total_time = 0;
    unsigned int time_left  = 0;

    clock_t start_time, count_time;

    char buf[256];

    start_time = clock();
    clear();
    refresh();
    attron(COLOR_PAIR(GREEN));
    mvprintw(0, 1, "%s", type);
    attroff(COLOR_PAIR(GREEN));
    do {
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
    } while (time_left > 0);
    clear();
    refresh();

    snprintf(buf, 256, "mpv %s --volume=%d > /dev/null 2>&1 &", SOUND, VOLUME);
    system(buf);
}

void cpom(void)
{
    display_main_menu();

    switch (getch()) {
    case 'h':
        countdown_timer(1500, "WORK");
        break;
    case 'j':
        countdown_timer(6900, "LONG WORK");
        break;
    case 'k':
        countdown_timer(300, "SHORT BREAK");
        break;
    case 'l':
        countdown_timer(900, "LONG BREAK");
        break;
    case 'f':
        countdown_timer(5, "LOL");
        break;
    case 'q':
        endwin();
        exit(0);
    default:
        break;
    }
}
