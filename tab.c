#include "tab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

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
// length (int) (includes the \0)
// (c string)

/// Author
// 0x00 0x02
// (c string)

/// Tuning
// 0x00 0x03
/// Note (1 byte: 0x00 - 0x06 = A - G)
/// Shift (1-byte: 0x00 - 0x04 = double flat - double sharp)
/// Octave (uint8)
/// 6 times

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

    const char end_of_file[] = {0xFF, 0xFF};
    fwrite(end_of_file, sizeof(char), 2, f);

    fclose(f);
}
