#include "export.h"
#include "rhythm.h"
#include <hpdf.h>
#include <stdio.h>
#include <stdbool.h>

void generate_text_file(struct Tab *t, const char *file) {

}





struct Fonts {
    HPDF_Font normal;
    HPDF_Font bold;
    HPDF_Font italics;

    HPDF_Font tablature;

    HPDF_Font music;
};

enum StemDirection {
    Up,
    Down
};

struct StemInfo {
    enum StemDirection direction;
    int length;
    bool flag;
};

struct StemGroupDrawingData {
    int x;

    // The location of the tip of the stem
    int stem_x;
    int stem_y;
};

#define TITLE_SIZE 36
#define AUTHOR_SIZE 14

#define MARGIN_TOP 36
#define MARGIN_BOTTOM 36
#define MARGIN_LEFT 36
#define MARGIN_RIGHT 36

#define STAFF_EM 24
#define STAFF_SPACE (STAFF_EM / 4)
#define BARLINE_THIN (STAFF_SPACE * 0.16)
#define BARLINE_THICK (STAFF_SPACE * 0.5)
#define STAFF_LINE (STAFF_SPACE * 0.13)
#define BRACKET_EXTENSION 2

#define SPACE_PER_TICK(tab) ((STAFF_SPACE * 6) / (float)tab->ticks_per_quarter)

struct Fonts load_fonts(HPDF_Doc doc);
void pdf_display_header(HPDF_Page page, struct Fonts f, struct Tab *t);
void pdf_draw_staff(HPDF_Page page, struct Fonts f, int y);
void pdf_draw_measure(HPDF_Page page, struct Fonts f, struct Tab *t, struct Measure *m, float x, float width, int staff_y);
void pdf_draw_barline(HPDF_Page page, int x, int y1, int y2, float thickness);
struct StemGroupDrawingData pdf_draw_notegroup(HPDF_Page page, struct Fonts f, float x, float y, struct Tab *t, struct StemGroup group, struct StemInfo info);
void pdf_draw_beam(HPDF_Page page, float x1, float y1, float x2, float y2, enum StemDirection direction);
void evaluate_stems(struct Tab *t, struct RhythmData r, struct StemInfo *data);

float pdf_measure_ideal_width(struct Tab *t, struct Measure *m);

void generate_pdf(struct Tab *t, const char *file) {
    HPDF_Doc pdf;
    pdf = HPDF_New(NULL, NULL);
    HPDF_UseUTFEncodings(pdf);
    struct Fonts f = load_fonts(pdf);

    HPDF_Page pg1 = HPDF_AddPage(pdf);
    pdf_display_header(pg1, f, t);

    HPDF_REAL width;
    width = HPDF_Page_GetWidth(pg1);

    float min_space[t->measures_n];
    float ideal_space[t->measures_n];
    for (int i = 0; i < t->measures_n; ++i) {
        struct Measure *m = &t->measures[i];
        // Compute the minimum space needed for this measure
        // The minimum space required per note = 2.5 * staff space
        min_space[i] = 2.5 * STAFF_SPACE * m->notes_n;
        
        ideal_space[i] = pdf_measure_ideal_width(t, m);
    }

    int n_lines = 1;
    int line_number[t->measures_n];
    float calculated_width[t->measures_n];
    float usable_width = width - MARGIN_LEFT - MARGIN_RIGHT - 72;
    float working_width = 0;
    int leftmost_measure = 0;
    for (int i = 0; i < t->measures_n; ++i) {
        // A measure's width should never be < ideal_space unless required to fit on one line
        // The number of measures should be sufficient such that if one measure were added the width of 1+ measures would be < ideal_space

        if (working_width + ideal_space[i] > usable_width) {
            // There's no room for this measure. Add it to the next line.
            // Assess the width of all the measures of the previous line
            float scale = usable_width / working_width;
            for (int j = leftmost_measure; j < i; ++j) {
                calculated_width[j] = ideal_space[j] * scale;
            }

            ++n_lines;
            leftmost_measure = i;
            line_number[i] = n_lines - 1;
        } else {
            // Add this 
            line_number[i] = n_lines - 1;
            working_width += ideal_space[i];
        }
    }
    float scale = usable_width / working_width;
    for (int i = leftmost_measure; i < t->measures_n; ++i)
        calculated_width[i] = ideal_space[i] * scale;

    int m = 0;
    float x = MARGIN_LEFT + 72;
    for (int i = 0; i < n_lines; ++i) {
        // Draw the line
        pdf_draw_staff(pg1, f, 600 - (100 * i));
        while (line_number[m] == i) {
           pdf_draw_measure(pg1, f, t, &t->measures[m], x, calculated_width[m], 600 - (100 * i));

           x += calculated_width[m];
           ++m;
        }
        x = MARGIN_LEFT + 72;
    }

    HPDF_SaveToFile(pdf, file);
    HPDF_Free(pdf);
}

float pdf_measure_ideal_width(struct Tab *t, struct Measure *m) {
    /*float ideal = 0;
    for (int j = 0; j < m->notes_n; ++j) {
        float n_preffered = SPACE_PER_TICK(t) * m->notes[j].length;
        float n_min = 2.5 * STAFF_SPACE;
        if (n_preffered < n_min)
            ideal += n_min;
        else
            ideal += n_preffered;
    }
    return ideal;*/
    return 12 + SPACE_PER_TICK(t) * ((m->ts_top * t->ticks_per_quarter * 4) / m->ts_bottom);
}

struct Fonts load_fonts(HPDF_Doc doc) {
    struct Fonts f;

    f.normal = HPDF_GetFont(doc, "Times-Roman", "StandardEncoding");
    f.bold = HPDF_GetFont(doc, "Times-Bold", "StandardEncoding");
    f.italics = HPDF_GetFont(doc, "Times-Italic", "StandardEncoding");
    f.tablature = HPDF_GetFont(doc, "Helvetica-Bold", "StandardEncoding");
    const char *music_font = HPDF_LoadTTFontFromFile(doc, "/Users/jonathan/Library/Fonts/Bravura.ttf", HPDF_TRUE);
    f.music = HPDF_GetFont(doc, music_font, "UTF-8");

    return f;
}

void pdf_display_header(HPDF_Page page, struct Fonts f, struct Tab *t) {
    HPDF_REAL width, height;
    width = HPDF_Page_GetWidth(page);
    height = HPDF_Page_GetHeight(page);

    HPDF_Page_BeginText(page);

    HPDF_Page_SetFontAndSize(page, f.normal, TITLE_SIZE);
    HPDF_Page_TextRect(page, MARGIN_LEFT, height - MARGIN_TOP, width - MARGIN_RIGHT, height - MARGIN_TOP - 36, t->info.title, HPDF_TALIGN_CENTER, NULL);

    HPDF_Page_SetFontAndSize(page, f.normal, AUTHOR_SIZE);
    HPDF_Page_TextRect(page, MARGIN_LEFT, height - MARGIN_TOP - 36 - 15, width - MARGIN_RIGHT, height - MARGIN_TOP - 36 - 15 - 14, t->info.band, HPDF_TALIGN_RIGHT, NULL);

    HPDF_Page_EndText(page);
}


void pdf_draw_staff(HPDF_Page page, struct Fonts f, int y) {
    HPDF_REAL width = HPDF_Page_GetWidth(page);

    // Draw staff lines
    HPDF_Page_SetLineWidth(page, STAFF_LINE);
    HPDF_Page_SetLineCap(page, HPDF_BUTT_END);
    for (int i = 0; i < 6; ++i) {
        HPDF_Page_MoveTo(page, MARGIN_LEFT, y + (STAFF_SPACE * i));
        HPDF_Page_LineTo(page, width - MARGIN_RIGHT, y + (STAFF_SPACE * i));
        HPDF_Page_Stroke(page);
    }

    HPDF_Page_BeginText(page);

    // The baseline for the tab clef is the center of the staff
    HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
    HPDF_Page_MoveTextPos(page, MARGIN_LEFT + 15, y + ((STAFF_SPACE * 5) / 2));
    HPDF_Page_ShowText(page, "\xEE\x81\xAE");
    HPDF_Page_EndText(page);




    int y_treble = y + 50;
    for (int i = 0; i < 5; ++i) {
        HPDF_Page_MoveTo(page, MARGIN_LEFT, y_treble + (STAFF_SPACE * i));
        HPDF_Page_LineTo(page, width - MARGIN_RIGHT, y_treble + (STAFF_SPACE * i));
        HPDF_Page_Stroke(page);
    }
    pdf_draw_barline(page, MARGIN_LEFT, y, y_treble + STAFF_EM, BARLINE_THIN);


    HPDF_Page_BeginText(page);
    // The baseline for the G Clef is the G on the staff (2nd line)
    int g_clef_baseline = STAFF_SPACE;
    HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
    HPDF_Page_MoveTextPos(page, MARGIN_LEFT + 15, y_treble + g_clef_baseline);
    HPDF_Page_ShowText(page, "\xEE\x81\x90");
    HPDF_Page_EndText(page);

    // Draw the bracket to the left
    float bracket_height = (STAFF_SPACE * 4) + 50 + (2 * 0.25) + (2 * BRACKET_EXTENSION);
    float bracket_y = y - 0.25 - BRACKET_EXTENSION;
    HPDF_Page_SetLineWidth(page, 3);

    HPDF_Page_MoveTo(page, MARGIN_LEFT - 5, bracket_y);
    HPDF_Page_LineTo(page, MARGIN_LEFT - 5, bracket_y + bracket_height);
    HPDF_Page_Stroke(page);

    // Draw bracket ends
    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
    HPDF_Page_MoveTextPos(page, MARGIN_LEFT - 6.5, bracket_y);
    HPDF_Page_ShowText(page, "\xEE\x80\x84");
    HPDF_Page_EndText(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_MoveTextPos(page, MARGIN_LEFT - 6.5, bracket_y + bracket_height);
    HPDF_Page_ShowText(page, "\xEE\x80\x83");
    HPDF_Page_EndText(page);
}

#define STEM_THICKNESS (STAFF_SPACE * 0.12)
#define STEM_UP_BOTTOM_RIGHT_X (STAFF_SPACE * 1.18)
#define STEM_UP_BOTTOM_RIGHT_Y (STAFF_SPACE * 0.168)
#define STEM_DOWN_TOP_LEFT_X (STAFF_SPACE * 0.0)
#define STEM_DOWN_TOP_LEFT_Y (STAFF_SPACE * -0.168)

void pdf_draw_measure(HPDF_Page page, struct Fonts f, struct Tab *t, struct Measure *m, float x, float width, int y) {
    // Draw upper barline
    pdf_draw_barline(page, x + width, y + 50, y + 50 + STAFF_EM, BARLINE_THIN);

    // Draw lower barline
    pdf_draw_barline(page, x + width, y, y + (STAFF_SPACE * 5), BARLINE_THIN);

    float scale = width / pdf_measure_ideal_width(t, m);

    float offset = 0;

    struct RhythmData r = analyzeMeasure(t, m);
    for (int i = 0; i < m->notes_n; ++i) {
        struct Note *n = &m->notes[i];
        float allocated_width;
        float n_preffered = SPACE_PER_TICK(t) * n->length;
        float n_min = 2.5 * STAFF_SPACE;
        if (n_preffered < n_min)
            allocated_width = n_min * scale;
        else
            allocated_width = n_preffered * scale;

        float notehead_x = x + 12 + (SPACE_PER_TICK(t) * n->offset * scale);

        // Draw a white rectangle to hide the staff line
        HPDF_Page_SetRGBFill(page, 1, 1, 1);
        HPDF_Page_Rectangle(page, notehead_x, y + (STAFF_SPACE * (5 - n->string)) - (STAFF_SPACE / 2), 10, STAFF_SPACE);
        HPDF_Page_Fill(page);
        HPDF_Page_SetRGBFill(page, 0, 0, 0);

        // Draw the fret number
        char fret[8];
        sprintf(fret, "%d", n->fret);
        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, f.tablature, STAFF_EM / 4);
        HPDF_Page_TextRect(page, notehead_x, y + (STAFF_SPACE * (5 - n->string)) + (STAFF_SPACE / 2), notehead_x + 10, y + (STAFF_SPACE * (5 - n->string)) - (STAFF_SPACE / 2),
                fret, HPDF_TALIGN_CENTER, NULL);
        HPDF_Page_EndText(page);

        offset += allocated_width;
    }

    struct StemInfo *stem_data = malloc(r.groups_n * sizeof(struct StemInfo));
    evaluate_stems(t, r, stem_data);

    struct StemGroupDrawingData drawing_data[r.groups_n];

    for (int i = 0; i < r.groups_n; ++i) {
        struct StemGroup group = r.groups[i];

        drawing_data[i] = pdf_draw_notegroup(page, f, x + 12 + (SPACE_PER_TICK(t) * group.offset * scale), y + 50 + (STAFF_LINE / 2), t, group, stem_data[i]);
    }

    for (int i = 0; i < r.beams_n; ++i ) {
        // Draw that beam
        struct BeamGroup *beam = &r.beams[i];

        struct StemGroup *first = beam->stems[0];
        struct StemGroup *last = beam->stems[beam->stems_n - 1];

        int i_first = 0;
        while (&r.groups[i_first] != first)
            ++i_first;

        int i_last = 0;
        while (&r.groups[i_last] != last)
            ++i_last;

        struct StemGroupDrawingData data_first = drawing_data[i_first];
        struct StemGroupDrawingData data_last = drawing_data[i_last];

        pdf_draw_beam(page, data_first.stem_x, data_first.stem_y, data_last.stem_x, data_last.stem_y, stem_data[i_first].direction);
    }
}

void pdf_draw_barline(HPDF_Page page, int x, int y1, int y2, float thickness) {
    HPDF_Page_SetLineWidth(page, thickness);
    HPDF_Page_SetLineCap(page, HPDF_BUTT_END);

    HPDF_Page_MoveTo(page, x - (thickness / 2), y1);
    HPDF_Page_LineTo(page, x - (thickness / 2), y2);
    HPDF_Page_Stroke(page);
}

#define NOTEHEAD_WHOLE "\xEE\x82\xA2"
#define NOTEHEAD_HALF  "\xEE\x82\xA3"
#define NOTEHEAD_BLACK "\xEE\x82\xA4"

#define FLAG_8TH_UP    "\xEE\x89\x80"
#define FLAG_8TH_DOWN  "\xEE\x89\x81"
#define FLAG_16TH_UP   "\xEE\x89\x82"
#define FLAG_16TH_DOWN "\xEE\x89\x83"

struct StemGroupDrawingData pdf_draw_notegroup(HPDF_Page page, struct Fonts f, float x, float y, struct Tab *t, struct StemGroup group, struct StemInfo stem_data) {
    // Stem Direction (1 = up, 2 = down)
    const struct Tone e3 = {E, Natural, 3};
    const int eighth = t->ticks_per_quarter / 2;
    const int sixteenth = t->ticks_per_quarter / 4;

    const char *flag;

    struct StemGroupDrawingData data = {x, 0, 0};

    // Find origin of stem and draw it
    if (stem_data.direction == Up) {
        struct Note *origin = group_getLeast(t, &group);
        struct Tone tone = tone_add_semitones(t->info.tuning.strings[origin->string], origin->fret);
        float origin_y = y + ((STAFF_SPACE / 2) * tones_distance_diatonic(e3, tone));
        HPDF_Page_Rectangle(page, x + STEM_UP_BOTTOM_RIGHT_X - STEM_THICKNESS, origin_y + STEM_UP_BOTTOM_RIGHT_Y, STEM_THICKNESS, stem_data.length * 0.5 * STAFF_SPACE);
        HPDF_Page_Fill(page);

        data.stem_x = x + STEM_UP_BOTTOM_RIGHT_X - STEM_THICKNESS;
        data.stem_y = origin_y + STEM_UP_BOTTOM_RIGHT_Y + (stem_data.length * 0.5 * STAFF_SPACE);

        if (stem_data.flag) {
            if (origin->length == eighth)
                flag = FLAG_8TH_UP;
            else if (origin->length == sixteenth)
                flag = FLAG_16TH_UP;

            HPDF_Page_BeginText(page);
            HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
            HPDF_Page_MoveTextPos(page, x + STEM_UP_BOTTOM_RIGHT_X - STEM_THICKNESS, origin_y + STEM_UP_BOTTOM_RIGHT_Y + (stem_data.length * 0.5 * STAFF_SPACE));
            HPDF_Page_ShowText(page, flag);
            HPDF_Page_EndText(page);
        }
    } else if (stem_data.direction == Down) {
        struct Note *origin = group_getGreatest(t, &group);
        struct Tone tone = tone_add_semitones(t->info.tuning.strings[origin->string], origin->fret);
        float origin_y = y + ((STAFF_SPACE / 2) * tones_distance_diatonic(e3, note_to_tone(t, *origin)));
        HPDF_Page_Rectangle(page, x + STEM_DOWN_TOP_LEFT_X, origin_y + STEM_DOWN_TOP_LEFT_Y, STEM_THICKNESS, stem_data.length * -0.5 * STAFF_SPACE);
        HPDF_Page_Fill(page);

        data.stem_x = x + STEM_DOWN_TOP_LEFT_X + STEM_THICKNESS;
        data.stem_y = origin_y + STEM_DOWN_TOP_LEFT_Y + (stem_data.length * -0.5 * STAFF_SPACE);

        if (stem_data.flag) {
            if (origin->length == eighth)
                flag = FLAG_8TH_DOWN;
            else if (origin->length == sixteenth)
                flag = FLAG_16TH_DOWN;

            HPDF_Page_BeginText(page);
            HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
            HPDF_Page_MoveTextPos(page, x + STEM_DOWN_TOP_LEFT_X, origin_y + STEM_DOWN_TOP_LEFT_Y - (stem_data.length * 0.5 * STAFF_SPACE));
            HPDF_Page_ShowText(page, flag);
            HPDF_Page_EndText(page);
        }
    }

    // Draw the noteheads
    for (int i = 0; i < group.notes_n; ++i) {
        struct Note *n = group.notes[i].note;
        struct Tone tone = tone_add_semitones(t->info.tuning.strings[n->string], n->fret);

        const char *notehead;
        // < half note => black notehead
        if (n->length < t->ticks_per_quarter * 2)
            notehead = NOTEHEAD_BLACK;
        else if (n->length < t->ticks_per_quarter * 4)
            notehead = NOTEHEAD_HALF;
        else
            notehead = NOTEHEAD_WHOLE;

        float notehead_y = y + ((STAFF_SPACE / 2) * tones_distance_diatonic(e3, tone));

        //float notehead_x = x + (SPACE_PER_TICK(t) * n->offset * scale);

        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
        HPDF_Page_MoveTextPos(page, x, notehead_y);
        HPDF_Page_ShowText(page, notehead);
        HPDF_Page_EndText(page);
    }

    return data;
}

#define BEAM_THICKNESS (0.5 * STAFF_SPACE) 
void pdf_draw_beam(HPDF_Page page, float x1, float y1, float x2, float y2, enum StemDirection direction) {
    HPDF_Page_MoveTo(page, x1, y1);
    HPDF_Page_LineTo(page, x2, y2);

    switch (direction) {
    case Up:
        // Up (beam is below the tips of the stems)
        HPDF_Page_LineTo(page, x2, y2 - BEAM_THICKNESS);
        HPDF_Page_LineTo(page, x1, y1 - BEAM_THICKNESS);
        break;
    case Down:
        // Down (beam is above the tips of the stems)
        HPDF_Page_LineTo(page, x2, y2 + BEAM_THICKNESS);
        HPDF_Page_LineTo(page, x1, y1 + BEAM_THICKNESS);
        break;
    }

    HPDF_Page_Fill(page);
}

void evaluate_stems(struct Tab *t, struct RhythmData r, struct StemInfo *data) {
    const struct Tone b3 = {B, Natural, 3};

    // Go through beams first
    for (int b = 0; b < r.beams_n; ++b) {
        struct BeamGroup *beam = &r.beams[b];
        enum StemDirection stem_direction;
        if (beam->stems_n == 2) {
            struct Note *least_1 = group_getLeast(t, beam->stems[0]);
            struct Note *least_2 = group_getLeast(t, beam->stems[1]);

            struct Tone least_t1 = note_to_tone(t, *least_1);
            struct Tone least_t2 = note_to_tone(t, *least_2);

            struct Note *greatest_1 = group_getGreatest(t, beam->stems[0]);
            struct Note *greatest_2 = group_getGreatest(t, beam->stems[1]);

            struct Tone greatest_t1 = note_to_tone(t, *greatest_1);
            struct Tone greatest_t2 = note_to_tone(t, *greatest_2);

            int distance_low_1 = tones_distance_diatonic(b3, least_t1);
            int distance_low_2 = tones_distance_diatonic(b3, least_t2);
            int distance_high_1 = tones_distance_diatonic(b3, greatest_t1);
            int distance_high_2 = tones_distance_diatonic(b3, greatest_t2);

            int distance_low;
            int distance_high;

            if (distance_low_1 < distance_low_2)
                distance_low = distance_low_1;
            else if (distance_low_2 < distance_low_1)
                distance_low = distance_low_2;
            else
                distance_low = distance_low_1;

            if (distance_high_1 > distance_high_2)
                distance_high = distance_high_1;
            else if (distance_high_2 > distance_high_1)
                distance_high = distance_high_2;
            else
                distance_high = distance_high_1;

            if (abs(distance_low) > abs(distance_high))
                stem_direction = Up;
            else if (abs(distance_high) > abs(distance_low))
                stem_direction = Down;
            else
                stem_direction = Down;
        } else if (beam->stems_n > 2) {
            int majority = 0;
            for (int i = 0; i < beam->stems_n; ++i) {
                for (int j = 0; j < beam->stems[i]->notes_n; ++j) {
                    struct Note *n = beam->stems[i]->notes[j].note;
                    struct Tone tone = tone_add_semitones(t->info.tuning.strings[n->string], n->fret);
                    int distance = tones_distance_diatonic(b3, tone);
                    if (distance < 0)
                        majority -= 1;
                    else if (distance > 0)
                        majority += 1;
                }
            }

            if (majority > 0)
                stem_direction = Down;
            else if (majority < 0)
                stem_direction = Up;
            else if (majority == 0)
                stem_direction = Down;
        }

        // Assign the direction to each note in the beam
        for (int i = 0; i < beam->stems_n; ++i) {
            //beam->stems[i]
            int index = 0;
            while (&r.groups[index] != beam->stems[i])
                ++index;

            data[index].direction = stem_direction;
        }
    }

    // Give the group a stem if it isn't part of a beam
    for (int i = 0; i < r.groups_n; ++i) {
        if (r.groups[i].beam == NULL) {
            struct StemGroup group = r.groups[i];

            // Individual stems have flags
            data[i].flag = true;

            // Find the direction and length of the individual stem
            enum StemDirection stem_direction;
            int stem_length = 0;
            if (group.notes_n == 1) {
                struct Tone tone = tone_add_semitones(t->info.tuning.strings[group.notes[0].note->string], group.notes[0].note->fret);
                if (tones_distance_diatonic(b3, tone) < 0)
                    stem_direction = Up;
                else
                    stem_direction = Down;
            } else if (group.notes_n > 1) {
                // Get bottom-most and top-most note
                struct Note *least = group_getLeast(t, &group);
                struct Note *greatest = group_getGreatest(t, &group);

                struct Tone least_t = tone_add_semitones(t->info.tuning.strings[least->string], least->fret);
                struct Tone greatest_t = tone_add_semitones(t->info.tuning.strings[greatest->string], greatest->fret);

                int distance_low = tones_distance_diatonic(b3, least_t);
                int distance_high = tones_distance_diatonic(b3, greatest_t);

                int greatest_distance;
                if (abs(distance_low) > abs(distance_high))
                    greatest_distance = distance_low;
                else if (abs(distance_low) < abs(distance_high))
                    greatest_distance = distance_high;

                if (greatest_distance < 0)
                    stem_direction = Up;
                else if (greatest_distance > 0)
                    stem_direction = Down;
                else if (greatest_distance == 0) {
                    int majority = 0;
                    for (int i = 0; i < group.notes_n; ++i) {
                        struct Note *n = group.notes[i].note;
                        struct Tone tone = tone_add_semitones(t->info.tuning.strings[n->string], n->fret);
                        int distance = tones_distance_diatonic(b3, tone);
                        if (distance < 0)
                            majority -= 1;
                        else if (distance > 0)
                            majority += 1;
                    }

                    if (majority > 0)
                        stem_direction = Down;
                    else if (majority < 0)
                        stem_direction = Up;
                    else if (majority == 0)
                        stem_direction = Down;
                }

                
            }

            data[i].direction = stem_direction;
        } else
            data[i].flag = false;

        // Calculate stem length
        struct StemGroup group = r.groups[i];
        int stem_length = 0;
        if (group.notes_n == 1) {
            struct Tone tone = tone_add_semitones(t->info.tuning.strings[group.notes[0].note->string], group.notes[0].note->fret);
            if (abs(tones_distance_diatonic(b3, tone)) < 8)
                stem_length = 7;
            else
                stem_length = abs(tones_distance_diatonic(b3, tone));
        } else {
            // Get bottom-most and top-most note
            struct Note *least = group_getLeast(t, &group);
            struct Note *greatest = group_getGreatest(t, &group);

            struct Tone least_t = tone_add_semitones(t->info.tuning.strings[least->string], least->fret);
            struct Tone greatest_t = tone_add_semitones(t->info.tuning.strings[greatest->string], greatest->fret);

            if (abs(tones_distance_diatonic(least_t, greatest_t)) < 4)
                stem_length = 7;
            else
                stem_length = abs(tones_distance_diatonic(least_t, greatest_t)) + 4;
        }

        data[i].length = stem_length;
    }
}
