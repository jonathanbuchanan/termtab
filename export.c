#include "export.h"
#include <hpdf.h>

void generate_text_file(struct Tab *t, const char *file) {

}





struct Fonts {
    HPDF_Font normal;
    HPDF_Font bold;
    HPDF_Font italics;
    HPDF_Font monospace;
};
#define TITLE_SIZE 36
#define AUTHOR_SIZE 14

struct Fonts load_fonts(HPDF_Doc doc);
void pdf_display_title(HPDF_Page page, struct Fonts f, struct Tab *t);

void generate_pdf(struct Tab *t, const char *file) {
    HPDF_Doc pdf;
    pdf = HPDF_New(NULL, NULL);
    struct Fonts f = load_fonts(pdf);



    HPDF_Page pg1 = HPDF_AddPage(pdf);
    pdf_display_title(pg1, f, t);



    HPDF_SaveToFile(pdf, file);
    HPDF_Free(pdf);
}

struct Fonts load_fonts(HPDF_Doc doc) {
    struct Fonts f;

    f.normal = HPDF_GetFont(doc, "Times-Roman", "StandardEncoding");
    f.bold = HPDF_GetFont(doc, "Times-Bold", "StandardEncoding");
    f.italics = HPDF_GetFont(doc, "Times-Italic", "StandardEncoding");
    f.monospace = HPDF_GetFont(doc, "Courier", "StandardEncoding");

    return f;
}

void pdf_display_title(HPDF_Page page, struct Fonts f, struct Tab *t) {
    HPDF_REAL width, height;
    width = HPDF_Page_GetWidth(page);
    height = HPDF_Page_GetHeight(page);

    HPDF_Page_BeginText(page);
    HPDF_Page_SetFontAndSize(page, f.normal, TITLE_SIZE);

    HPDF_Page_TextRect(page, 0, height, width, height - 100, t->info.title, HPDF_TALIGN_CENTER, NULL);

    HPDF_Page_EndText(page);
}
