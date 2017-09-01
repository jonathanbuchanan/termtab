#include "cmd.h"

bool parse(char *cmd);

bool prompt(struct Window *window) {
    // Prompt the user for a command
    curs_set(1);
    echo();
    mvwaddch(window->cmd, 1, 0, ':');
    wrefresh(window->cmd);
    char buffer[256];
    mvwgetnstr(window->cmd, 1, 1, buffer, 256);
    curs_set(0);
    noecho();

    // Parse the entered command
    return parse(buffer);
}

bool parse(char *cmd) {
    if (strcmp(cmd, "q") == 0)
        return false;
    return true;
}
