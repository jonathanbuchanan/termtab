#include "rhythm.h"

#include <stdlib.h>
#include <string.h>

struct RhythmData analyzeMeasure(struct Measure *m) {
    struct RhythmData rhythm;

    // Group the notes up if they occur at the same time (chords, etc.)
    int group_number[m->notes_n];
    int groups_n = 0;

    for (int i = 0; i < m->notes_n; ++i) {
        int equal = -1;
        for (int j = 0; j < i; ++j) {
            if (m->notes[i].offset == m->notes[j].offset)
                equal = j;
        }
        if (equal == -1) {
            group_number[i] = groups_n;
            ++groups_n;
        } else
            group_number[i] = group_number[equal];
    }

    int *group_size = malloc(sizeof(int) * groups_n);
    memset(group_size, 0, groups_n);
    for (int i = 0; i < m->notes_n; ++i) {
        group_size[group_number[i]] += 1;
    }

    rhythm.groups_n = groups_n;
    rhythm.groups = malloc(sizeof(struct StemGroup) * groups_n);

    for (int i = 0; i < groups_n; ++i) {
        rhythm.groups[i].notes_n = group_size[i];
        rhythm.groups[i].notes = malloc(sizeof(struct NoteValue) * group_size[i]);
    }

    int *group_indices = malloc(sizeof(int) * groups_n);
    memset(group_indices, 0, groups_n);
    for (int i = 0; i < m->notes_n; ++i) {
        rhythm.groups[group_number[i]].notes[group_indices[group_number[i]]] = (struct NoteValue){&m->notes[i], 0};
        rhythm.groups[group_number[i]].offset = m->notes[i].offset;
        ++group_indices[group_number[i]];
    }

    return rhythm;
}

struct Note * group_getGreatest(struct Tab *t, struct StemGroup *group) {
    struct Note *greatest = group->notes[0].note;
    for (int i = 0; i < group->notes_n; ++i) {
        if (tones_distance(note_to_tone(t, *greatest), note_to_tone(t, *group->notes[i].note)) > 0)
            greatest = group->notes[i].note;
    }
    return greatest;
}

struct Note * group_getLeast(struct Tab *t, struct StemGroup *group) {
    struct Note *least = group->notes[0].note;
    for (int i = 0; i < group->notes_n; ++i) {
        if (tones_distance(note_to_tone(t, *least), note_to_tone(t, *group->notes[i].note)) < 0)
            least = group->notes[i].note;
    }
    return least;
}
