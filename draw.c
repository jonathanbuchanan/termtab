#include "draw.h"
#include <stdlib.h>

struct Window {
    WINDOW *term;
    WINDOW *tab;
    WINDOW *cmd;
};

// Creates the window for the tab editor
void init_tab_window(struct Window *window);
void draw_tab_window(struct Window *window);

// Creates the window for the command line
void init_cmd_window(struct Window *window);

// Begins text input by turning on echo and cursor
void begin_input();
// Ends text input by turning off echo and cursor
void end_input();

struct Window * init_window() {
    struct Window *w = malloc(sizeof(struct Window));
    w->term = initscr();
    init_tab_window(w);
    init_cmd_window(w);
    draw(w);
    return w;
}

void kill_window(const struct Window *window) {
    endwin();
}

#define TAB_WINDOW_HEIGHT 20
void init_tab_window(struct Window *window) {
    int y, x;
    getmaxyx(window->term, y, x);
    WINDOW *tab = newwin(TAB_WINDOW_HEIGHT, x, 0, 0);
    wborder(tab, ' ', ' ', ' ', '_', ' ', ' ', '_', '_');
    wrefresh(tab);
    window->tab = tab;
}

#define CMD_WINDOW_HEIGHT 2
void init_cmd_window(struct Window *window) {
    int y, x;
    getmaxyx(window->term, y, x);
    WINDOW *cmd = newwin(CMD_WINDOW_HEIGHT, x, y - CMD_WINDOW_HEIGHT, 0);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    wbkgd(cmd, COLOR_PAIR(1));
    wrefresh(cmd);
    window->cmd = cmd;
}




void draw(struct Window *window) {
    draw_tab_window(window);
    draw_cmd_window_blank(window);
}

void begin_input() {
    curs_set(1);
    echo();
}

void end_input() {
    curs_set(0);
    noecho();
}




void draw_tab_window(struct Window *window) {
    int y, x;
    getmaxyx(window->term, y, x);
    // Draw tab lines
    for (int y = 7; y < 19; y += 2)
        mvwhline(window->tab, y, 0, '-', x);
    wrefresh(window->tab);
}

void draw_cmd_window_blank(struct Window *window) {
    werase(window->cmd);
    wrefresh(window->cmd);
}

void draw_cmd_window_prompt(struct Window *window, char *buffer) {
    begin_input();
    mvwaddch(window->cmd, 1, 0, ':');
    wrefresh(window->cmd);
    mvwgetstr(window->cmd, 1, 1, buffer);
    end_input();
}
