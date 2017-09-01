#ifndef DRAW_H
#define DRAW_H

#include <ncurses.h>

struct Window;

// Creates the main window and initializes ncurses
struct Window * init_window();
// Kills the main window
void kill_window(const struct Window *);
// Draws all the windows
void draw(struct Window *);
void draw_cmd_window_blank(struct Window *);
void draw_cmd_window_prompt(struct Window *, char *);

#endif
