#include "draw.h"
#include "tab.h"
#include <stdlib.h>
#include <string.h>

struct Window {
    WINDOW *term;
    WINDOW *tab;
    WINDOW *status;
    WINDOW *cmd;
};

// Creates the window for the tab editor
void init_tab_window(struct Window *window);
void init_status_window(struct Window *window);
void init_cmd_window(struct Window *window);

// Begins text input by turning on echo and cursor
void begin_input();
// Ends text input by turning off echo and cursor
void end_input();

struct Window * init_window() {
    struct Window *w = malloc(sizeof(struct Window));
    w->term = initscr();
    init_tab_window(w);
    init_status_window(w);
    init_cmd_window(w);
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
    mvwhline(tab, TAB_WINDOW_HEIGHT - 1, 0, '_', x);
    wrefresh(tab);
    window->tab = tab;
}

#define STATUS_WINDOW_HEIGHT 1
void init_status_window(struct Window *window) {
    int y, x;
    getmaxyx(window->term, y, x);
    WINDOW *status = newwin(STATUS_WINDOW_HEIGHT, x, y - STATUS_WINDOW_HEIGHT - 1, 0);
    start_color();
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    wbkgd(status, COLOR_PAIR(2));
    wrefresh(status);
    window->status = status;
}

#define CMD_WINDOW_HEIGHT 1
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




void draw(struct State *state) {
    draw_status(state);
    draw_cmd(state);
    draw_tab(state);

    position_cursor(state);
}

void begin_input() {
    curs_set(1);
    echo();
}

void end_input() {
    curs_set(0);
    noecho();
}



void draw_status(struct State *state) {
    int x, y;
    getmaxyx(state->window->status, y, x);

    werase(state->window->status);
    mvwprintw(state->window->status, 0, 0, state->msg);
    const char *mode;
    switch (state->mode) {
    case Command:
        mode = "COMMAND";
        break;
    case Edit:
        mode = "EDIT";
        break;
    }
    mvwprintw(state->window->status, 0, x - strlen(mode), mode);
    wrefresh(state->window->status);
}

void draw_cmd(struct State *state) {
    werase(state->window->cmd);
    wrefresh(state->window->cmd);
}

void draw_cmd_prompt(struct State *state, char *buffer) {
    werase(state->window->cmd);
    mvwaddch(state->window->cmd, 0, 0, ':');
    wrefresh(state->window->cmd);
    begin_input();
    mvwgetstr(state->window->cmd, 0, 1, buffer);
    end_input();
}

void draw_tab(struct State *state) {
    struct Window *window = state->window;
    struct Tab *tab = state->tab;

    int height, width;
    getmaxyx(window->term, height, width);
    // Title and band
    mvwprintw(window->tab, 0, 0, "Title: %s", tab->info.title);
    mvwprintw(window->tab, 1, 0, "Author: %s", tab->info.band);

    // Draw tuning and lines
    for (int i = 0; i < 6; ++i) {
        int y = 7 + (2 * i);
        char buff[5];
        tone_to_string(tab->info.tuning.strings[i], buff, 5);
        mvwprintw(window->tab, y, 0, "%d", i);
        mvwprintw(window->tab, y, 2, buff);
        mvwhline(window->tab, y, 8, '-', width);
    }

    // Draw seperating line
    mvwvline(window->tab, 7, 7, '|', 11);

    // Draw barlines
    for (int i = 0; i < state->tab->measures_n; ++i) {
        int x = 7 + (9 * (i + 1));
        mvwvline(window->tab, 7, x, '|', 11);
    }

    mvwhline(window->tab, TAB_WINDOW_HEIGHT - 1, 0, '_', width);
    wrefresh(window->tab);
}

void position_cursor(struct State *state) {
    struct Window *window = state->window;

    if (state->mode == Command)
        curs_set(0);
    else if (state->mode == Edit)
        curs_set(1);
    // Move cursor
    wmove(window->tab, 7 + (2 * state->edit.string), 8);

    wrefresh(window->tab);
}



int next_char(struct Window *window) {
    return wgetch(window->cmd);
}
