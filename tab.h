#ifndef TAB_H
#define TAB_H

#include <stddef.h>

enum Note {
    A,
    B,
    C,
    D,
    E,
    F,
    G
};

enum NoteShift {
    DoubleFlat,
    Flat,
    Natural,
    Sharp,
    DoubleSharp
};

struct Tone {
    enum Note note;
    enum NoteShift shift;
    int octave;
};

// Returns a string for any given tone
void tone_to_string(struct Tone tone, char *buffer, size_t n);

struct Tuning {
    struct Tone string_1, string_2, string_3, string_4, string_5, string_6;
};
#define STANDARD_TUNING (struct Tuning){{E, Natural, 4}, {B, Natural, 3}, {G, Natural, 3}, {D, Natural, 3}, {A, Natural, 2}, {E, Natural, 2}}

struct TabInfo {
    char *title;
    char *band;
    struct Tuning tuning;
};

struct Tab {
    struct TabInfo info;
};

// Loads a tab from a file
void open_tab(const char *file);

#endif
