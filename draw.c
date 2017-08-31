#include "draw.h"
#include <stdlib.h>
#include <ncurses.h>

struct Window {
    WINDOW *term;
    WINDOW *tab;
};

// Creates the window for the tab editor
void init_tab_window(struct Window *window);
void draw_tab_window(struct Window *window);

struct Window * init_window() {
    struct Window *w = malloc(sizeof(struct Window));
    w->term = initscr();
    init_tab_window(w);
    draw_tab_window(w);
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




void draw_tab_window(struct Window *window) {
    int y, x;
    getmaxyx(window->term, y, x);
    // Draw tab lines
    for (int y = 7; y < 19; y += 2)
        mvwhline(window->tab, y, 0, '-', x);
    wrefresh(window->tab);
}
