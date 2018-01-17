#include "edit.h"
#include "cmd.h"
#include "draw.h"

#include <stdlib.h>
#include <errno.h>

bool edit_input(struct State *s, int c) {
    switch (c) {
    case 27:
        s->mode = Command;
        break;
    case 'h':
        if (s->edit.x > 0)
            s->edit.x -= s->edit.cursor_width;
        break;
    case 'j':
        if (s->edit.string < 5)
            ++s->edit.string;
        break;
    case 'k':
        if (s->edit.string > 0)
            --s->edit.string;
        break;
    case 'l':
        if (s->edit.x < measure_get_ticks(s->tab, s->edit.measure) - s->edit.cursor_width)
            s->edit.x += s->edit.cursor_width;
        break;
    case 'a':
        new_measure(s->tab);
        break;
    case 'n':
        if (s->edit.measure > 0)
            --s->edit.measure;
        break;
    case 'm':
        if (s->edit.measure < s->tab->measures_n - 1)
            ++s->edit.measure;
        break;
    // Increase cursor size
    case 'g':
        s->edit.cursor_width = s->edit.cursor_width * 2;
        break;
    // Decrease cursor size
    case 'f':
        if (s->edit.cursor_width > 1)
            s->edit.cursor_width = s->edit.cursor_width / 2;
        break;
    case 'w':
        add_note(s);
        break;
    // Remove a note
    case 'x':
        remove_note(s);
        break;
    // Change the key
    case 'y':
        change_key(s);
        break;
    default:
        break;
    }
    return true;
}

void remove_note(struct State *s) {
    struct Note *n = measure_get_note(s->tab, s->edit.measure, s->edit.string, s->edit.x);
    if (n != NULL)
        measure_remove_note(s->tab, s->edit.measure, n);
}

void add_note(struct State *s) {
    char buff[256];
    draw_tab_note_prompt(s, buff);
    struct Note n = string_to_note(buff, s->edit.string, s->edit.x, s->edit.cursor_width);
    measure_new_note(s->tab, s->edit.measure, n);
}

void change_key(struct State *s) {
    char buff_key_center[256], buff_tonality[256];
    draw_tab_key_prompt(s, buff_key_center, buff_tonality);
    struct PitchClass t = string_to_pitch_class(buff_key_center);
    enum Tonality tonality;

    int tonality_n = strtol(buff_tonality, NULL, 10);
    if (errno == ERANGE || errno == EINVAL) {
        // Uh oh
    }
    if (tonality_n == 1)
        tonality = Major;
    else if (tonality_n == 2)
        tonality = Minor;
    else {
        // Uh oh
    }

    s->tab->measures[s->edit.measure].key = (struct Key){t, tonality};
}
