#include "export.h"
#include <hpdf.h>

void generate_text_file(struct Tab *t, const char *file) {

}





struct Fonts {
    HPDF_Font normal;
    HPDF_Font bold;
    HPDF_Font italics;
    HPDF_Font monospace;
    HPDF_Font music;
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

#define SPACE_PER_TICK(tab) (18 / tab->ticks_per_quarter)

struct Fonts load_fonts(HPDF_Doc doc);
void pdf_display_header(HPDF_Page page, struct Fonts f, struct Tab *t);
void pdf_draw_staff(HPDF_Page page, struct Fonts f, int y);
void pdf_draw_measure(HPDF_Page page, struct Fonts f, struct Measure *m, float x, float width, int staff_y);
void pdf_draw_barline(HPDF_Page page, int x, int y1, int y2, float thickness);

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
        float ideal = 0;
        for (int j = 0; j < m->notes_n; ++j) {
            float n_preffered = SPACE_PER_TICK(t) * m->notes[j].length;
            float n_min = 2.5 * STAFF_SPACE;
            if (n_preffered < n_min)
                ideal += n_min;
            else
                ideal += n_preffered;
        }
        ideal_space[i] = ideal;
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
           pdf_draw_measure(pg1, f, &t->measures[m], x, calculated_width[m], 600 - (100 * i));

           x += calculated_width[m];
           ++m;
        }
        x = MARGIN_LEFT + 100;
    }

    HPDF_SaveToFile(pdf, file);
    HPDF_Free(pdf);
}

struct Fonts load_fonts(HPDF_Doc doc) {
    struct Fonts f;

    f.normal = HPDF_GetFont(doc, "Times-Roman", "StandardEncoding");
    f.bold = HPDF_GetFont(doc, "Times-Bold", "StandardEncoding");
    f.italics = HPDF_GetFont(doc, "Times-Italic", "StandardEncoding");
    f.monospace = HPDF_GetFont(doc, "Courier", "StandardEncoding");
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

void pdf_draw_measure(HPDF_Page page, struct Fonts f, struct Measure *m, float x, float width, int y) {
    // Draw upper barline
    pdf_draw_barline(page, x + width, y + 50, y + 50 + STAFF_EM, BARLINE_THIN);

    // Draw lower barline
    pdf_draw_barline(page, x + width, y, y + (STAFF_SPACE * 5), BARLINE_THIN);
}

void pdf_draw_barline(HPDF_Page page, int x, int y1, int y2, float thickness) {
    HPDF_Page_SetLineWidth(page, thickness);
    HPDF_Page_SetLineCap(page, HPDF_BUTT_END);

    HPDF_Page_MoveTo(page, x + (thickness / 2), y1);
    HPDF_Page_LineTo(page, x + (thickness / 2), y2);
    HPDF_Page_Stroke(page);
}
