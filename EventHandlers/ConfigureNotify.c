#include "../moreStuff.h"
#include "../structs.h"
#include "../stuff.h"
#include "EventHandlers.h"
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>

extern Fonts fonts;
extern Term term;
extern int masterFd;

int max(int a, int b) {
  if(a < b) return b;
  return a;
}

int temp = 54;

void handleConfigureNotify(XEvent *e) {
  int w = e->xconfigure.width;
  int h = e->xconfigure.height;
  printf("WIDTH AND HEIGHT!!! %d %d\n", w, h);

  int charWidth = fonts.normal->max_advance_width;
  int charHeight = fonts.normal->height;

  int cols = (w - (2 * MARGIN_LEFT)) / charWidth;
  int rows = ((h - 2 * MARGIN_LEFT) / charHeight) - 1;
  printf("ROWS AND COLS!!! %d %d\n", rows, cols);

  int oldRows = term.rows;
  int oldCols = term.cols;
  term.rows = rows;
  term.cols = cols;

  if ((term.lines = realloc(term.lines, sizeof(Line *) * term.rows)) == NULL) {
    die("OH NO!");
  };

  /*
   * On startup, I have 160 cols. Upon initial configure notify,
   * this gois down to 108. So cols decreases. But this decrease
   * happens when the term state is empty (or just prompt? Not sure
   * if the prompt is there yet or not). So we go from having 160 cols,
   * to 108 cols.
   *
   * Later, we INCREASE the cols. So that means when the lineData realloc
   * happens below, the columns that are greater than the previous size,
   * are getting intialized with garbage. Later, we try to render the
   * glyphs and get segfault.
   *
   * My idea: don't loop with the new rows/cols as bounds, but use
   * the max(old row/col, new row/col) . Then while looping, can 
   * check if current loop is greater than the prev size.
   * If it's larger, initialize with 0 values.
   * Otherwise, don't need to initialize with anything.
   * */
  for (int i = 0; i < term.rows; i++) {
    term.lines[i] = realloc(term.lines[i], sizeof(Line));
    term.lines[i]->dirty = 1;
    term.lines[i]->row = i;
    term.lines[i]->lineData = realloc(term.lines[i]->lineData, sizeof(JGlyph) * term.cols); // this is the line that breaks lineData[109]
    for(int j = 0; j < max(term.cols, oldCols); j++) {
      if(j >= term.cols) break;

      term.lines[i]->lineData[j].row = i;
      term.lines[i]->lineData[j].col = j;
      if(j >= oldCols) {
        term.lines[i]->lineData[j].c = '\0';
        term.lines[i]->lineData[j].bg = 0;
        term.lines[i]->lineData[j].fg = 0;
      }
    }
  }

  // resize tty
  struct winsize ws;
  ws.ws_row = term.rows;
  ws.ws_col = term.cols;
  // ws.ws_row = 20;
  // ws.ws_col = 20;
  // ws.ws_xpixel = term.cols * fonts.normal->max_advance_width;
  // ws.ws_ypixel = term.rows * fonts.normal->ascent;
  // ws.ws_xpixel = 0;
  // ws.ws_ypixel = 0;
  ws.ws_xpixel = 5000;
  ws.ws_ypixel = 5000;

	if (ioctl(masterFd, TIOCSWINSZ, &ws) < 0) fprintf(stderr, "Couldn't set window size: %s\n", strerror(errno));

  ioctl(masterFd, TIOCGWINSZ, &ws);
  printf("{\n");
  printf("  ws_row: %i\n", ws.ws_row);
  printf("  ws_col: %i\n", ws.ws_col);
  printf("  ws_xpixel: %i\n", ws.ws_xpixel);
  printf("  ws_ypixel: %i\n", ws.ws_ypixel);
  printf("}\n");
}
