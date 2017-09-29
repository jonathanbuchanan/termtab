#ifndef CMD_H
#define CMD_H

#include "draw.h"
#include "tab.h"

enum Mode {
    Normal,
    EditTuning
};

struct State {
    struct Window *window;
    struct Tab *tab;
    enum Mode mode;
};

// Prompts the user to enter a command
bool prompt(struct State *s);
// Parses a command and executes the corresponding function
bool parse(struct State *s, const char *cmd);



// *** Below are the actual commands to be executed ***
// Opens a file for editing
#define CMD_EDIT "edit"
void edit(struct State *s, char *file);

// Saves a file (file is optional if already known in Tab struct)
#define CMD_SAVE "save"
void save(struct State *s, char *file);

// Sets the title of the tab
#define CMD_TITLE "title"
void title(struct State *s, char *title);

// Sets the author of the tab
#define CMD_AUTHOR "author"
void author(struct State *s, char *author);

// Begins editing the tuning
#define CMD_EDIT_TUNING "edit_tuning"
void edit_tuning();

#endif
