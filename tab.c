#include "tab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

void tone_to_string(struct Tone tone, char *buffer, size_t n) {
    char *note;
    switch (tone.note) {
        case A: note = "A"; break;
        case B: note = "B"; break;
        case C: note = "C"; break;
        case D: note = "D"; break;
        case E: note = "E"; break;
        case F: note = "F"; break;
        case G: note = "G"; break;
    }

    char *shift;
    switch (tone.shift) {
        case DoubleFlat: shift = "bb"; break;
        case Flat: shift = "b"; break;
        case Natural: shift = ""; break;
        case Sharp: shift = "#"; break;
        case DoubleSharp: shift = "##"; break;
    }

    snprintf(buffer, n, "%s%s%d", note, shift, tone.octave);
}

struct Tone string_to_tone(const char *str) {
    struct Tone t;
    const char *i = str;

    // Note
    char note = *str;
    switch (note) {
        case 'a': t.note = A; break;
        case 'A': t.note = A; break;
        case 'b': t.note = B; break;
        case 'B': t.note = B; break;
        case 'c': t.note = C; break;
        case 'C': t.note = C; break;
        case 'd': t.note = D; break;
        case 'D': t.note = D; break;
        case 'e': t.note = E; break;
        case 'E': t.note = E; break;
        case 'f': t.note = F; break;
        case 'F': t.note = F; break;
        case 'g': t.note = G; break;
        case 'G': t.note = G; break;
    }
    ++i;

    // Shift
    if (!isdigit(*i)) {
        if (strncmp(i, "bb", 2) == 0) {
            t.shift = DoubleFlat;
            i += 2;
        } else if (strncmp(i, "b", 1) == 0) {
            t.shift = Flat;
            ++i;
        } else if (strncmp(i, "#", 1) == 0) {
            t.shift = Sharp;
            ++i;
        } else if (strncmp(i, "##", 2) == 0) {
            t.shift = DoubleSharp;
            i += 2;
        }
    } else {
        t.shift = Natural;
    }

    // Octave
    t.octave = strtol(i, NULL, 10);
    if (errno == ERANGE || errno == EINVAL) {
        // Uh oh
    }

    return t;
}

struct Note string_to_note(const char *str, int string_number, int offset, int length) {
    struct Note n;

    n.fret = strtol(str, NULL, 10);
    n.string = string_number;
    n.offset = offset;
    n.length = length;

    return n;
}

int tones_distance(struct Tone a, struct Tone b) {
    const int distance_from_C[] = {
        9,  // A
        11, // B
        0,  // C
        2,  // D
        4,  // E
        5,  // F
        7,  // G
    };

    const int shift[] = {
        -2, // Double Flat
        -1, // Flat
        0,  // Natural
        1,  // Sharp
        2,  // Double Sharp
    };

    const int distance_per_octave = 12;

    return ((b.octave - a.octave) * distance_per_octave) + ((distance_from_C[b.note] + shift[b.shift]) - (distance_from_C[a.note] + shift[a.shift]));
}

int tones_distance_diatonic(struct Tone a, struct Tone b) {
    const int distance_from_C[] = {
        5, // A
        6, // B
        0, // C
        1, // D
        2, // E
        3, // F
        4  // G
    };

    const int distance_per_octave = 7;

    return ((b.octave - a.octave) * distance_per_octave) + (distance_from_C[b.note] - distance_from_C[a.note]);
}

struct Tone tone_increment_diatonic(struct Tone t) {
    if (t.note != G && t.note != B)
        return (struct Tone){t.note + 1, Natural, t.octave};
    else if (t.note == B)
        return (struct Tone){t.note + 1, Natural, t.octave + 1};
    else// if (t.note == G)
        return (struct Tone){A, Natural, t.octave};
}

struct Tone tone_increment_semitone(struct Tone t) {
    switch (t.shift) {
        case DoubleFlat: return (struct Tone){t.note, Flat, t.octave}; break;
        case Flat: return (struct Tone){t.note, Natural, t.octave}; break;
        case Natural: return (struct Tone){t.note, Sharp, t.octave}; break;
        case Sharp: return (struct Tone){t.note, DoubleSharp, t.octave}; break;
        default: return (struct Tone){t.note, t.shift, t.octave}; break;
    }
}

struct Tone tone_add_semitones(struct Tone t, int semitones) {
    // Keep going up until the diatonic distance is 0
    struct Tone new = t;
    while (tones_distance(t, tone_increment_diatonic(new)) <= semitones) {
        new = tone_increment_diatonic(new);
    }

    return new;
}

struct Tone note_to_tone(struct Tab *t, struct Note n) {
    return tone_add_semitones(t->info.tuning.strings[n.string], n.fret);
}

struct Measure * new_measure(struct Tab *tab, int ts_top, int ts_bottom) {
    if (tab->measures_n == tab->measures_size) {
        tab->measures = realloc(tab->measures, sizeof(struct Measure) * tab->measures_size * 2);
        tab->measures_size = tab->measures_size * 2;
    }
    struct Measure *m = &tab->measures[tab->measures_n];
    ++tab->measures_n;
    *m = (struct Measure){ts_top, ts_bottom, malloc(sizeof(struct Note)), 0, 1};
    return m;
}

int measure_get_ticks(struct Tab *t, int m) {
    return (t->measures[m].ts_top * t->ticks_per_quarter * 4) / t->measures[m].ts_bottom;
}

struct Note * measure_get_note(struct Tab *t, int _m, int string, int offset) {
    struct Measure *m = &t->measures[_m];
    struct Note *note = NULL;
    for (int i = 0; i < m->notes_n; ++i) {
        if (m->notes[i].offset == offset && m->notes[i].string == string)
            note = &m->notes[i];
    }
    return note;
}

struct Note * measure_new_note(struct Tab *t, int _m, struct Note n) {
    struct Measure *m = &t->measures[_m];
    struct Note *note = measure_get_note(t, _m, n.string, n.offset);
    if (note == NULL) {
        if (m->notes_n == m->notes_size) {
            m->notes = realloc(m->notes, sizeof(struct Note) * m->notes_size * 2);
            m->notes_size *= 2;
        }
        note = &m->notes[m->notes_n];
        ++m->notes_n;
    }
    *note = n;
    return note;
}

void measure_remove_note(struct Tab *t, int _m, struct Note *n) {
    struct Measure *m = &t->measures[_m];
    struct Note *temp_notes = malloc(sizeof(struct Note) * (m->notes_size));
    int index = n - m->notes;
    memmove(temp_notes, m->notes, index * sizeof(struct Note));
    memmove(temp_notes + index, m->notes + index + 1, (m->notes_n - index) * sizeof(struct Note));
    free(m->notes);
    m->notes = temp_notes;
    --m->notes_n;
}

struct Tab new_tab(struct Tuning tuning, int tickrate) {
    struct Tab t = {{NULL, NULL, tuning}, "", malloc(sizeof(struct Measure)), 0, 1, tickrate};
    t.measures = new_measure(&t, 4, 4);
    return t;
}

/// File Formatting ---
/// Magic Number (spells out 'TABS')
// 0x54 0x41 0x42 0x53

/// Version code
// 0x00 0x00
// (4 byte version code)

/// Title
// 0x00 0x01
// length (int)
// (c string)

/// Author
// 0x00 0x02
// length (int)
// (c string)

/// Tuning
// 0x00 0x03
/// Note (1 byte: 0x00 - 0x06 = A - G)
/// Shift (1-byte: 0x00 - 0x04 = double flat - double sharp)
/// Octave (uint8)
/// 6 times

/// Tab
// 0x00 0x10
/// # of measures (int)
// Measure
/// Time Signature - Top (int)
/// Time Signature - Bottom (int)
/// # of notes (int)
// Note
/// String (int)
/// Fret (int)
/// Length (int)

/// End of file
// 0xFF 0xFF

#define FORMAT_VERSION (uint32_t)(1)

const char magic_number[4] = {0x54, 0x41, 0x42, 0x53};
const char title_marker[4] = {0xFF, 0x00, 0x00, 0xFF};
const char author_marker[4] = {0xFF, 0x00, 0x01, 0xFF};
const char tuning_marker[4] = {0xFF, 0x00, 0x02, 0xFF};
const char tab_marker[4] = {0xFF, 0x00, 0x03, 0xFF};
const char measure_marker[4] = {0xFF, 0x00, 0x04, 0xFF};
const char note_marker[4] = {0xFF, 0x00, 0x05, 0xFF};
const char end_marker[4] = {0xFF, 0xFF, 0xFF, 0xFF};

bool compare_blocks(const char *a, const char *b) {
    for (int i = 0; i < 4; ++i) {
        if (a[i] != b[i])
            return false;
    }
    return true;
}

void process_title(struct Tab *t, FILE *f) {
    uint32_t length;
    fread(&length, sizeof(uint32_t), 1, f);
    t->info.title = malloc(sizeof(char) * length);
    fread(t->info.title, sizeof(char), length, f);
}

void process_author(struct Tab *t, FILE *f) {
    uint32_t length;
    fread(&length, sizeof(uint32_t), 1, f);
    t->info.band = malloc(sizeof(char) * length);
    fread(t->info.band, sizeof(char), length, f);
}

void process_tuning(struct Tab *t, FILE *f) {
    for (int i = 0; i < 6; ++i) {
        struct Tone *tone = &t->info.tuning.strings[i];
        uint8_t note;
        uint8_t shift;
        uint8_t octave;

        fread(&note, sizeof(uint8_t), 1, f);
        fread(&shift, sizeof(uint8_t), 1, f);
        fread(&octave, sizeof(uint8_t), 1, f);

        tone->note = note;
        tone->shift = shift;
        tone->octave = octave;
    }
}

void process_note(struct Note *n, FILE *f) {
    uint8_t string;
    uint8_t fret;
    uint32_t offset;
    uint32_t length;

    fread(&string, sizeof(uint8_t), 1, f);
    fread(&fret, sizeof(uint8_t), 1, f);
    fread(&offset, sizeof(uint32_t), 1, f);
    fread(&length, sizeof(uint32_t), 1, f);

    n->string = string;
    n->fret = fret;
    n->offset = offset;
    n->length = length;
}

void process_measure(struct Measure *m, FILE *f) {
    uint8_t ts_top;
    uint8_t ts_bottom;
    uint32_t notes_n;

    fread(&ts_top, sizeof(uint8_t), 1, f);
    fread(&ts_bottom, sizeof(uint8_t), 1, f);
    fread(&notes_n, sizeof(uint32_t), 1, f);

    m->notes = malloc(sizeof(struct Note) * notes_n);

    // Loop over blocks
    int i = 0;
    while (i < notes_n) {
        char block[4];
        uint32_t block_length;
        fread(block, sizeof(char), 4, f);
        fread(&block_length, sizeof(uint32_t), 1, f);

        if (compare_blocks(block, note_marker)) { process_note(&m->notes[i], f); ++i; }
        else {
            // The block cannot be identified. skip to the next
            fseek(f, block_length, SEEK_CUR);
        }
    }

    m->ts_top = ts_top;
    m->ts_bottom = ts_bottom;
    m->notes_n = notes_n;
    m->notes_size = notes_n;
}

void process_tab_data(struct Tab *t, FILE *f) {
    // Tab
    uint32_t tickrate;
    uint32_t measures_n;

    fread(&tickrate, sizeof(uint32_t), 1, f);
    fread(&measures_n, sizeof(uint32_t), 1, f);

    t->measures = malloc(sizeof(struct Measure) * measures_n);

    // Loop over blocks
    int i = 0;
    while (i < measures_n) {
        char block[4];
        uint32_t block_length;
        fread(block, sizeof(char), 4, f);
        fread(&block_length, sizeof(uint32_t), 1, f);

        if (compare_blocks(block, measure_marker)) { process_measure(&t->measures[i], f); ++i; }
        else {
            // The block cannot be identified. skip to the next
            fseek(f, block_length, SEEK_CUR);
        }
    }

    t->ticks_per_quarter = tickrate;
    t->measures_n = measures_n;
    t->measures_size = measures_n;
}

void open_tab(struct Tab *t, const char *file) {
    FILE *f = fopen(file, "rb");

    // Check file signature
    const char sig[] = {0x54, 0x41, 0x42, 0x53};
    char read_sig[4];
    fread(read_sig, sizeof(char), 4, f);
    if (strncmp(sig, read_sig, 4) != 0) {
        // ERROR!
    }

    // Check file version
    uint32_t version;
    fread(&version, sizeof(char), 4, f);

    // Loop over blocks
    while (1) {
        char block[4];
        uint32_t block_length;
        fread(block, sizeof(char), 4, f);
        fread(&block_length, sizeof(uint32_t), 1, f);

        if (compare_blocks(block, title_marker)) { process_title(t, f); }
        else if (compare_blocks(block, author_marker)) { process_author(t, f); }
        else if (compare_blocks(block, tuning_marker)) { process_tuning(t, f); }
        else if (compare_blocks(block, tab_marker)) { process_tab_data(t, f); }
        else if (compare_blocks(block, end_marker)) { break; }
        else {
            // The block cannot be identified. skip to the next
            fseek(f, block_length, SEEK_CUR);
        }
        /*if (BLOCK_EQUAL(block, title_code)) {
            int len;
            fread(&len, sizeof(int), 1, f);
            t->info.title = malloc(sizeof(char) * len);
            fread(t->info.title, sizeof(char), len, f);
        } else if (BLOCK_EQUAL(block, author_code)) {
            int len;
            fread(&len, sizeof(int), 1, f);
            t->info.band = malloc(sizeof(char) * len);
            fread(t->info.band, sizeof(char), len, f);
        } else if (BLOCK_EQUAL(block, end_of_file)) {
            break;
        } else if (BLOCK_EQUAL(block, tuning_code)) {
            for (int i = 0; i < 6; ++i) {
                struct Tone *tone = &t->info.tuning.strings[i];
                int note;
                fread(&note, sizeof(int), 1, f);
                int shift;
                fread(&shift, sizeof(int), 1, f);
                int octave;
                fread(&octave, sizeof(int), 1, f);
                tone->note = note;
                tone->shift = shift;
                tone->octave = octave;
            }
        } else if (BLOCK_EQUAL(block, tab_code)) {
            const char measure_code[] = {0xAA, 0xAA};
            const char note_code[] = {0xBB, 0xBB};
            fread(&t->ticks_per_quarter, sizeof(int), 1, f);

            int measures_n;
            fread(&measures_n, sizeof(size_t), 1, f);
            // Allocate room for measures
            t->measures = malloc(sizeof(struct Measure) * measures_n);
            t->measures_n = measures_n;
            t->measures_size = measures_n;
            for (int i = 0; i < t->measures_n; ++i) {
                process_measure(&t->measures[i], f);
            }
        }*/
    }

    fclose(f);
}

void save_tab(const struct Tab *tab, const char *file) {
    FILE *f = fopen(file, "wb");

    // Magic Number
    fwrite(&magic_number, sizeof(char), 4, f);

    // Version
    uint32_t version = FORMAT_VERSION;
    fwrite(&version, sizeof(uint32_t), 1, f);

    // Title
    uint32_t title_length = strlen(tab->info.title) + 1;
    uint32_t title_block_length = sizeof(uint32_t) + title_length;
    fwrite(title_marker, sizeof(char), 4, f);
    fwrite(&title_block_length, sizeof(uint32_t), 1, f);
    fwrite(&title_length, sizeof(uint32_t), 1, f);
    fwrite(tab->info.title, sizeof(char), title_length, f); 

    // Author
    uint32_t author_length = strlen(tab->info.band) + 1;
    uint32_t author_block_length = sizeof(uint32_t) + author_length;
    fwrite(author_marker, sizeof(char), 4, f);
    fwrite(&author_block_length, sizeof(uint32_t), 1, f);
    fwrite(&author_length, sizeof(uint32_t), 1, f);
    fwrite(tab->info.band, sizeof(char), author_length, f);

    // Tuning
    uint32_t tuning_length = (sizeof(uint8_t) * 3) * 6;
    fwrite(tuning_marker, sizeof(char), 4, f);
    fwrite(&tuning_length, sizeof(uint32_t), 1, f);
    for (int i = 0; i < 6; ++i) {
        struct Tone tone = tab->info.tuning.strings[i];
        uint8_t note = tone.note;
        uint8_t shift = tone.shift;
        uint8_t octave = tone.octave;

        fwrite(&note, sizeof(uint8_t), 1, f);
        fwrite(&shift, sizeof(uint8_t), 1, f);
        fwrite(&octave, sizeof(uint8_t), 1, f);
    }

    // Tab
    uint32_t tab_length;
    uint32_t tickrate = tab->ticks_per_quarter;
    uint32_t measures_n = tab->measures_n;
    fwrite(tab_marker, sizeof(char), 4, f);
    fwrite(&tab_length, sizeof(uint32_t), 1, f);
    fwrite(&tickrate, sizeof(uint32_t), 1, f);
    fwrite(&measures_n, sizeof(uint32_t), 1, f);
    for (int i = 0; i < tab->measures_n; ++i) {
        struct Measure *m = &tab->measures[i];

        uint32_t measure_length = sizeof(char) + (sizeof(uint8_t) * 2) + (sizeof(uint32_t) * 2) +
                    (m->notes_n * (sizeof(char) + (sizeof(uint8_t) * 2) + (sizeof(uint32_t) * 3)));
        uint8_t ts_top = m->ts_top;
        uint8_t ts_bottom = m->ts_bottom;
        uint32_t notes_n = m->notes_n;

        fwrite(measure_marker, sizeof(char), 4, f);
        fwrite(&measure_length, sizeof(uint32_t), 1, f);
        fwrite(&ts_top, sizeof(uint8_t), 1, f);
        fwrite(&ts_bottom, sizeof(uint8_t), 1, f);
        fwrite(&notes_n, sizeof(uint32_t), 1, f);
        for (int j = 0; j < m->notes_n; ++j) {
            struct Note *n = &m->notes[j];

            uint32_t note_length = sizeof(char) + (sizeof(uint8_t) * 2) + (sizeof(uint32_t) * 3);
            uint8_t string = n->string;
            uint8_t fret = n->fret;
            uint32_t offset = n->offset;
            uint32_t length = n->length;

            fwrite(note_marker, sizeof(char), 4, f);
            fwrite(&note_length, sizeof(uint32_t), 1, f);
            fwrite(&string, sizeof(uint8_t), 1, f);
            fwrite(&fret, sizeof(uint8_t), 1, f);
            fwrite(&offset, sizeof(uint32_t), 1, f);
            fwrite(&length, sizeof(uint32_t), 1, f);
        }
    }


    uint32_t end_length = 0;
    fwrite(end_marker, sizeof(char), 4, f);
    fwrite(&end_length, sizeof(uint32_t), 1, f);

    fclose(f);
}
