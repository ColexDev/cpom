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

#define CONTINUE 0
#define SKIP     1

#define NOTIFY()                                                                \
    char buf[256];                                                              \
    snprintf(buf, 256, "mpv %s --volume=%d > /dev/null 2>&1 &", SOUND, VOLUME); \
    system(buf);                                                                \

#define QUIT() \
    endwin();  \
    exit(1);   \

#define CLEANUP_TIMER()                         \
    clear();                                    \
    refresh();                                  \
    if (strcmp(type, "BREAK") != 0) {           \
        total_time_spent_sec += ct->total_secs; \
    }                                           \
    free(ct);                                   \


#define WORK  countdown_timer(intervals[0] * 60, names[0]);
#define BREAK countdown_timer(intervals[1] * 60, names[1]);

char keybindings[] = {'w',     'b',     'q'   };
char* names[]      = {"WORK",  "BREAK", "QUIT"};
int intervals[]    = {60,      10,      0     }; /* IN MINUTES */

char* spinner[4]   = {"\\", "|", "/", "-"};

unsigned int total_time_spent_sec = 0;
unsigned int total_time_spent_min = 0;

typedef struct {
    unsigned int min;
    unsigned int sec;
    unsigned int msec;
    unsigned int total_time;
    unsigned int total_secs;
    unsigned int time_left;
} Countdown_time;

void
init_countdown_time(Countdown_time* ct)
{
    ct->min        = 0;
    ct->sec        = 0;
    ct->msec       = 0;
    ct->total_time = 0;
    ct->total_secs = 0;
    ct->time_left  = 0;
}

void
init_colors(void)
{
    use_default_colors();
    init_color(COLOR_GRAY, 255, 255, 255);
    init_pair(GREEN, COLOR_GREEN, DEFAULT);
    init_pair(GRAY, COLOR_GRAY, DEFAULT);
}

void
display_total_time(void)
{
    while (total_time_spent_sec >= 60) {
        total_time_spent_min += 1;
        total_time_spent_sec -= 60;
    }

    mvprintw(5, 1, "Time Spent This Session: ");
    attron(COLOR_PAIR(GRAY));
    mvprintw(5, 26, "%dm %ds", total_time_spent_min, total_time_spent_sec);
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

    int num_of_bindings = sizeof(keybindings) / sizeof(keybindings[0]);

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
repeat:
    nodelay(stdscr, FALSE);

    /* FIX: Change this to the printing method in countdown_timer, using arrays */
    mvprintw(3, 1, "Time Paused...");
    attron(COLOR_PAIR(GREEN));
    mvprintw(4, 1, "[c] ");
    attroff(COLOR_PAIR(GREEN));
    mvprintw(4, 5, "Continue");
    attron(COLOR_PAIR(GREEN));
    mvprintw(5, 1, "[s] ");
    attroff(COLOR_PAIR(GREEN));
    mvprintw(5, 5, "Skip");
    attron(COLOR_PAIR(GREEN));
    mvprintw(6, 1, "[q] ");
    attroff(COLOR_PAIR(GREEN));
    mvprintw(6, 5, "Quit");

    refresh();

    switch (getch()) {
    case 'c':
        break;
    case 's':
        return SKIP;
    case 'q':
        QUIT();
    default:
        goto repeat;
    }

    nodelay(stdscr, TRUE);
    clear();
    return CONTINUE;
}

void
display_time_left(Countdown_time* ct)
{
    attron(COLOR_PAIR(GREEN));
    attron(A_BOLD);
    mvprintw(1, 1, "[%s] ", spinner[ct->time_left % 4]);
    attroff(A_BOLD);
    attroff(COLOR_PAIR(GREEN));

    mvprintw(1, 5, "%dm %ds\n", ct->time_left / 60, ct->time_left % 60);
    refresh();
}

void
countdown_timer(unsigned int time_in_sec, char* type)
{
    nodelay(stdscr, TRUE);
    Countdown_time* ct = malloc(sizeof(Countdown_time));
    init_countdown_time(ct);

    clock_t start_time, count_time;

    start_time = clock();
    clear();
    refresh();
    do {
        attron(COLOR_PAIR(GREEN));
        mvprintw(0, 1, "%s", type);
        attroff(COLOR_PAIR(GREEN));
        count_time = clock();
        ct->msec       = count_time - start_time;
        ct->total_secs = (ct->msec / CLOCKS_PER_SEC);
        ct->sec        = (ct->total_secs - (ct->min * 60));
        ct->min        = ((ct->msec / CLOCKS_PER_SEC) / 60);

        ct->time_left  = time_in_sec - ct->total_secs;

        display_time_left(ct);

        switch (getch()) {
        case 'p':
            if (pause_timer() == SKIP) {
                CLEANUP_TIMER();
                return;
            }
            break;
        case 'q':
            free(ct);
            QUIT();
        }
    } while (ct->time_left > 0);

    CLEANUP_TIMER();
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
    case 'b':
        BREAK;
        break;
    case 'q':
        QUIT();
    default:
        break;
    }
}
