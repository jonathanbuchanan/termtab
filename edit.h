#ifndef EDIT_H
#define EDIT_H

#include <ncurses.h>
#include "tab.h"

struct State;
struct EditingState {
    // The cursor defines a selection of a certain width. All measurements in terms of ticks
    int string;
    int measure;
    int x;
    int cursor_width;
};



bool edit_input(struct State *s, int c);

void remove_note(struct State *s);
void add_note(struct State *s);

#endif
