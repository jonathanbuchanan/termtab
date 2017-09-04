#include "cmd.h"
#include <stdlib.h>
#include <string.h>

#include "tab.h"

void execute_cmd(char *cmd, char **args);

bool prompt(struct Window *window) {
    // Prompt the user for a command
    char buffer[256];
    draw_cmd_window_prompt(window, buffer);

    // Parse the entered command
    bool close = parse(buffer);
    
    draw_cmd_window_blank(window);

    return close;
}

bool parse(const char *str) {
    char *string = malloc(strlen(str) + 1);
    strcpy(string, str);    

    // Count the tokens in the command
    int token_count = 1;
    strtok(string, " \t");
    while (strtok(NULL, " \t") != NULL)
        token_count += 1;

    // Separate the string into tokens by ' '
    strcpy(string, str);
    char *cmd = strtok(string, " \t");
    char **args = calloc(token_count, sizeof(char *));
    for (int i = 0; i < token_count; ++i)
        args[i] = strtok(NULL, " \t");

    if (strcmp(cmd, "q") == 0)
        return false;
    execute_cmd(cmd, args);
    return true;
}

void execute_cmd(char *cmd, char **args) {
    if (strcmp(cmd, CMD_EDIT) == 0)
        edit(args[0]);
}



void edit(char *file) {
    open_tab(file);
}
