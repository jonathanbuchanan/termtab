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
    // Add a technique
    case 't':
        add_technique(s);
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

#include "log.h"

enum TechniqueClass {
    Legato,
    Slide
};

struct TechniquePrototype {
    enum TechniqueClass class;
    int n_measure;
    int n_string;
    int n_offset;
    struct Note *note;
};

void add_technique(struct State *s) {
    char buff[256];
    draw_tab_technique_prompt(s, buff);
    int tech = strtol(buff, NULL, 10);
    // 1=legato, 2=slide

    struct TechniquePrototype *proto = malloc(sizeof(struct TechniquePrototype));
    bool second_note = false;
    switch (tech) {
        case 1:
            proto->class = Legato;
            second_note = true;
            break;
        case 2:
            proto->class = Slide;
            second_note = true;
            break;
    }
    struct Note *n = measure_get_note(s->tab, s->edit.measure, s->edit.string, s->edit.x);
    proto->n_measure = s->edit.measure;
    proto->n_string = s->edit.string;
    proto->n_offset = s->edit.x;
    proto->note = n;

    // Go into select mode to choose a second argument
    // Send our incomplete technique as userdata
    if (second_note) {
        enter_select_mode(s, add_technique_callback, proto, Edit);
    } else {

    }

    
}

void add_technique_callback(struct State *s, struct Note *n, void *userdata) {
    // Add the tech!
    struct TechniquePrototype proto = *(struct TechniquePrototype *)userdata;

    struct Note *first;
    struct Note *second;
    if (proto.n_measure < s->edit.measure) {
        first = proto.note;
        second = n;
    } else if (proto.n_measure > s->edit.measure) {
        first = n;
        second = proto.note;
    } else if (proto.n_offset < s->edit.x) {
        first = proto.note;
        second = n;
    } else if (proto.n_offset > s->edit.x) {
        first = n;
        second = proto.note;
    }

    struct Technique to;
    struct Technique from;

    switch (proto.class) {
    case Legato:
        to = (struct Technique){LegatoTo, second};
        from = (struct Technique){LegatoFrom, first};
        break;
    case Slide:
        to = (struct Technique){SlideTo, second};
        from = (struct Technique){SlideFrom, first};
        break;
    }

    note_new_technique(first, to);
    note_new_technique(second, from);

    free(userdata);
}



void enter_select_mode(struct State *s, void (*callback)(struct State *s, struct Note *, void *userdata), void *userdata, int previousMode) {
    s->mode = Select;
    s->select.callback = callback;
    s->select.userdata = userdata;
    s->select.previousMode = previousMode;
}

void select_chosen(struct State *s) {
    struct Note *n = measure_get_note(s->tab, s->edit.measure, s->edit.string, s->edit.x);
    if (n != NULL) {
        (s->select.callback)(s, n, s->select.userdata);
        // Leave select mode
        s->mode = s->select.previousMode;
    }
}

bool select_input(struct State *s, int c) {
    switch (c) {
    case 'h':
        if (s->edit.x > 0)
            s->edit.x -= s->edit.cursor_width;
        break;
    case 'l':
        if (s->edit.x < measure_get_ticks(s->tab, s->edit.measure) - s->edit.cursor_width)
            s->edit.x += s->edit.cursor_width;
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
    // User has pressed <ENTER> and chosen a note
    case 10:
        select_chosen(s);
        break;
    }
    return true;
}
