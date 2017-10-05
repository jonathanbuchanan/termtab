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

// Returns a tone for a string
struct Tone string_to_tone(const char *str);

struct Tuning {
    struct Tone strings[6];
};
#define STANDARD_TUNING {.strings = {{E, Natural, 4}, {B, Natural, 3}, {G, Natural, 3}, {D, Natural, 3}, {A, Natural, 2}, {E, Natural, 2}}}

struct TabInfo {
    char *title;
    char *band;
    struct Tuning tuning;
};

struct Tab {
    struct TabInfo info;
    char *file;
};

// Loads a tab from a file into the pointed-to tab
void open_tab(struct Tab *tab, const char *file);

// Saves a tab to a file
void save_tab(const struct Tab *tab, const char *file);

#endif
