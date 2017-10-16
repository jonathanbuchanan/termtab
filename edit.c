#include "edit.h"

bool edit_input(struct State *s, int c) {
    switch (c) {
    case 27:
        s->mode = Command;
        break;
    default:
        break;
    }
    return true;
}
