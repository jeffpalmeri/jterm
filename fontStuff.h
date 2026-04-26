#ifndef FONTSTUFF
#define FONTSTUFF

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

void loadFonts(char *font_string);
void openXft(Display *display, FcPattern *match, XftFont **fo);

#endif
