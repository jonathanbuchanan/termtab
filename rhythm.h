#ifndef RHYTHM_H
#define RHYTHM_H

#include "tab.h"

// Holds a pointer to the note and an integer indicating its value
struct NoteValue {
    struct Note *note;
    int value; // The inverse of the note value (e.g. 2 = half note)
};

// A grouping of notes that share a stem
struct StemGroup {
    int notes_n;
    int offset;
    struct NoteValue *notes;
};

// Contains rhythm information about a measure
struct RhythmData {
    struct Measure *measure;

    int groups_n;
    struct StemGroup *groups;
};

struct RhythmData analyzeMeasure(struct Measure *measure);
struct Note * group_getGreatest(struct Tab *t, struct StemGroup *group);
struct Note * group_getLeast(struct Tab *t, struct StemGroup *group);

#endif
