#include "rhythm.h"

#include <stdlib.h>
#include <string.h>

int compare_groups(const void *group1, const void *group2) {
    struct StemGroup *a = (struct StemGroup *)group1;
    struct StemGroup *b = (struct StemGroup *)group2;

    if (a->offset < b->offset) return -1;
    if (a->offset == b->offset) return 0;
    if (a->offset > b->offset) return 1;

    return 0;
}

struct RhythmData analyzeMeasure(struct Tab *t, struct Measure *m) {
    struct RhythmData rhythm = {m, 0, NULL, 0, NULL};

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
        rhythm.groups[i].beam = NULL;
    }

    int *group_indices = malloc(sizeof(int) * groups_n);
    memset(group_indices, 0, groups_n);
    for (int i = 0; i < m->notes_n; ++i) {
        rhythm.groups[group_number[i]].notes[group_indices[group_number[i]]] = (struct NoteValue){&m->notes[i], m->notes[i].length};
        rhythm.groups[group_number[i]].offset = m->notes[i].offset;
        ++group_indices[group_number[i]];
    }

    // Sort the groups by offset
    qsort(rhythm.groups, rhythm.groups_n, sizeof(struct StemGroup), compare_groups);

    int g = 0;
    int beams = 0;

    // TODO: Intelligently size array. Don't guess!
    rhythm.beams = malloc(sizeof(struct BeamGroup) * 100);    

    for (int i = 0; i < m->ts_top; ++i) {
        while (rhythm.groups[g].offset < (i + 1) * ((t->ticks_per_quarter * 4) / m->ts_bottom)) {
            int stems_n = 0;
            // TODO: Check for extreme cases (1 note, etc.)

            // Continue adding notes to the beam as long as it is unbroken
            struct BeamGroup beam;
            while (rhythm.groups[g + stems_n].offset < (i + 1) * ((t->ticks_per_quarter * 4) / m->ts_bottom)) {
                if (g + stems_n + 1 >= rhythm.groups_n) {
                    ++stems_n;
                    break;
                }

                if (rhythm.groups[g + stems_n].offset + rhythm.groups[g + stems_n].notes[0].value == rhythm.groups[g + stems_n + 1].offset) {
                    ++stems_n;
                } else {
                    ++g;
                    break;
                }
            }

            // Denegerate group (Only 1 stem)? Discard.
            if (stems_n < 2) {
                g += stems_n;
                continue;
            }

            // Add the stems to the beam
            beam.stems_n = stems_n;
            beam.stems = malloc(sizeof(struct StemGroup) * stems_n);
            beam.offset = rhythm.groups[g].offset;
            for (int j = 0; j < stems_n; ++j) {
                beam.stems[j] = &rhythm.groups[g + j];
                rhythm.groups[g + j].beam = &beam;
            }

            rhythm.beams[beams] = beam;

            g += stems_n;
            ++beams;

            if (g >= rhythm.groups_n)
                break;
        }
    }

    rhythm.beams_n = beams;

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
