#ifndef EDIT_H
#define EDIT_H

#include <ncurses.h>
#include "tab.h"

struct State;

struct LayoutCache {
    int *line_numbers;
    int *offsets;

    int top_row;
};

struct EditingState {
    // The cursor defines a selection of a certain width. All measurements in terms of ticks
    int string;
    int measure;
    int x;
    int cursor_width;

    // Caches the layout of the tab editor
    struct LayoutCache layout;
};



bool edit_input(struct State *s, int c);

void remove_note(struct State *s);
void add_note(struct State *s);
void change_key(struct State *s);

#endif
