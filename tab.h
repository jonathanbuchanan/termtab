#ifndef TAB_H
#define TAB_H

#include <stddef.h>
#include <stdio.h>

struct Tab;

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

struct Note {
    int string;
    int fret;
    int offset;
    int length;
};

// Returns a string for any given tone
void tone_to_string(struct Tone tone, char *buffer, size_t n);

// Returns a tone for a string
struct Tone string_to_tone(const char *str);

// Returns a note for a string
struct Note string_to_note(const char *str, int string, int offset, int length);

// Computes the distance (in semitones) between two tones. If b > a, result is positive. If a > b, result is negative.
int tones_distance(struct Tone a, struct Tone b);

// Computes the diatonic distance between two tones
int tones_distance_diatonic(struct Tone a, struct Tone b);

// Returns a tone for another tone and a distance to add to it
struct Tone tone_add_semitones(struct Tone t, int semitones);

// Converts a note to a tone
struct Tone note_to_tone(struct Tab *t, struct Note n);

struct Tuning {
    struct Tone strings[6];
};
#define STANDARD_TUNING (struct Tuning){.strings = {{E, Natural, 4}, {B, Natural, 3}, {G, Natural, 3}, {D, Natural, 3}, {A, Natural, 2}, {E, Natural, 2}}}

struct TabInfo {
    char *title;
    char *band;
    struct Tuning tuning;
};

struct Measure {
    int ts_top, ts_bottom;
    struct Note *notes;
    size_t notes_n;
    size_t notes_size;
};

struct Tab {
    struct TabInfo info;
    char *file;
    struct Measure *measures;
    size_t measures_n;
    size_t measures_size;
    int ticks_per_quarter;
};

// Creates a blank measure with no notes, doubling the array if necessary
struct Measure * new_measure(struct Tab *tab, int ts_top, int ts_bottom);

// Returns the number of ticks in a measure
int measure_get_ticks(struct Tab *tab, int measure);

// Returns a pointer to a note (or NULL if it can't be found) for a given offset and string
struct Note * measure_get_note(struct Tab *tab, int measure, int string, int offset);

// Adds a note to a measure and returns its pointer
struct Note * measure_new_note(struct Tab *tab, int measure, struct Note n);

// Removes a note from a measure
void measure_remove_note(struct Tab *tab, int measure, struct Note *n);

// Creates a blank tab with one measure
struct Tab new_tab(struct Tuning t, int tick_rate);

// Loads a tab from a file into the pointed-to tab
void open_tab(struct Tab *tab, const char *file);

// Saves a tab to a file
void save_tab(const struct Tab *tab, const char *file);

#endif
