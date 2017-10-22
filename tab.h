#ifndef TAB_H
#define TAB_H

#include <stddef.h>

enum Pitch {
    A,
    B,
    C,
    D,
    E,
    F,
    G
};

enum PitchShift {
    DoubleFlat,
    Flat,
    Natural,
    Sharp,
    DoubleSharp
};

struct Tone {
    enum Pitch note;
    enum PitchShift shift;
    int octave;
};

// Returns a string for any given tone
void tone_to_string(struct Tone tone, char *buffer, size_t n);

// Returns a tone for a string
struct Tone string_to_tone(const char *str);

struct Tuning {
    struct Tone strings[6];
};
#define STANDARD_TUNING (struct Tuning){.strings = {{E, Natural, 4}, {B, Natural, 3}, {G, Natural, 3}, {D, Natural, 3}, {A, Natural, 2}, {E, Natural, 2}}}

struct TabInfo {
    char *title;
    char *band;
    struct Tuning tuning;
};

struct Note {
    int string;
    int fret;
    int length;
};

struct Measure {
    int ts_top, ts_bottom;
    struct Note *notes;
    size_t notes_n;
};

struct Tab {
    struct TabInfo info;
    char *file;
    struct Measure *measures;
    size_t measures_n;
    size_t measures_size;
};

// Creates a blank measure with no notes, doubling the array if necessary
struct Measure * new_measure(struct Tab *tab, int ts_top, int ts_bottom);

// Creates a blank tab with one measure
struct Tab new_tab(struct Tuning t);

// Loads a tab from a file into the pointed-to tab
void open_tab(struct Tab *tab, const char *file);

// Saves a tab to a file
void save_tab(const struct Tab *tab, const char *file);

#endif
