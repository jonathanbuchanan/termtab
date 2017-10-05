#include "draw.h"
#include "cmd.h"
#include "tab.h"
#include <ncurses.h>

// Processes character input (return false to exit, true to continue)
bool input(char c, struct State *s);

int main(int argc, char **argv) {
    struct Tab t = {{NULL, NULL, STANDARD_TUNING}, NULL};
    struct State s = {init_window(), &t, Normal};
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    do {
        switch (s.mode) {
        case Normal:
            
            break;
        case EditTuning:
            
            break;
        }
        draw_with_tab(s.window, &t);
    } while (input(cmd_getch(s.window), &s));
    kill_window(s.window);
    return 0;
}

bool input(char c, struct State *s) {
    switch (c) {
    case ':':
        // Prompt for a command (triggered by ':')
        return prompt(s);
    default:
        return true;
    }
}
