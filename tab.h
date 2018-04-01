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

enum TechniqueType {
    LegatoTo,
    LegatoFrom,
    SlideTo,
    SlideFrom,
    BendTo,
    BendFrom
};

struct Technique {
    enum TechniqueType type;
    int receiver_measure;
    int receiver_note;
};

struct Note {
    int string;
    int fret;
    int offset;
    int length;

    struct Technique *techniques;
    size_t techniques_n;
    size_t techniques_size;
};

enum Tonality {
    Major,
    Minor
};

struct Key {
    struct PitchClass key_center; // Ignore the octave for this purpose
    enum Tonality tonality;
};

struct KeySignature {
    struct PitchClass notes[7];
};

void pitch_class_to_string(struct PitchClass pitch_class, char *buffer, size_t n);
struct PitchClass string_to_pitch_class(const char *str);
int pitch_class_distance_positive(struct PitchClass a, struct PitchClass b);
int pitch_class_distance_diatonic_positive(struct PitchClass a, struct PitchClass b);

void tone_to_string(struct Tone tone, bool show_octave, char *buffer, size_t n);
struct Tone string_to_tone(const char *str, bool octave);

struct Note string_to_note(const char *str, int string, int offset, int length);

int tones_distance(struct Tone a, struct Tone b);
int tones_distance_diatonic(struct Tone a, struct Tone b);
struct Tone tone_from_note(struct Note n, struct Tab *t, struct KeySignature key_signature);

struct Tone note_to_tone(struct Tab *t, struct Note n, struct KeySignature key_signature);

void key_to_string(struct Key key, char *buffer, size_t n);
bool keys_equal(struct Key a, struct Key b);

struct KeySignature get_key_signature(struct Key key);

// Changes a key signature so a given tone fits into it
// The return value is what should be placed in front of the tone as an accidental
bool key_signature_contains_tone(struct KeySignature key_signature, struct Tone t);
enum PitchShift key_signature_add_tone(struct KeySignature *key_signature, struct Tone t);

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

// Adds a technique to a measure
struct Technique * note_new_technique(struct Note *n, struct Technique t);

// Creates a blank tab with one measure
struct Tab new_tab(struct Tuning t, int tick_rate);

// Loads a tab from a file into the pointed-to tab
void open_tab(struct Tab *tab, const char *file);

// Saves a tab to a file
void save_tab(const struct Tab *tab, const char *file);

#endif
