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
    wattron(status, COLOR_PAIR(1));
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

    const char *mode;
    char format[256];
    memset(format, '\0', 256);
    switch (state->mode) {
        case Command:
            mode = "COMMAND";
            break;
        case Edit:
            mode = "EDIT";

            struct Measure *m = &state->tab->measures[state->edit.measure];
            struct Note *n = measure_get_note(state->tab, state->edit.measure, state->edit.string, state->edit.x);

            char key[10];
            key_to_string(m->key, key, 10);

            // Measure: [m]
            if (n == NULL)
                snprintf(format, 256, "Measure: [%d] | Time Signature: [%d/%d] | Key: [%s]", state->edit.measure, m->ts_top, m->ts_bottom, key);
            else {
                struct Tone t = tone_add_semitones(state->tab->info.tuning.strings[n->string], n->fret); 
                char note[8];
                tone_to_string(t, true, note, 8);
                snprintf(format, 256, "Measure: [%d] | Time Signature: [%d/%d] | Key: [%s] | Note: [%s]", state->edit.measure, m->ts_top, m->ts_bottom, key, note);
            }
            break;
    }



    if (strlen(state->msg) != 0)
        mvwprintw(state->window->status, 0, 0, state->msg);
    else {
        mvwprintw(state->window->status, 0, 0, format);
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

    werase(window->tab);

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
            used_width = measure_width(tab, &state->tab->measures[i]);
            line_number[i] = lines - 1;
            state->edit.layout.line_numbers[i] = lines - 1;
        }
    }

    // The measure being edited should be in the middle of the three lines
    int top_line = line_number[state->edit.measure] - 1;
    if (line_number[state->edit.measure] == lines - 1 && lines > 2)
        top_line = lines - 3;
    if (line_number[state->edit.measure] == 0)
        top_line = 0;

    state->edit.layout.top_row = top_line;

    int displayed_lines = 3;
    if (lines < 3)
        displayed_lines = lines;
    
    int m = 0;
    while (1) {
        if (line_number[m] == top_line)
            break;
        ++m;
    }

    int offset = 8;
    for (int i = 0; i < displayed_lines; ++i) {
        int line = i + top_line;
        int y = 7 + (14 * i);

        // Draw tuning and lines
        for (int i = 0; i < 6; ++i) {
            int y_line = y + 1 + (2 * i);
            char buff[5];
            tone_to_string(tab->info.tuning.strings[i], true, buff, 5);
            mvwprintw(window->tab, y_line, 0, "%d", i);
            mvwprintw(window->tab, y_line, 2, buff);
            mvwhline(window->tab, y_line, 8, '-', width);
        }

        // Draw seperating line
        mvwvline(window->tab, y + 1, 7, '|', 11);
        mvwhline(window->tab, y + 12, 0, '_', width);

        while (line_number[m] == line && m < state->tab->measures_n) {
            state->edit.layout.offsets[m] = offset;
            offset += draw_measure(window, offset, y, tab, m);
            ++m;
        }

        offset = 8;
    }

    wrefresh(window->tab);
}

void draw_tab_note_prompt(struct State *state, char *buffer) {
    mvwprintw(state->window->tab, 3, 0, "Enter a note: ");
    wrefresh(state->window->tab);
    begin_input();
    mvwgetstr(state->window->tab, 3, 14, buffer);
    end_input();
    wmove(state->window->tab, 3, 0);
    wclrtoeol(state->window->tab);
    wrefresh(state->window->tab);
}

void draw_tab_key_prompt(struct State *state, char *buffer_key_center, char *buffer_tonality) {
    mvwprintw(state->window->tab, 3, 0, "Enter the key center: ");
    wrefresh(state->window->tab);
    begin_input();
    mvwgetstr(state->window->tab, 3, 22, buffer_key_center);
    end_input();

    mvwprintw(state->window->tab, 4, 0, "Choose the tonality (1=major, 2=minor): ");
    wrefresh(state->window->tab);
    begin_input();
    mvwgetstr(state->window->tab, 4, 40, buffer_tonality);
    end_input();

    wmove(state->window->tab, 3, 0);
    wclrtoeol(state->window->tab);
    wmove(state->window->tab, 4, 0);
    wclrtoeol(state->window->tab);

    wrefresh(state->window->tab);
}

#define TICKS_PER_COLUMN 2
int draw_measure(struct Window *w, int x, int y, struct Tab *t, int measure) {
    struct Measure *m = &t->measures[measure];
    int width = (m->ts_top * t->ticks_per_quarter * 4) / (m->ts_bottom * TICKS_PER_COLUMN);
    mvwvline(w->tab, y + 1, x + width, '|', 11);
    mvwprintw(w->tab, y, x, "%d", measure);
    for (int i = 0; i < m->notes_n; ++i) {
        struct Note *n = &m->notes[i];
        mvwprintw(w->tab, y + 1 + (2 * n->string), x + (n->offset / TICKS_PER_COLUMN), "%d", n->fret);
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

    int line = state->edit.layout.line_numbers[state->edit.measure] - state->edit.layout.top_row;

    // Find x and y position
    int x = state->edit.layout.offsets[state->edit.measure];
    int y = 8 + (14 * line) + (2 * state->edit.string);

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
