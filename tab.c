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

struct Measure * new_measure(struct Tab *tab, int ts_top, int ts_bottom) {
    if (tab->measures_n == tab->measures_size) {
        tab->measures = realloc(tab->measures, sizeof(struct Measure) * tab->measures_size * 2);
        tab->measures_size *= 2;
    }
    struct Measure *m = &tab->measures[tab->measures_n];
    ++tab->measures_n;
    *m = (struct Measure){ts_top, ts_bottom, NULL, 0};
    return m;
}

struct Tab new_tab(struct Tuning tuning) {
    struct Tab t = {{"", "", tuning}, "", malloc(sizeof(struct Measure)), 0, 1};
    t.measures = new_measure(&t, 4, 4);
    return t;
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
        const char end_of_file[] = {0xFF, 0xFF};

        if (block[0] == title_code[0] && block[1] == title_code[1]) {
            int len;
            fread(&len, sizeof(int), 1, f);
            t->info.title = malloc(sizeof(char) * len);
            fread(t->info.title, sizeof(char), len, f);
        } else if (block[0] == author_code[0] && block[1] == author_code[1]) {
            int len;
            fread(&len, sizeof(int), 1, f);
            t->info.band = malloc(sizeof(char) * len);
            fread(t->info.band, sizeof(char), len, f);
        } else if (block[0] == end_of_file[0] && block[1] == end_of_file[1]) {
            break;
        } else if (block[0] == tuning_code[0] && block[1] == tuning_code[1]) {
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
        }
    }

    fclose(f);
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
// Measure
/// 0xAA 0xAA
/// Time Signature - Top (int)
/// Time Signature - Bottom (int)
/// # of notes (int)
// Note
/// 0xBB 0xBB
/// String (int)
/// Fret (int)
/// Length (int)

/// End of file
// 0xFF 0xFF

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
    const char measure_code[] = {0xAA, 0xAA};
    const char note_code[] = {0xBB, 0xBB};
    for (int i = 0; i < tab->measures_n; ++i) {
        struct Measure *m = &tab->measures[i];
        fwrite(measure_code, sizeof(char), 2, f);

        fwrite(&m->ts_top, sizeof(int), 1, f);
        fwrite(&m->ts_bottom, sizeof(int), 1, f);
        fwrite(&m->notes_n, sizeof(size_t), 1, f);
        for (int i = 0; i < m->notes_n; ++i) {
            fwrite(note_code, sizeof(char), 2, f);

            fwrite(&m->notes[i].string, sizeof(int), 1, f);
            fwrite(&m->notes[i].fret, sizeof(int), 1, f);
            fwrite(&m->notes[i].length, sizeof(int), 1, f);
        }
    }


    const char end_of_file[] = {0xFF, 0xFF};
    fwrite(end_of_file, sizeof(char), 2, f);

    fclose(f);
}
