#ifndef DRAW_H
#define DRAW_H

#include <ncurses.h>

struct Window;
struct Tab;

// Creates the main window and initializes ncurses
struct Window * init_window();
// Kills the main window
void kill_window(const struct Window *);
// Draws all the windows
void draw(struct Window *);
void draw_with_tab(struct Window *, struct Tab *);

void draw_status_window_clear(struct Window *);
void draw_status_window_blank(struct Window *);
void draw_status_window_msg(struct Window *, const char *);

void draw_cmd_window_blank(struct Window *);
void draw_cmd_window_prompt(struct Window *, char *);

void draw_tab_window_blank(struct Window *);
void draw_tab_window(struct Window *, struct Tab *);

int cmd_getch(struct Window *);

#endif
