#include "tab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

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

struct Measure * new_measure(struct Tab *tab, int ts_top, int ts_bottom) {
    if (tab->measures_n == tab->measures_size) {
        tab->measures = realloc(tab->measures, sizeof(struct Measure) * tab->measures_size * 2);
        tab->measures_size *= 2;
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
    struct Tab t = {{"", "", tuning}, "", malloc(sizeof(struct Measure)), 0, 1, tickrate};
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

#define BLOCK_EQUAL(a, b) (a[0] == b[0] && a[1] == b[1])
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
    const char version_code[] = {0x00, 0x00};
    char read_version_code[2];
    fread(read_version_code, sizeof(char), 2, f);
    if (strncmp(version_code, read_version_code, 2) != 0) {
        // ERROR!
    }

    char version[4];
    fread(version, sizeof(char), 4, f);

    // Loop over blocks
    while (1) {
        char block[2];
        fread(block, sizeof(char), 2, f);

        const char title_code[] = {0x00, 0x01};
        const char author_code[] = {0x00, 0x02};
        const char tuning_code[] = {0x00, 0x03};
        const char tab_code[] = {0x00, 0x10};
        const char end_of_file[] = {0xFF, 0xFF};

        if (BLOCK_EQUAL(block, title_code)) {
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
                struct Measure *m = &t->measures[i];                
                fread(&m->ts_top, sizeof(int), 1, f);
                fread(&m->ts_bottom, sizeof(int), 1, f);

                int notes_n;
                fread(&notes_n, sizeof(size_t), 1, f);

                m->notes = malloc(sizeof(struct Note) * notes_n);
                m->notes_n = notes_n;
                m->notes_size = notes_n;
                for (int j = 0; j < m->notes_n; ++j) {
                    struct Note *n = &m->notes[j];
                    fread(&n->string, sizeof(int), 1, f);
                    fread(&n->fret, sizeof(int), 1, f);
                    fread(&n->offset, sizeof(int), 1, f);
                    fread(&n->length, sizeof(int), 1, f);
                }
            }
        }
    }

    fclose(f);
}

void save_tab(const struct Tab *tab, const char *file) {
    FILE *f = fopen(file, "wb");

    // Write tab info (title, author, tuning, etc) first
    const char sig[] = {0x54, 0x41, 0x42, 0x53};
    fwrite(sig, sizeof(char), 4, f);

    const char version_code[] = {0x00, 0x00};
    const char version[] = {0x00, 0x00, 0x00, 0x00};
    fwrite(version_code, sizeof(char), 2, f);
    fwrite(version, sizeof(char), 4, f);

    const char title_code[] = {0x00, 0x01};
    fwrite(title_code, sizeof(char), 2, f);
    int title_len = strlen(tab->info.title) + 1;
    fwrite(&title_len, sizeof(int), 1, f);
    fwrite(tab->info.title, sizeof(char), strlen(tab->info.title) + 1, f); 

    const char author_code[] = {0x00, 0x02};
    fwrite(author_code, sizeof(char), 2, f);
    int author_len = strlen(tab->info.band) + 1;
    fwrite(&author_len, sizeof(int), 1, f);
    fwrite(tab->info.band, sizeof(char), strlen(tab->info.band) + 1, f);


    const char tuning_code[] = {0x00, 0x03};
    fwrite(tuning_code, sizeof(char), 2, f);
    for (int i = 0; i < 6; ++i) {
        struct Tone tone = tab->info.tuning.strings[i];
        // Note
        fwrite(&tone.note, sizeof(int), 1, f);
        // Shift
        fwrite(&tone.shift, sizeof(int), 1, f);
        // Octave
        fwrite(&tone.octave, sizeof(int), 1, f);
    }

    const char tab_code[] = {0x00, 0x10};
    fwrite(tab_code, sizeof(char), 2, f);
    fwrite(&tab->ticks_per_quarter, sizeof(int), 1, f);
    fwrite(&tab->measures_n, sizeof(size_t), 1, f);
    for (int i = 0; i < tab->measures_n; ++i) {
        struct Measure *m = &tab->measures[i];

        fwrite(&m->ts_top, sizeof(int), 1, f);
        fwrite(&m->ts_bottom, sizeof(int), 1, f);
        fwrite(&m->notes_n, sizeof(size_t), 1, f);
        for (int i = 0; i < m->notes_n; ++i) {
            fwrite(&m->notes[i].string, sizeof(int), 1, f);
            fwrite(&m->notes[i].fret, sizeof(int), 1, f);
            fwrite(&m->notes[i].offset, sizeof(int), 1, f);
            fwrite(&m->notes[i].length, sizeof(int), 1, f);
        }
    }


    const char end_of_file[] = {0xFF, 0xFF};
    fwrite(end_of_file, sizeof(char), 2, f);

    fclose(f);
}
