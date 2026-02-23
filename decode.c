#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
// #include <sys/types.h>

#define BETWEEN(x, a, b)	((a) <= (x) && (x) <= (b))
#define LEN(a)			(sizeof(a) / sizeof(a)[0])

/* Arbitrary sizes */
#define UTF_INVALID   0xFFFD
#define UTF_SIZ       4
#define ESC_BUF_SIZ   (128*UTF_SIZ)
#define ESC_ARG_SIZ   16
#define STR_BUF_SIZ   ESC_BUF_SIZ
#define STR_ARG_SIZ   ESC_ARG_SIZ

/* macros */
#define IS_SET(flag)		((term.mode & (flag)) != 0)
#define ISCONTROLC0(c)		(BETWEEN(c, 0, 0x1f) || (c) == 0x7f)
#define ISCONTROLC1(c)		(BETWEEN(c, 0x80, 0x9f))
#define ISCONTROL(c)		(ISCONTROLC0(c) || ISCONTROLC1(c))
#define ISDELIM(u)		(u && wcschr(worddelimiters, u))

typedef uint_least32_t Rune;
typedef unsigned char uchar;

static const uchar utfbyte[UTF_SIZ + 1] = {0x80,    0, 0xC0, 0xE0, 0xF0};
static const uchar utfmask[UTF_SIZ + 1] = {0xC0, 0x80, 0xE0, 0xF0, 0xF8};
static const Rune utfmin[UTF_SIZ + 1] = {       0,    0,  0x80,  0x800,  0x10000};
static const Rune utfmax[UTF_SIZ + 1] = {0x10FFFF, 0x7F, 0x7FF, 0xFFFF, 0x10FFFF};


static size_t utf8decode(const char *, Rune *, size_t);
static Rune utf8decodebyte(char, size_t *);
static char utf8encodebyte(Rune, size_t);
static size_t utf8validate(Rune *, size_t);

size_t
utf8decode(const char *c, Rune *u, size_t clen)
{
	size_t i, j, len, type;
	Rune udecoded;

	*u = UTF_INVALID;
	if (!clen)
		return 0;
	udecoded = utf8decodebyte(c[0], &len);
	if (!BETWEEN(len, 1, UTF_SIZ))
		return 1;
	for (i = 1, j = 1; i < clen && j < len; ++i, ++j) {
		udecoded = (udecoded << 6) | utf8decodebyte(c[i], &type);
		if (type != 0)
			return j;
	}
	if (j < len)
		return 0;
	*u = udecoded;
	utf8validate(u, len);

	return len;
}

Rune
utf8decodebyte(char c, size_t *i)
{
	for (*i = 0; *i < LEN(utfmask); ++(*i))
		if (((uchar)c & utfmask[*i]) == utfbyte[*i])
			return (uchar)c & ~utfmask[*i];

	return 0;
}

size_t
utf8validate(Rune *u, size_t i)
{
	if (!BETWEEN(*u, utfmin[i], utfmax[i]) || BETWEEN(*u, 0xD800, 0xDFFF))
		*u = UTF_INVALID;
	for (i = 1; *u > utfmax[i]; ++i)
		;

	return i;
}

// 0x1b 0x5b == ESC[ == '\033' '\0133' == 27 91

int main(int argc, char *argv[])
{
  char byteArr1[] = {0x1b, 0x5b, 0x3f, 0x32, 0x30, 0x30, 0x34, 0x68, 0x73, 0x68, 0x2d, 0x35, 0x2e, 0x33, 0x24, 0x20};

  char byteArr2[] = {0x0d, 0x0a, 0x1b, 0x5b, 0x3f, 0x32, 0x30, 0x30, 0x34, 0x6c, 0x0d, 0x1b, 0x5b, 0x3f, 0x32, 0x30, 0x30, 0x34, 0x68, 0x73, 0x68, 0x2d, 0x35, 0x2e, 0x33, 0x24, 0x20};

	Rune u;
  Rune res = utf8decode(byteArr1, &u, sizeof(byteArr1));
  printf("The rune is %u\n", res);
  return 0;
}

/*
 * esc[?2004h ----> after hitting the h, it calls csiparse()+csihandle()
 *    [j
 *   -- 2004h = turn on bracketd paste mode (2004l = turn it off)
 * Main loop --> ttyread --> twrite --> tputc --> tcontrolcode
 * -- For the first byte upon starting up, which is an escape, st
 *    basically decides that nothing needs to be done or printed
 *    because of the current states, and then we move the esc state
 *    into a start state (ESC_START) by flipping that bit on.
 * -- The next byte which is '[' will flip on the ESC_CSI bit for the
 *    escape state
 * 65 = 1000001 so the modes that are on are MODE_WRAP and MODE_UTF8
 *
 *
 * lldb ./st
 * b x.c:1973 (on the ttyread() call in the main run() loop
 * run
 *
 * The first time hitting this breakpoint is already very informational.
 * I see first read happen from the ttyfd, and then can look at that buffer.
 * print buflen
 * print ret
 * print buf
 */
