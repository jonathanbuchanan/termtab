#ifndef CMD_H
#define CMD_H

#include "draw.h"

// Prompts the user to enter a command
bool prompt(struct Window *window, struct Tab *tab);
// Parses a command and executes the corresponding function
bool parse(struct Window *window, struct Tab *tab, const char *cmd);



// *** Below are the actual commands to be executed ***
// Opens a file for editing
#define CMD_EDIT "edit"
void edit(char *file);

// Sets the title of the tab
#define CMD_TITLE "title"
void title(struct Window *window, struct Tab *tab, char *title);

// Sets the author of the tab
#define CMD_AUTHOR "author"
void author(struct Window *window, struct Tab *tab, char *author);

#endif
