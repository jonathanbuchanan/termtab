#include "tab.h"
#include <stdio.h>

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

void open_tab(const char *file) {
    struct Tuning t = STANDARD_TUNING;
}
