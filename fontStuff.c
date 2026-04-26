#include "fontStuff.h"
#include "moreStuff.h"

extern Fonts fonts;
extern Display *display;

void openXft(Display *display, FcPattern *match, XftFont **fo) {
  // bfont = XftFontOpenPattern(display, match);
  *fo = XftFontOpenPattern(display, match);

  if (!fo) die("XftFontOpenPattern failed\n");
}

void loadFonts(char *font_string) {
  if (!FcInit()) {
    die("FcInit failed\n");
  }

  FcPattern *pattern = FcNameParse((const FcChar8 *)font_string);
  if (!pattern) {
    FcFini();
    die("Could not parse font name\n");
  }

  FcPatternAddDouble(pattern, FC_PIXEL_SIZE, 12);
  FcPatternAddDouble(pattern, FC_SIZE, 12);

  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);
  FcResult result;
  FcPattern *match;

  match = FcFontMatch(NULL, pattern, &result);

  openXft(display, match, &fonts.normal);

  // italic
  FcPatternDel(pattern, FC_SLANT);
  FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ITALIC);
  match = FcFontMatch(NULL, pattern, &result);
  if (result != FcResultMatch) {
    fprintf(stderr, "[italic] Font match failed: %d\n", result);
  }
  openXft(display, match, &fonts.italic);

  // bold_italic
  FcPatternDel(pattern, FC_WEIGHT);
  FcPatternAddInteger(pattern, FC_WEIGHT, FC_WEIGHT_BOLD);
  match = FcFontMatch(NULL, pattern, &result);
  if (result != FcResultMatch) {
    fprintf(stderr, "[bold_italic] Font match failed: %d\n", result);
  }
  openXft(display, match, &fonts.bold_italic);

  // bold
  FcPatternDel(pattern, FC_SLANT);
  FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ROMAN);
  match = FcFontMatch(NULL, pattern, &result);
  if (result != FcResultMatch) {
    fprintf(stderr, "[bold] Font match failed: %d\n", result);
  }
  openXft(display, match, &fonts.bold);
}
