#include "draw.h"
#include "cmd.h"
#include "tab.h"
#include <ncurses.h>

// Processes character input (return false to exit, true to continue)
bool input(char c, struct Window *window, struct Tab *tab);

int main(int argc, char **argv) {
    struct Window *w = init_window();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    struct Tab t = {{"", "", STANDARD_TUNING}, NULL};

    do {
        draw_with_tab(w, &t);
    } while (input(cmd_getch(w), w, &t));
    kill_window(w);
    return 0;
}

bool input(char c, struct Window *window, struct Tab *tab) {
    switch (c) {
    case ':':
        // Prompt for a command (triggered by ':')
        return prompt(window, tab);
    default:
        return true;
    }
}
