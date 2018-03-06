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

// In select mode, the user is prompted to pick a note
// The callback is called with a pointer to the chosen note after selection
struct SelectState {
    void (*callback)(struct State *s, struct Note *, void *userdata);
    void *userdata;
    int previousMode;
};



bool edit_input(struct State *s, int c);

void remove_note(struct State *s);
void add_note(struct State *s);
void change_key(struct State *s);
void add_technique(struct State *s);
void add_technique_callback(struct State *s, struct Note *n, void *userdata);



void enter_select_mode(struct State *s, void (*callback)(struct State *s, struct Note *, void *userdata), void *userdata, int previousMode);
bool select_input(struct State *s, int c);

#endif
