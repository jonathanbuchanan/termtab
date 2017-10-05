#include "cmd.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "tab.h"

struct Command {
    char *cmd;
    char **args;
    int n_args;
};

// Executes a command
void execute_cmd(struct State *s, struct Command *cmd);

// If an argument at the index is found, it is copied into the buffer and true is returned
bool get_arg(struct Command *cmd, int n, char *buff);

// If an argument at the index is found, it is copied into the buffer as an int and true is returned
bool get_arg_int(struct Command *cmd, int n, int *buff);

// Raises an error
void cmd_error(struct State *s, int code);

#define CMD_MISSING_ARG 1
#define CMD_MISSING_ARG_MSG "The command is missing an argument."

#define CMD_PARSING_ERR 2
#define CMD_PARSING_ERR_MSG "The command could not be parsed. Check command syntax."



void init_and_cpy_range(char **buffer, const char *start, const char *end) {
    *buffer = malloc(end - start + 1);
    strncpy(*buffer, start, end - start);
    (*buffer)[end - start] = '\0';
}

// Tokenizes a string. Returns true unless an error was found
bool tokenize(const char *string, struct Command *command) {
    int c = 0;
    const char *i = string;
    const char *token_start = string;
    enum {None, Word, Quote} state = None;

    for (i = string; *i != '\0'; ++i) {
        switch (state) {
        case None:
            if (!isspace(*i)) {
                if (*i == '"') {
                    state = Quote;
                    token_start = i + 1;
                } else {
                    state = Word;
                    token_start = i;
                }
            }
            break;
        case Word:
            if (isspace(*i)) {
                if (c == 0)
                    init_and_cpy_range(&command->cmd, token_start, i);
                else
                    init_and_cpy_range(&command->args[c - 1], token_start, i);
                state = None;
                ++c;
            }
            break;
        case Quote:
            if (*i == '"') {
                if (c == 0)
                    init_and_cpy_range(&command->cmd, token_start, i);
                else
                    init_and_cpy_range(&command->args[c - 1], token_start, i);
                state = None;
                ++c;
            }
            break;
        }
    }
    if (state == Word) {
        if (c == 0)
            init_and_cpy_range(&command->cmd, token_start, i);
        else
            init_and_cpy_range(&command->args[c - 1], token_start, i);
        ++c;
    } else if (state == Quote) {
        // Unmatched quotes, raise error
        return false;
    }
    if (c < 1) {
        // No tokens found, raise error
        return false;
    }
    command->n_args = c - 1;

    return true;
}

bool prompt(struct State *s) {
    // Prompt the user for a command
    char buffer[256];
    draw_cmd_window_prompt(s->window, buffer);

    // Parse the entered command
    bool close = parse(s, buffer);
    
    draw_cmd_window_blank(s->window);

    return close;
}

bool parse(struct State *s, const char *str) {
    struct Command cmd = {};

    char string[256];
    strcpy(string, str);    

    cmd.args = calloc(16, sizeof(char *));
    if (tokenize(string, &cmd) == false) {
        // Tokenization resulted in error
        cmd_error(s, CMD_PARSING_ERR);
        return true;
    }

    if (strcmp(cmd.cmd, "q") == 0) {
        free(cmd.args);
        return false;
    }

    execute_cmd(s, &cmd);
    free(cmd.args);
    return true;
}

void execute_cmd(struct State *s, struct Command *cmd) {
    if (strcmp(cmd->cmd, CMD_EDIT) == 0) {
        char arg[256];
        if (get_arg(cmd, 0, arg) == false) {
            cmd_error(s, CMD_MISSING_ARG);
            return;
        }
        edit(s, arg);
    } else if (strcmp(cmd->cmd, CMD_SAVE) == 0) {
        char arg[256];
        if (get_arg(cmd, 0, arg) == false) {
            cmd_error(s, CMD_MISSING_ARG);
            return;
        }
        save(s, arg);
    } else if (strcmp(cmd->cmd, CMD_TITLE) == 0) {
        char arg[256];
        if (get_arg(cmd, 0, arg) == false) {
            cmd_error(s, CMD_MISSING_ARG);
            return;
        }
        title(s, arg);
    } else if (strcmp(cmd->cmd, CMD_AUTHOR) == 0) {
        char arg[256];
        if (get_arg(cmd, 0, arg) == false) {
            cmd_error(s, CMD_MISSING_ARG);
            return;
        }
        author(s, arg);
    } else if (strcmp(cmd->cmd, CMD_SET_STRING) == 0) {
        int string;
        char arg[256];
        if (get_arg_int(cmd, 0, &string) == false) {
            cmd_error(s, CMD_MISSING_ARG);
            return;
        }
        if (get_arg(cmd, 1, arg) == false) {
            cmd_error(s, CMD_MISSING_ARG);
            return;
        }
        set_string(s, string, arg);
    }
    // If no error, clear the status bar
    draw_status_window_clear(s->window);
}

bool get_arg(struct Command *cmd, int n, char *buffer) {
    if (n + 1 > cmd->n_args)
        return false;
    strcpy(buffer, cmd->args[n]);
    return true;
}

bool get_arg_int(struct Command *cmd, int n, int *buffer) {
    if (n + 1 > cmd->n_args)
        return false;
    long result = strtol(cmd->args[n], NULL, 10);
    if (errno == ERANGE || errno == EINVAL) {
        return false;
    }
    *buffer = (int)result;
    return true;
}

void cmd_error(struct State *s, int code) {
    switch (code) {
        case CMD_MISSING_ARG:
            draw_status_window_msg(s->window, CMD_MISSING_ARG_MSG);
            break;
        case CMD_PARSING_ERR:
            draw_status_window_msg(s->window, CMD_PARSING_ERR_MSG);
            break;
    }
}



void edit(struct State *s, char *file) {
    open_tab(s->tab, file);
}

void save(struct State *s, char *file) {
    save_tab(s->tab, file);
}

void title(struct State *s, char *title) {
    char *new_title = realloc(s->tab->info.title, strlen(title) + 1);
    if (new_title != NULL) {
        s->tab->info.title = new_title;
        strcpy(s->tab->info.title, title);
    } else {
        // ERROR
    }
}

void author(struct State *s, char *author) {
    char *new_author = realloc(s->tab->info.band, strlen(author) + 1);
    if (new_author != NULL) {
        s->tab->info.band = new_author;
        strcpy(s->tab->info.band, author);
    } else {
        // ERROR
    }
}

void set_string(struct State *s, int string, char *tone) {
    struct Tone t = string_to_tone(tone);
    if (string < 6)
        s->tab->info.tuning.strings[string] = t;
    else {
        // ERROR
    }
}
