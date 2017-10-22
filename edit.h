#ifndef EDIT_H
#define EDIT_H

#include <ncurses.h>
#include "tab.h"

struct State;
struct EditingState {
    int string;
    struct Measure *measure;
    int x;
};



bool edit_input(struct State *s, int c);

#endif
