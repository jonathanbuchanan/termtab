#ifndef CMD_H
#define CMD_H

#include "tab.h"
#include "edit.h"

#include <ncurses.h>

struct Window;

enum Mode {
    Command,
    Edit,
    Select
};

struct State {
    struct Window *window;
    struct Tab *tab;
    struct EditingState edit;
    struct SelectState select;
    char *msg;
    enum Mode mode;
};

// Handles user input
bool cmd_input(struct State *s, char c);
// Prompts the user to enter a command
bool prompt(struct State *s);
// Parses a command and executes the corresponding function
bool parse(struct State *s, const char *cmd);



// *** Below are the actual commands to be executed ***
// Opens a file for editing
#define CMD_EDIT "edit"
void edit(struct State *s, char *file);

// Enters edit mode
#define CMD_EDIT_MODE "edit_mode"
void edit_mode(struct State *s);

// Saves a file (file is optional if already known in Tab struct)
#define CMD_SAVE "save"
void save(struct State *s, char *file);

// Sets the title of the tab
#define CMD_TITLE "title"
void title(struct State *s, char *title);

// Sets the author of the tab
#define CMD_AUTHOR "author"
void author(struct State *s, char *author);

// Sets the tone of a string
#define CMD_SET_STRING "set_string"
void set_string(struct State *s, int string, char *tone);

// Exports the tab to a plain text file
#define CMD_EXPORT_TXT "export_txt"
void export_txt(struct State *s, char *file);

// Exports the tab to a PDF
#define CMD_EXPORT_PDF "export_pdf"
void export_pdf(struct State *s, char *file);

#endif
