#ifndef CMD_H
#define CMD_H

#include "draw.h"

// Prompts the user to enter a command
bool prompt(struct Window *window);
// Parses a command and executes the corresponding function
bool parse(const char *cmd);



// *** Below are the actual commands to be executed ***
// Opens a file for editing
#define CMD_EDIT "edit"
void edit(char *file);

#endif
