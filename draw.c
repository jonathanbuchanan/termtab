#include "draw.h"
#include "tab.h"
#include <stdlib.h>
#include <string.h>

#define STATUS_WINDOW_HEIGHT 1
#define CMD_WINDOW_HEIGHT 1
#define TAB_WINDOW_HEIGHT(term_height) (term_height - STATUS_WINDOW_HEIGHT - CMD_WINDOW_HEIGHT)

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
    use_default_colors();
    init_tab_window(w);
    init_status_window(w);
    init_cmd_window(w);
    return w;
}

void kill_window(const struct Window *window) {
    endwin();
}

void init_tab_window(struct Window *window) {
    int y, x;
    getmaxyx(window->term, y, x);
    WINDOW *tab = newwin(TAB_WINDOW_HEIGHT(y), x, 0, 0);
    wrefresh(tab);
    window->tab = tab;
}

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

    realloc_cache(&state->edit.layout, tab->measures_n);

    int height, width;
    getmaxyx(window->term, height, width);
    // Title and band
    mvwprintw(window->tab, 0, 0, "Title: %s", tab->info.title);
    mvwprintw(window->tab, 1, 0, "Author: %s", tab->info.band);

    // Draw measures 
    int available_width = width - 8;
    int line_number[state->tab->measures_n];
    int lines = 1;

    int used_width = 0;
    for (int i = 0; i < state->tab->measures_n; ++i) {
        if (used_width + measure_width(tab, &state->tab->measures[i]) <= available_width) {
            // This measure fits
            line_number[i] = lines - 1;
            used_width += measure_width(tab, &state->tab->measures[i]);


            state->edit.layout.line_numbers[i] = lines - 1;
        } else {
            // This measure can't fit, make a new line
            ++lines;
            used_width = 0;
            line_number[i] = lines - 1;
            state->edit.layout.line_numbers[i] = lines - 1;
        }
    }

    int m = 0;
    int offset = 8;
    for (int i = 0; i < lines; ++i) {
        int y = 7 + (14 * i);

        // Draw tuning and lines
        for (int i = 0; i < 6; ++i) {
            int y_line = y + (2 * i);
            char buff[5];
            tone_to_string(tab->info.tuning.strings[i], buff, 5);
            mvwprintw(window->tab, y_line, 0, "%d", i);
            mvwprintw(window->tab, y_line, 2, buff);
            mvwhline(window->tab, y_line, 8, '-', width);
        }

        // Draw seperating line
        mvwvline(window->tab, y, 7, '|', 11);
        mvwhline(window->tab, y + 12, 0, '_', width);

        while (line_number[m] == i) {
            state->edit.layout.offsets[m] = offset;
            offset += draw_measure(window, offset, y, tab, &state->tab->measures[m]);
            ++m;
        }

        offset = 8;
    }

    wrefresh(window->tab);
}

void draw_tab_note_prompt(struct State *state, char *buffer) {
    mvwprintw(state->window->tab, 5, 0, "Enter a note: ");
    wrefresh(state->window->tab);
    begin_input();
    mvwgetstr(state->window->tab, 5, 14, buffer);
    end_input();
    wmove(state->window->tab, 5, 0);
    wclrtoeol(state->window->tab);
    wrefresh(state->window->tab);
}

#define TICKS_PER_COLUMN 2
int draw_measure(struct Window *w, int x, int y, struct Tab *t, struct Measure *m) {
    int width = (m->ts_top * t->ticks_per_quarter * 4) / (m->ts_bottom * TICKS_PER_COLUMN);
    mvwvline(w->tab, y, x + width, '|', 11);
    for (int i = 0; i < m->notes_n; ++i) {
        struct Note *n = &m->notes[i];
        mvwprintw(w->tab, y + (2 * n->string), x + (n->offset / TICKS_PER_COLUMN), "%d", n->fret);
    }
    return width + 1;
}

int measure_width(struct Tab *t, struct Measure *m) {
    return ((m->ts_top * t->ticks_per_quarter * 4) / (m->ts_bottom * TICKS_PER_COLUMN)) + 1;
}

void position_cursor(struct State *state) {
    if (state->mode != Edit)
        return;
    init_pair(2, COLOR_WHITE, COLOR_WHITE);
    struct Window *window = state->window;
    int w = state->edit.cursor_width / TICKS_PER_COLUMN;

    // Find x and y position
    int x = state->edit.layout.offsets[state->edit.measure];
    int y = 7 + (14 * state->edit.layout.line_numbers[state->edit.measure]) + (2 * state->edit.string);



    // Offset within measure
    int offset_x = state->edit.x / TICKS_PER_COLUMN;

    wmove(window->tab, y, x + offset_x);
    wattron(window->tab, COLOR_PAIR(2));
    for (int i = 0; i < w; ++i)
        waddch(window->tab, ' ');
    wattroff(window->tab, COLOR_PAIR(2));

    wrefresh(window->tab);
}



int next_char(struct Window *window) {
    return wgetch(window->cmd);
}

void realloc_cache(struct LayoutCache *cache, int m) {
    cache->line_numbers = realloc(cache->line_numbers, sizeof(int) * m);
    cache->offsets = realloc(cache->offsets, sizeof(int) * m);
}
