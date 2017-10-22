#include "edit.h"
#include "cmd.h"

bool edit_input(struct State *s, int c) {
    switch (c) {
    case 27:
        s->mode = Command;
        break;
    case 'h':

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

        break;
    case 'a':
        new_measure(s->tab, 4, 4);
        break;
    default:
        break;
    }
    return true;
}
