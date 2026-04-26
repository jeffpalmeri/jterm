#include <X11/extensions/Xrender.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>

#include <fontconfig/fontconfig.h>

#include "pty.h"
#include "ptyFork.h"

#include "moreStuff.h"
#include "structs.h"
#include "stuff.h"
#include "fontStuff.h"
#include "EventHandlers/EventHandlers.h"

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MAX_SNAME 1000

int height = 1200;
int width = 1600;

Display *display;
Fonts fonts;
XftColor xft_font_color;
XftColor xft_bg_color;
XftDraw *draw;
Term term;
Window window;
Colormap colormap;
Visual *visual;
CS cs;
XEvent evt;
int masterFd;

void drawGlyph() {}

static void (*handler[LASTEvent])(XEvent *) = {
    [KeyPress] = handleKeyPress,
};

char *hack_nerd_font_string = "Hack Nerd Font:pixelsize=28:antialias=true:autohint=true";

void run() {
  do {
    XNextEvent(display, &evt);
  } while (evt.type != MapNotify);
  de_printf("after the do while loop\n");

  int xfd = XConnectionNumber(display);
  char buf[256];
  while (1) {
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(masterFd, &rfd);
    FD_SET(xfd, &rfd);
    //   struct timespec seltv, *tv;
    // tv = timeout >= 0 ? &seltv : NULL;

    if (pselect(MAX(xfd, masterFd) + 1, &rfd, NULL, NULL, NULL, NULL) < 0) {
      if (errno == EINTR)
        continue;
      die("select failed: %s\n", strerror(errno));
    }

    if (FD_ISSET(masterFd, &rfd)) {
      ssize_t numRead = read(masterFd, buf, 256);
      de_printf("**** numRead from masterFd: %zd\n", numRead);
      de_printf("And what the heck did I actually read?: %.*s\n", numRead, buf);
      // de_printf("And what the heck did I actually read2?: 0x%02x\n", (unsigned char)buf);

      // Helpful debug for seeing shell bytes
      fprintf(stdout, "\n----------start------------\n");
      for (ssize_t i = 0; i < numRead; i++) {
        fprintf(stdout, "%02x ", (unsigned char)buf[i]);
      }
      fprintf(stdout, "\n----------end------------\n");

      vtParse(buf, numRead, &term, &cs, handle_csi, put_char);
      renderTerm();
    }

    while (XPending(display)) {
      XNextEvent(display, &evt);
      // de_printf("---------start-------------\n");
      // de_printf("Event type is %d\n", evt.type);
      if (handler[evt.type]) {
        handler[evt.type](&evt);
      }
    }
  }
}

int main(int argc, char **argv) {
  memset(&cs, 0, sizeof(cs));
  const int maxSnLen = 1000;
  char slaveName[1000];

  struct termios ttyOrig;
  struct winsize ws;
  char *shell;
  if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1) {
    die("tcgetattr");
  }
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0) {
    die("ioctl-TIOCGWINSZ");
  }

  pid_t childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
  if (childPid == 0) {
    // shell = getenv("SHELL");
    // if(shell == NULL || *shell == '\0') shell = "/bin/sh";
    shell = "/bin/sh";
    execlp(shell, shell, (char *)NULL);
    die("execlp"); // If we get here, something went wrong
  }

  de_printf("masterFd %i\n", masterFd);

  term = (Term){5, 160, 0, 0, 0, 0, 0, 0, 0, 0};
  // How big will each "line" be?
  // #rows * sizeof(Line*),
  // and each Line will be (JGlyph * #cols) + int + int
  term.lines = malloc(sizeof(Line *) * term.rows);
  for (int i = 0; i < term.rows; i++) {
    term.lines[i] = malloc(sizeof(Line));
    // term.lines[i] = malloc((sizeof(JGlyph) * term.cols) + (2*sizeof(int)));
    term.lines[i]->dirty = 1;
    term.lines[i]->row = i;
    term.lines[i]->lineData = malloc(sizeof(JGlyph) * term.cols);
    for(int j = 0; j < term.cols; j++) {
      term.lines[i]->lineData[j].row = i;
      term.lines[i]->lineData[j].col = j;
    }
  }
  // term.lines = malloc(sizeof(Line*) * term.rows);
  // for(int i = 0; i < term.rows; i++) {
  //   term.lines[i] = malloc(sizeof(Line) * term.cols);
  // }

  display = XOpenDisplay(NULL);
  de_printf("Dispay opened\n");

  int screen = XDefaultScreen(display);
  visual = XDefaultVisual(display, screen);

  Window parent = XRootWindow(display, screen);

  XColor grey;
  colormap = DefaultColormap(display, screen);
  XParseColor(display, colormap, "#808080", &grey);
  XAllocColor(display, colormap, &grey);

  XSetWindowAttributes attrs = {0};

  attrs.background_pixel = grey.pixel;
  attrs.colormap = XCreateColormap(display, parent, visual, AllocNone);
  attrs.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

  window = XCreateWindow(display, parent, 0, 0, width, height, 0,
                         XDefaultDepth(display, screen), InputOutput, visual,
                         CWBackPixel | CWColormap | CWEventMask, &attrs);

  if (!window)
    die("XCreateWindow failed\n");

  de_printf("Complete, window id is %lu\n", window);

  fonts = (Fonts){NULL, NULL, NULL, NULL};
  loadFonts(hack_nerd_font_string);
  draw = XftDrawCreate(display, window, visual, colormap);

  XRenderColor xr = {0x0000, 0x0000, 0x0000, 0xffff};
  XftColorAllocValue(display, visual, colormap, &xr, &xft_font_color);

  XRenderColor bgxr = {grey.red, grey.green, grey.blue, 0xffff};
  XftColorAllocValue(display, visual, colormap, &bgxr, &xft_bg_color);

  de_printf("xft colors allocated\n");

  de_printf("\nStarting rendering next\n");

  XMapWindow(display, window);
  GC gc = XCreateGC(display, window, 0, NULL);
  XSetForeground(display, gc, BlackPixel(display, screen));
  XFillRectangle(display, window, gc, 0, 0, 800, 800);

  XMapWindow(display, window);
  XFlush(display);

  run();

  return 0;
}

// #define XK_BackSpace   0xff08  /* U+0008 BACKSPACE */
// #define XK_Left        0xff51  /* Move left, left arrow */
//
//
// case 'P': /* DCH -- Delete <n> char */
// 	DEFAULT(csiescseq.arg[0], 1);
// 	tdeletechar(csiescseq.arg[0]);
// 	break;
//
//
// case 'C': /* CUF -- Cursor <n> Forward */
// case 'a': /* HPR -- Cursor <n> Forward */
// 	DEFAULT(csiescseq.arg[0], 1);
// 	tmoveto(term.c.x+csiescseq.arg[0], term.c.y);
// 	break;
//
// 	abcdefg........LLLLB ----> CCCK
//             ^    
//                ^=======
// /usr/include/X11/keysymdef.h
// tomoveto
