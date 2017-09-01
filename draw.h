#ifndef DRAW_H
#define DRAW_H

#include <ncurses.h>

struct Window {
    WINDOW *term;
    WINDOW *tab;
    WINDOW *cmd;
};

// Creates the main window and initializes ncurses
struct Window * init_window();
// Kills the main window
void kill_window(const struct Window *);
// Draws all the windows
void draw(struct Window *);

#endif
