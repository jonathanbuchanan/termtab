#ifndef DRAW_H
#define DRAW_H

#include <ncurses.h>
#include "cmd.h"

struct Window;
struct Tab;

// Creates the main window and initializes ncurses
struct Window * init_window();
// Kills the main window
void kill_window(const struct Window *);
// Draws all the windows
void draw(struct State *);

void draw_status(struct State *);
void draw_cmd(struct State *);
void draw_cmd_prompt(struct State *, char *buffer);
void draw_tab(struct Window *, struct Tab *);

int next_char(struct Window *);

#endif
