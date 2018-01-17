#ifndef TAB_H
#define TAB_H

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

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

struct PitchClass {
    enum Pitch pitch;
    enum PitchShift shift;
};

struct Tone {
    struct PitchClass pitch_class;
    int octave;
};

struct Note {
    int string;
    int fret;
    int offset;
    int length;
};

enum Tonality {
    Major,
    Minor
};

struct Key {
    struct PitchClass key_center; // Ignore the octave for this purpose
    enum Tonality tonality;
};

void pitch_class_to_string(struct PitchClass pitch_class, char *buffer, size_t n);
struct PitchClass string_to_pitch_class(const char *str);

void tone_to_string(struct Tone tone, bool show_octave, char *buffer, size_t n);
struct Tone string_to_tone(const char *str, bool octave);

struct Note string_to_note(const char *str, int string, int offset, int length);

int tones_distance(struct Tone a, struct Tone b);
int tones_distance_diatonic(struct Tone a, struct Tone b);
struct Tone tone_add_semitones(struct Tone t, int semitones);

struct Tone note_to_tone(struct Tab *t, struct Note n);

void key_to_string(struct Key key, char *buffer, size_t n);

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
    struct Key key;

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
// The new measure will "inherit" its time signature and key from the preceding measure
struct Measure * new_measure(struct Tab *tab);

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
