#include "cmd.h"
#include <stdlib.h>
#include <string.h>

#include "tab.h"

struct Command {
    char *cmd;
    char **args;
    int n_args;
};

// Executes a command
void execute_cmd(struct Window *window, struct Tab *tab, struct Command *cmd);

// If an argument at the index is found, it is copied into the buffer and true is returned
bool get_arg(struct Command *cmd, int n, char *buff);

// Raises an error
void cmd_error(struct Window *window, int code);

#define CMD_MISSING_ARG 1
#define CMD_MISSING_ARG_MSG "The command is missing an argument"



bool prompt(struct Window *window, struct Tab *tab) {
    // Prompt the user for a command
    char buffer[256];
    draw_cmd_window_prompt(window, buffer);

    // Parse the entered command
    bool close = parse(window, tab, buffer);
    
    draw_cmd_window_blank(window);

    return close;
}

bool parse(struct Window *window, struct Tab *tab, const char *str) {
    struct Command cmd = {};

    char string[256];
    strcpy(string, str);    

    // Count the tokens in the command
    int token_count = 0;
    if (strtok(string, " \t") != NULL)
        token_count += 1;
    while (strtok(NULL, " \t") != NULL)
        token_count += 1;
    if (token_count < 1)
        return true;

    // Separate the string into tokens by ' '
    strcpy(string, str);
    cmd.cmd = strtok(string, " \t");
    cmd.args = calloc(token_count, sizeof(char *));
    for (int i = 0; i < token_count; ++i)
        cmd.args[i] = strtok(NULL, " \t");
    cmd.n_args = token_count - 1;

    if (strcmp(cmd.cmd, "q") == 0) {
        free(cmd.args);
        return false;
    }

    execute_cmd(window, tab, &cmd);
    free(cmd.args);
    return true;
}

void execute_cmd(struct Window *window, struct Tab *tab, struct Command *cmd) {
    if (strcmp(cmd->cmd, CMD_EDIT) == 0) {
        char arg[256];
        if (get_arg(cmd, 0, arg) == false) {
            cmd_error(window, CMD_MISSING_ARG);
            return;
        }
        edit(arg);
    } else if (strcmp(cmd->cmd, CMD_TITLE) == 0) {
        char arg[256];
        if (get_arg(cmd, 0, arg) == false) {
            cmd_error(window, CMD_MISSING_ARG);
            return;
        }
        title(window, tab, arg);
    } else if (strcmp(cmd->cmd, CMD_AUTHOR) == 0) {
        char arg[256];
        if (get_arg(cmd, 0, arg) == false) {
            cmd_error(window, CMD_MISSING_ARG);
            return;
        }
        author(window, tab, arg);
    }
    // If no error, clear the status bar
    draw_status_window_clear(window);
}

bool get_arg(struct Command *cmd, int n, char *buffer) {
    if (n + 1 > cmd->n_args)
        return false;
    strcpy(buffer, cmd->args[n]);
    return true;
}

void cmd_error(struct Window *window, int code) {
    switch (code) {
        case CMD_MISSING_ARG:
            draw_status_window_msg(window, CMD_MISSING_ARG_MSG);
            break;
    }
}



void edit(char *file) {
    open_tab(file);
}

void title(struct Window *window, struct Tab *tab, char *title) {
    char *new_title = realloc(tab->info.title, strlen(title) + 1);
    if (new_title != NULL) {
        tab->info.title = new_title;
        strcpy(tab->info.title, title);
    } else {
        // ERROR
    }
}

void author(struct Window *window, struct Tab *tab, char *author) {
    char *new_author = realloc(tab->info.band, strlen(author) + 1);
    if (new_author != NULL) {
        tab->info.band = new_author;
        strcpy(tab->info.band, author);
    } else {
        // ERROR
    }
}
