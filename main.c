#include "draw.h"
#include "cmd.h"
#include "edit.h"
#include "tab.h"
#include <ncurses.h>

int main(int argc, char **argv) {
    struct Tab t = new_tab(STANDARD_TUNING, 32);
    struct State s = {init_window(), &t, {0, 0, 0, 32}, "", Command};
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    bool running = true;
    do {
        draw(&s);
        s.msg = "";
        switch (s.mode) {
        case Command:
            running = cmd_input(&s, next_char(s.window));
            break;
        case Edit:
            running = edit_input(&s, next_char(s.window));
            break;
        }
    } while (running);
    kill_window(s.window);
    return 0;
}
