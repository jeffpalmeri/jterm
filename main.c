#include <X11/extensions/Xrender.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include <fontconfig/fontconfig.h>

int height = 1200;
int width = 1600;

void die(const char *errString, ...) {
  va_list argPtr;

  va_start(argPtr, errString);
  vfprintf(stderr, errString, argPtr);
  va_end(argPtr);

  exit(1);
}

typedef struct {
  int row;
  int col;
  int cursor_x;
  int cursor_y;
} Term;

static Term term;

void drawCursor(XftFont *font, XftColor *color, XftDraw *draw) {
  XRectangle rab;
  rab.x = 0;
  rab.y = 0;
  rab.height = font->height;
  rab.width = font->max_advance_width;
  XftDrawRect(draw, color, term.cursor_x, term.cursor_y - font->ascent, rab.width, rab.height);
}

void drawGlyph()
{

}

void handleKeyPress(XEvent *e) {}

static void (*handler[LASTEvent])(XEvent *) = {
  [KeyPress] = handleKeyPress,
};

char* font1 = "Hack Nerd Font:pixelsize=28:antialias=true:autohint=true";

FcPattern* loadFont()
{
  if(!FcInit()) {
    die("FcInit failed\n");
  }

  FcPattern* pattern = FcNameParse((const FcChar8 *)font1);
  if(!pattern) {
    FcFini();
    die("Could not parse font name\n");
  }

  FcChar8* family;
  FcChar8* file;
  double size;
  double pixelsize;

  if(FcPatternGetString(pattern, FC_FAMILY, 0, &family) == FcResultMatch) {
    printf("Font family: %s\n", family);
  }

  if(FcPatternGetString(pattern, FC_FILE, 0, &file) == FcResultMatch) {
    printf("Font file: %s\n", file);
  } else {
    printf("No file?\n");
  }

  if(FcPatternGetDouble(pattern, FC_PIXEL_SIZE, 0, &pixelsize) == FcResultMatch) {
    printf("Pixel size: %.1f\n", pixelsize);
  } else {
    printf("No size?\n");
  }

  FcPatternAddDouble(pattern, FC_PIXEL_SIZE, 12);
  FcPatternAddDouble(pattern, FC_SIZE, 12);

  if(FcPatternGetDouble(pattern, FC_SIZE, 0, &size) == FcResultMatch) {
    printf("Font size: %.1f\n", size);
  } else {
    printf("No size?\n");
  }

  printf("*** Doing substitutes and matching now ***\n");

  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);
  FcResult result;
  FcPattern *match = FcFontMatch(NULL, pattern, &result);

  if(result != FcResultMatch) {
    fprintf(stderr, "Font match failed: %d\n", result);
  }

  printf("Font match seems to have succeeded\n");
  printf("Trying file again now\n");

  if(FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch) {
    printf("Font file: %s\n", file);
  } else {
    fprintf(stderr, "No file, but should have one\n");
  }

  printf("*** Moving on to Xft now ***\n");

  return match;
}

XftFont* openXft(Display *display, FcPattern *match)
{
  XftFont *font = XftFontOpenPattern(display, match);
  if(!font) die("XftFontOpenPattern failed\n");

  printf("Xft font opened successfully\n");

  printf("Loaded font: ascent=%d descent=%d max_advance=%d\n", 
      font->ascent, font->descent, font->max_advance_width);

  return font;
}

int main(int argc, char** argv)
{
  term = (Term){ 24, 80, 50, 100 };

  Display *display = XOpenDisplay(NULL);
  printf("Dispay opened\n");

  int screen = XDefaultScreen(display);
  Visual *visual = XDefaultVisual(display, screen);

  Window parent = XRootWindow(display, screen);

  XColor grey;
  Colormap colormap = DefaultColormap(display, screen);
  XParseColor(display, colormap, "#808080", &grey);
  XAllocColor(display, colormap, &grey);

  XSetWindowAttributes attrs = {0};

  attrs.background_pixel = grey.pixel;
  attrs.colormap = XCreateColormap(display, parent, visual, AllocNone);
  attrs.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

  Window window = XCreateWindow(
      display,
      parent,
      0,
      0,
      width,
      height,
      0,
      XDefaultDepth(display, screen),
      InputOutput,
      visual,
      CWBackPixel | CWColormap | CWEventMask,
      &attrs
  );

  if(!window) die("XCreateWindow failed\n");

  printf("Complete, window id is %lu\n", window);

  FcPattern *match = loadFont();
  XftFont *font = openXft(display, match);
  XftDraw *draw = XftDrawCreate(display, window, visual, colormap);

  XftColor xft_font_color;
  XRenderColor xr = {0x0000, 0x0000 , 0x0000, 0xffff};
  XftColorAllocValue(display, visual, colormap, &xr, &xft_font_color);

  XftColor xft_bg_color;
  XRenderColor bgxr = {grey.red, grey.green, grey.blue, 0xffff};
  XftColorAllocValue(display, visual, colormap, &bgxr, &xft_bg_color);

  printf("xft colors allocated\n");

  printf("\nStarting rendering next\n");
  
  XMapWindow(display, window);
  GC gc = XCreateGC(display, window, 0, NULL);
  XSetForeground(display, gc, BlackPixel(display, screen));
  XFillRectangle(display, window, gc, 0, 0, 800, 800);
  XFreeGC(display, gc);

  XMapWindow(display, window);
  XFlush(display);

  XEvent evt;
  XFlush(display);

  do {
    XNextEvent(display, &evt);
  } while (evt.type != MapNotify);
  printf("after the do while loop\n");

  XRectangle rab;
  rab.x = 0;
  rab.y = 0;
  rab.height = font->height;
  rab.width = font->max_advance_width;
  XftDrawRect(draw, &xft_font_color, term.cursor_x, term.cursor_y - font->ascent, rab.width, rab.height);


  while(1) {
  // while(XPending(display)) {
    XNextEvent(display, &evt);
    printf("Event type is %d\n", evt.type);

    if(evt.type == KeyPress) {
      XKeyEvent *xke = &evt.xkey;
      KeySym keysym = NoSymbol;
      char buf[64];
      int len;

      len = XLookupString(xke, buf, sizeof buf, &keysym, NULL);
      printf("==========\n");
      printf("KeyPress len is: %d\n", len);
      printf("KeyPress buf is: %s\n", buf);
      printf("KeyPress buf in hex: ");
      for(int i = 0; i < len; i++) {
        printf("0x%02x ", (unsigned char)buf[i]);
      }
      printf("\n");
      printf("KeySym is: %lu\n", keysym);
      printf("KeySym to string is: %s\n", XKeysymToString(keysym));
      printf("==========\n\n");

      if(xke->state & Mod1Mask) {
        printf("Typed with ALT held down and len is %d\n", len);
      }

      // if(keysym == 65293) { // return
      if(keysym == XK_Return) { // return
        // Delete the cursor from the end of the line
        drawCursor(font, &xft_bg_color, draw);

        // XK_Return;
        // term.cursor_y += 50;
        term.cursor_y += font->height * 1.2;
        term.cursor_x = 50;

        // Draw cursor now at the start of the new line
        drawCursor(font, &xft_font_color, draw);

        continue;
      }

      if(keysym == 65288) { // backspace
                            //
        printf("Backspace!\n");
        // Delete the previous cursor
        drawCursor(font, &xft_bg_color, draw);

        int cell_width = font->max_advance_width;
        int cell_height = font->height;
        XRectangle r;
        r.x = 0;
        r.y = 0;
        r.height = cell_height;
        r.width = cell_width;

        term.cursor_x -= font->max_advance_width;

        int x = term.cursor_x;
        int y = term.cursor_y;

        XftDrawRect(draw, &xft_bg_color, x, y - font->ascent, cell_width, cell_height); // width and height?
        XftDrawSetClipRectangles(draw, x, y - font->ascent, &r, 1);
        XftDrawSetClip(draw, 0);

        // Draw new cursor after the backspace
        drawCursor(font, &xft_font_color, draw);

        continue;
      }

      if(len > 0) {
        // Maybe using XKeysymToString is the more correct way than doing this?
        buf[len] = '\0';
        unsigned int codepoint = (unsigned char)buf[0];
        printf("Ok getting serious, the letter typed is %s\n", buf);

        FT_UInt glyph = XftCharIndex(display, font, codepoint); 
        printf("XftCharIndex() seems to be called successfully %u\n", glyph);
        // XftColor xft_font_color;
        // XRenderColor xr = {0x0000, 0x0000 , 0x0000, 0xffff};
        // XftColorAllocValue(display, visual, colormap, &xr, &xft_font_color);
        // printf("xft color allocated\n");

        int cell_width = font->max_advance_width;
        // int cell_height = font->ascent + font->descent;
        int cell_height = font->height;


        XRectangle r;
        r.x = 0;
        r.y = 0;
        r.height = cell_height;
        r.width = cell_width;

        int x = term.cursor_x;
        int y = term.cursor_y;

        XftDrawRect(draw, &xft_bg_color, x, y - font->ascent, cell_width, cell_height); // width and height?
        XftDrawSetClipRectangles(draw, x, y - font->ascent, &r, 1);
        XftGlyphFontSpec spec;
        spec.font = font;
        spec.glyph = glyph;
        spec.x = x;
        spec.y = y;

        XftDrawGlyphFontSpec(draw, &xft_font_color, &spec, 1);

        XftDrawSetClip(draw, 0);

        term.cursor_x += font->max_advance_width;

        // Draw the new cursor position, since last char has been drawn, and x position updated
        drawCursor(font, &xft_font_color, draw);
      }
    }
  }

  return 0;
}

