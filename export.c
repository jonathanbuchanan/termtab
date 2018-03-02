#include "export.h"
#include "rhythm.h"
#include "log.h"
#include <hpdf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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
#define TIME_SIGNATURE_WIDTH (STAFF_SPACE * 1.8)

#define SPACE_PER_TICK(tab) ((STAFF_SPACE * 4) / (float)tab->ticks_per_quarter)

struct Fonts load_fonts(HPDF_Doc doc);
void pdf_display_header(HPDF_Page page, struct Fonts f, struct Tab *t);



void pdf_draw_staff(HPDF_Page page, struct Fonts f, int y);
void pdf_draw_time_signature(HPDF_Page page, struct Fonts f, float x, float y, int ts_top, int ts_bottom);
void pdf_draw_key_signature(HPDF_Page page, struct Fonts f, float x, float y, struct Key key);



void pdf_draw_measure(HPDF_Page page, struct Fonts f, struct Tab *t, int measure_number, float x, float width, int staff_y, bool time_signature, bool key_signature);
void pdf_draw_barline(HPDF_Page page, int x, int y1, int y2, float thickness);
struct StemGroupDrawingData pdf_draw_notegroup(HPDF_Page page, struct Fonts f, float x, float y, struct Tab *t, struct StemGroup group, struct StemInfo info, struct PitchClass *key);
void pdf_draw_beam(HPDF_Page page, float x1, float y1, float x2, float y2, enum StemDirection direction);
void pdf_draw_rest(HPDF_Page page, struct Fonts f, float x, float y, struct Tab *t, int value);
void evaluate_stems(struct Tab *t, struct RhythmData r, struct StemInfo *data, struct PitchClass *key);

struct MeasureWidthBreakdown {
    float scalable_width;
    float non_scalable_width;
};
struct MeasureWidthBreakdown pdf_measure_ideal_width(struct Tab *t, int m, bool time_signature, bool key_signature);
int key_signature_width(struct Key key);

bool measure_should_show_time_signature(struct Tab *t, int m) {
    if (m == 0)
        return true;
    else if (t->measures[m].ts_top != t->measures[m - 1].ts_top || t->measures[m].ts_bottom != t->measures[m - 1].ts_bottom)
        return true;
    return false;
}

bool measure_should_show_key_signature(struct Tab *t, int m, int line_index) {
    if (line_index == 0)
        return true;
    else if (!keys_equal(t->measures[m].key, t->measures[m - 1].key))
        return true;
    return false;
}

void generate_pdf(struct Tab *t, const char *file) {
    HPDF_Doc pdf;
    pdf = HPDF_New(NULL, NULL);
    HPDF_UseUTFEncodings(pdf);
    struct Fonts f = load_fonts(pdf);

    HPDF_Page pg1 = HPDF_AddPage(pdf);
    pdf_display_header(pg1, f, t);

    HPDF_REAL width;
    width = HPDF_Page_GetWidth(pg1);

    int n_lines = 1;
    int line_number[t->measures_n];
    struct MeasureWidthBreakdown ideal_width[t->measures_n];
    float calculated_width[t->measures_n];
    float usable_width = width - MARGIN_LEFT - MARGIN_RIGHT - 36;

    struct MeasureWidthBreakdown working_width;

    int leftmost_measure = 0;
    for (int i = 0; i < t->measures_n; ++i) {
        // A measure's width should never be < ideal_space unless required to fit on one line
        // The number of measures should be sufficient such that if one measure were added the width of 1+ measures would be < ideal_space
        bool time_sig = measure_should_show_time_signature(t, i);
        bool key_sig = measure_should_show_key_signature(t, i, i - leftmost_measure);

        struct MeasureWidthBreakdown ideal = pdf_measure_ideal_width(t, i, time_sig, key_sig);

        if (working_width.non_scalable_width + working_width.scalable_width +
                ideal.scalable_width + ideal.non_scalable_width > usable_width) {
            // There's no room for this measure. Add it to the next line.
            // Assess the width of all the measures of the previous line
            // non_scalable_width + (scale * scalable_width) = usable_width
            // scale = (usable_width - non_scalable_width) / scalable_width
            float scale = (usable_width - working_width.non_scalable_width) / working_width.scalable_width;
            for (int j = leftmost_measure; j < i; ++j) {
                calculated_width[j] = (ideal_width[j].scalable_width * scale) + ideal_width[j].non_scalable_width;
            }

            ++n_lines;


            leftmost_measure = i;
            line_number[i] = n_lines - 1;

            // We need to recalculate width here because the first measure of a line always has a key signature
            ideal = pdf_measure_ideal_width(t, i, time_sig, true);

            working_width.non_scalable_width = ideal.non_scalable_width;
            working_width.scalable_width = ideal.scalable_width;
        } else {
            // Add this 
            line_number[i] = n_lines - 1;

            working_width.non_scalable_width += ideal.non_scalable_width;
            working_width.scalable_width += ideal.scalable_width;
        }

        ideal_width[i] = ideal;
    }
    float scale = (usable_width - working_width.non_scalable_width) / working_width.scalable_width;
    for (int i = leftmost_measure; i < t->measures_n; ++i)
        calculated_width[i] = (ideal_width[i].scalable_width * scale) + ideal_width[i].non_scalable_width;

    int m = 0;
    float x = MARGIN_LEFT + 36;
    for (int i = 0; i < n_lines; ++i) {
        int line_index = 0;

        // Draw the lines
        pdf_draw_staff(pg1, f, 600 - (100 * i));
        while (line_number[m] == i) {
            bool time_sig = measure_should_show_time_signature(t, m);
            bool key_sig = measure_should_show_key_signature(t, m, line_index);

            pdf_draw_measure(pg1, f, t, m, x, calculated_width[m], 600 - (100 * i), time_sig, key_sig);

            x += calculated_width[m];
            ++m;
            ++line_index;
        }
        x = MARGIN_LEFT + 36;
    }

    HPDF_SaveToFile(pdf, file);
    HPDF_Free(pdf);
}

struct MeasureWidthBreakdown pdf_measure_ideal_width(struct Tab *t, int measure_number, bool time_signature, bool key_signature) {
    struct MeasureWidthBreakdown breakdown;

    struct Measure *m = &t->measures[measure_number];
    breakdown.scalable_width = 12 + SPACE_PER_TICK(t) * ((m->ts_top * t->ticks_per_quarter * 4) / m->ts_bottom);
    if (time_signature)
        breakdown.non_scalable_width += TIME_SIGNATURE_WIDTH;
    if (key_signature)
        breakdown.non_scalable_width += key_signature_width(m->key);

    return breakdown;
}

int key_signature_width(struct Key key) {
    struct PitchClass sig[7];
    get_key_signature(key, sig);

    int symbols = 0;
    for (int i = 0; i < 7; ++i)
        if (sig[i].shift != Natural)
            ++symbols;

    return 10 * symbols;
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

#define TS_1 "\xEE\x82\x81"
#define TS_2 "\xEE\x82\x82"
#define TS_3 "\xEE\x82\x83"
#define TS_4 "\xEE\x82\x84"
#define TS_5 "\xEE\x82\x85"
#define TS_6 "\xEE\x82\x86"
#define TS_7 "\xEE\x82\x87"
#define TS_8 "\xEE\x82\x88"
#define TS_9 "\xEE\x82\x89"

void get_ts_char(int number, char **buff) {
    switch (number) {
        case 1: *buff = TS_1; break;
        case 2: *buff = TS_2; break;
        case 3: *buff = TS_3; break;
        case 4: *buff = TS_4; break;
        case 5: *buff = TS_5; break;
        case 6: *buff = TS_6; break;
        case 7: *buff = TS_7; break;
        case 8: *buff = TS_8; break;
        case 9: *buff = TS_9; break;
    }
}

void pdf_draw_time_signature(HPDF_Page page, struct Fonts f, float x, float y, int ts_top, int ts_bottom) {
    char *top_char;
    char *bottom_char;

    get_ts_char(ts_top, &top_char);
    get_ts_char(ts_bottom, &bottom_char);

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
    // Center on 1st staff line
    HPDF_Page_MoveTextPos(page, x, y + STAFF_SPACE);
    HPDF_Page_ShowText(page, bottom_char);
    HPDF_Page_EndText(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
    // Center on 4th staff line
    HPDF_Page_MoveTextPos(page, x, y + (STAFF_SPACE * 3));
    HPDF_Page_ShowText(page, top_char);
    HPDF_Page_EndText(page);
}

#define FLAT         "\xEE\x89\xA0"
#define NATURAL      "\xEE\x89\xA1"
#define SHARP        "\xEE\x89\xA2"
#define DOUBLE_SHARP "\xEE\x89\xA3"
#define DOUBLE_FLAT  "\xEE\x89\xA4"
void pdf_draw_key_signature(HPDF_Page page, struct Fonts f, float x, float y, struct Key key) {
    struct PitchClass classes[7];
    get_key_signature(key, classes);

    float advance = x;
    for (int i = 0; i < 7; ++i) {
        // Sharps and flats must be above the G on the second line
        int spaces;
        switch (classes[i].pitch) {
            case A: spaces = 0; break;
            case B: spaces = 1; break;
            case C: spaces = 2; break;
            case D: spaces = 3; break;
            case E: spaces = 4; break;
            case F: spaces = 5; break;
            case G: spaces = 6; break;
        }

        if (classes[i].shift == Natural)
            continue;

        char *modifier;
        switch (classes[i].shift) {
            case DoubleFlat: modifier = DOUBLE_FLAT; break;
            case Flat: modifier = FLAT; break;
            case Sharp: modifier = SHARP; break;
            case DoubleSharp: modifier = DOUBLE_SHARP; break;
            case Natural: modifier = ""; break;
        }

        HPDF_Page_BeginText(page);
        HPDF_Page_SetFontAndSize(page, f.music, STAFF_EM);
        HPDF_Page_MoveTextPos(page, advance, y + ((STAFF_SPACE / 2) * (spaces + 3)));
        HPDF_Page_ShowText(page, modifier);
        HPDF_Page_EndText(page);

        // TODO: Find proper value for this
        advance += 10;
    }
}

#define STEM_THICKNESS (STAFF_SPACE * 0.12)
#define STEM_UP_BOTTOM_RIGHT_X (STAFF_SPACE * 1.18)
#define STEM_UP_BOTTOM_RIGHT_Y (STAFF_SPACE * 0.168)
#define STEM_DOWN_TOP_LEFT_X (STAFF_SPACE * 0.0)
#define STEM_DOWN_TOP_LEFT_Y (STAFF_SPACE * -0.168)

void pdf_draw_measure(HPDF_Page page, struct Fonts f, struct Tab *t, int measure_number, float x, float width, int y, bool time_signature, bool key_signature) {
    struct Measure *m = &t->measures[measure_number];
    // Draw upper barline
    pdf_draw_barline(page, x + width, y + 50, y + 50 + STAFF_EM, BARLINE_THIN);

    // Draw lower barline
    pdf_draw_barline(page, x + width, y, y + (STAFF_SPACE * 5), BARLINE_THIN);

    struct MeasureWidthBreakdown breakdown = pdf_measure_ideal_width(t, measure_number, time_signature, key_signature);
    float scale = (width - breakdown.non_scalable_width) / breakdown.scalable_width;

    float offset = 0;

    // Key Signature?
    if (key_signature) {
        pdf_draw_key_signature(page, f, x + offset, y + 50, m->key);
        offset += key_signature_width(m->key);
    }

    // Time Signature?
    if (time_signature) {
        pdf_draw_time_signature(page, f, x + offset, y + 50, m->ts_top, m->ts_bottom);
        offset += TIME_SIGNATURE_WIDTH;
    }

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

    struct PitchClass key[7];
    get_key_signature(m->key, key);

    struct StemInfo *stem_data = malloc(r.groups_n * sizeof(struct StemInfo));
    evaluate_stems(t, r, stem_data, key);

    struct StemGroupDrawingData drawing_data[r.groups_n];

    for (int i = 0; i < r.groups_n; ++i) {
        struct StemGroup group = r.groups[i];

        drawing_data[i] = pdf_draw_notegroup(page, f, x + 12 + (SPACE_PER_TICK(t) * group.offset * scale), y + 50 + (STAFF_LINE / 2), t, group, stem_data[i], key);
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

struct StemGroupDrawingData pdf_draw_notegroup(HPDF_Page page, struct Fonts f, float x, float y, struct Tab *t, struct StemGroup group, struct StemInfo stem_data, struct PitchClass *key) {
    // Stem Direction (1 = up, 2 = down)
    const struct Tone e3 = {E, Natural, 3};
    const int eighth = t->ticks_per_quarter / 2;
    const int sixteenth = t->ticks_per_quarter / 4;

    const char *flag;

    struct StemGroupDrawingData data = {x, 0, 0};

    // Find origin of stem and draw it
    if (stem_data.direction == Up) {
        struct Note *origin = group_getLeast(t, &group, key);
        struct Tone tone = tone_add_semitones(t->info.tuning.strings[origin->string], origin->fret, key);
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
        struct Note *origin = group_getGreatest(t, &group, key);
        struct Tone tone = tone_add_semitones(t->info.tuning.strings[origin->string], origin->fret, key);
        float origin_y = y + ((STAFF_SPACE / 2) * tones_distance_diatonic(e3, note_to_tone(t, *origin, key)));
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
        //struct Tone tone = tone_add_semitones(t->info.tuning.strings[n->string], n->fret, key);
        struct Tone tone = tone_from_note(*n, t, key);

        bool need_accidental = false;
        enum PitchShift accidental;
        if (!key_signature_contains_tone(key, tone)) {
            need_accidental = true;
            accidental = key_signature_add_tone(key, tone);
        }

        if (need_accidental) {
            LOG("%d", accidental);
        }

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

#define REST_WHOLE      "\xEE\x93\xA3"
#define REST_HALF       "\xEE\x93\xA4"
#define REST_QUARTER    "\xEE\x93\xA5"
#define REST_EIGHTH     "\xEE\x93\xA6"
#define REST_SIXTEENTH  "\xEE\x93\xA7"

void pdf_draw_rest(HPDF_Page page, struct Fonts f, float x, float y, struct Tab *t, int value) {

}

void evaluate_stems(struct Tab *t, struct RhythmData r, struct StemInfo *data, struct PitchClass *key) {
    const struct Tone b3 = {B, Natural, 3};

    // Go through beams first
    for (int b = 0; b < r.beams_n; ++b) {
        struct BeamGroup *beam = &r.beams[b];
        enum StemDirection stem_direction;
        if (beam->stems_n == 2) {
            struct Note *least_1 = group_getLeast(t, beam->stems[0], key);
            struct Note *least_2 = group_getLeast(t, beam->stems[1], key);

            struct Tone least_t1 = note_to_tone(t, *least_1, key);
            struct Tone least_t2 = note_to_tone(t, *least_2, key);

            struct Note *greatest_1 = group_getGreatest(t, beam->stems[0], key);
            struct Note *greatest_2 = group_getGreatest(t, beam->stems[1], key);

            struct Tone greatest_t1 = note_to_tone(t, *greatest_1, key);
            struct Tone greatest_t2 = note_to_tone(t, *greatest_2, key);

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
                    struct Tone tone = tone_add_semitones(t->info.tuning.strings[n->string], n->fret, key);
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
                struct Tone tone = tone_add_semitones(t->info.tuning.strings[group.notes[0].note->string], group.notes[0].note->fret, key);
                if (tones_distance_diatonic(b3, tone) < 0)
                    stem_direction = Up;
                else
                    stem_direction = Down;
            } else if (group.notes_n > 1) {
                // Get bottom-most and top-most note
                struct Note *least = group_getLeast(t, &group, key);
                struct Note *greatest = group_getGreatest(t, &group, key);

                struct Tone least_t = tone_add_semitones(t->info.tuning.strings[least->string], least->fret, key);
                struct Tone greatest_t = tone_add_semitones(t->info.tuning.strings[greatest->string], greatest->fret, key);

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
                        struct Tone tone = tone_add_semitones(t->info.tuning.strings[n->string], n->fret, key);
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
            struct Tone tone = tone_add_semitones(t->info.tuning.strings[group.notes[0].note->string], group.notes[0].note->fret, key);
            if (abs(tones_distance_diatonic(b3, tone)) < 8)
                stem_length = 7;
            else
                stem_length = abs(tones_distance_diatonic(b3, tone));
        } else {
            // Get bottom-most and top-most note
            struct Note *least = group_getLeast(t, &group, key);
            struct Note *greatest = group_getGreatest(t, &group, key);

            struct Tone least_t = tone_add_semitones(t->info.tuning.strings[least->string], least->fret, key);
            struct Tone greatest_t = tone_add_semitones(t->info.tuning.strings[greatest->string], greatest->fret, key);

            if (abs(tones_distance_diatonic(least_t, greatest_t)) < 4)
                stem_length = 7;
            else
                stem_length = abs(tones_distance_diatonic(least_t, greatest_t)) + 4;
        }

        data[i].length = stem_length;
    }
}
