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

#define STAFF_HEIGHT 24
void pdf_draw_staff(HPDF_Page page, struct Fonts f, int y) {
    HPDF_REAL width = HPDF_Page_GetWidth(page);

    // Draw staff lines
    HPDF_Page_SetLineWidth(page, 0.5);
    HPDF_Page_SetLineCap(page, HPDF_BUTT_END);
    for (int i = 0; i < 5; ++i) {
        HPDF_Page_MoveTo(page, MARGIN_LEFT, y + ((STAFF_HEIGHT / 4) * i));
        HPDF_Page_LineTo(page, width - MARGIN_RIGHT, y + ((STAFF_HEIGHT / 4) * i));
        HPDF_Page_Stroke(page);
    }

    HPDF_Page_BeginText(page);

    // The baseline for the G Clef is the G on the staff (2nd line)
    int g_clef_baseline = STAFF_HEIGHT / 4;
    HPDF_Page_SetFontAndSize(page, f.music, STAFF_HEIGHT);
    HPDF_Page_MoveTextPos(page, MARGIN_LEFT + (STAFF_HEIGHT / 8), y + g_clef_baseline);
    HPDF_Page_ShowText(page, "\xEE\x81\x90");
}
