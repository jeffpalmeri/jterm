#ifndef MORESTUFF
#define MORESTUFF

#include "structs.h"

void die(const char *errString, ...);
int csi_ending_char(char b);
void parse_csi(CS *cs);
void printCS(CS *cs);
void handle_csi(CS *cs, Term *term);
void vtParse(const char *p, int size, Term *term, CS *cs, void (*handle_csi)(CS *cs, Term *term), void (*putChar)(Term *term, int i, const char *p));
void put_char(Term *term, int i, const char *p);

int calc_top(Term *term);

#endif
