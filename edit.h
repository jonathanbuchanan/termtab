#ifndef EDIT_H
#define EDIT_H

#include <ncurses.h>

struct State;
struct EditingState {
    int string;
    int x;
};



bool edit_input(struct State *s, int c);

#endif
