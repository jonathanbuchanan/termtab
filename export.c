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

struct Fonts load_fonts(HPDF_Doc doc);
void pdf_display_header(HPDF_Page page, struct Fonts f, struct Tab *t);
void pdf_draw_staff(HPDF_Page page, struct Fonts f, int y);
void pdf_draw_barline(HPDF_Page page, int x, int y1, int y2, float thickness);

void generate_pdf(struct Tab *t, const char *file) {
    HPDF_Doc pdf;
    pdf = HPDF_New(NULL, NULL);
    HPDF_UseUTFEncodings(pdf);
    struct Fonts f = load_fonts(pdf);



    HPDF_Page pg1 = HPDF_AddPage(pdf);
    pdf_display_header(pg1, f, t);
    pdf_draw_staff(pg1, f, 50);



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

#define STAFF_EM 24
#define STAFF_SPACE (STAFF_EM / 4)
#define BARLINE_THIN (STAFF_SPACE * 0.16)
#define BARLINE_THICK (STAFF_SPACE * 0.5)
#define STAFF_LINE (STAFF_SPACE * 0.13)
#define BRACKET_EXTENSION 2
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

void pdf_draw_barline(HPDF_Page page, int x, int y1, int y2, float thickness) {
    HPDF_Page_SetLineWidth(page, thickness);
    HPDF_Page_SetLineCap(page, HPDF_BUTT_END);

    HPDF_Page_MoveTo(page, x + (thickness / 2), y1);
    HPDF_Page_LineTo(page, x + (thickness / 2), y2);
    HPDF_Page_Stroke(page);
}
