#ifndef RHYTHM_H
#define RHYTHM_H

#include "tab.h"

// Holds a pointer to the note and an integer indicating its value
struct NoteValue {
    struct Note *note;
    int value; // The inverse of the note value (e.g. 2 = half note)
};

// A rest that lasts for a certain length
struct Rest {
    int offset;
    int length;
};

struct BeamGroup;

// A grouping of notes that share a stem
struct StemGroup {
    int notes_n;
    int offset;
    struct NoteValue *notes;
    struct BeamGroup *beam;
};

// A grouping of stems that share a beam
struct BeamGroup {
    int stems_n;
    int offset;
    struct StemGroup **stems;
};

// Contains rhythm information about a measure
struct RhythmData {
    struct Measure *measure;

    int groups_n;
    struct StemGroup *groups;

    int beams_n;
    struct BeamGroup *beams;

    int rests_n;
    struct Rest *rests;
};

struct RhythmData analyzeMeasure(struct Tab *t, struct Measure *measure);
struct Note * group_getGreatest(struct Tab *t, struct StemGroup *group, struct KeySignature key);
struct Note * group_getLeast(struct Tab *t, struct StemGroup *group, struct KeySignature key);

#endif
