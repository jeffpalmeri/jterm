#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "stuff.h"

/*
 * When starting the terminal for the first time
 * 1. I need to determine exactly what the starting state is.
 *    This will be a term mode, esc mode, csi struct... for now.
 *    Probably more later.
 * 2. Give some byte sequence, let the state change, and then assert
 *    that the new state is expected.
 *
 * So I'll need a new Term + whatever else I need that might not live
 * on Term, then run the bytes through a parser that will mutate Term.
 */

void TestCase(const char *p, int num_bytes, CS *expectCs);
int csCompare(CS *cs1, CS *cs2);

// typedef struct TestCase {
//   int byteArrLen;
//   char byteArr[512];
// } TestCase;

Term term;
CS cs;

// int same_buf(char *s1, char *s2) {
//
// }

// TestCase testCase1 {10, {0x1b, 0x5b, 0x3f, 0x32, 0x30, 0x30, 0x34, 0x68, 0x73, 0x68, 0x2d, 0x35, 0x2e, 0x33, 0x24, 0x20}};
const char enterBracketedEscapeMode[8] = {0x1b, 0x5b, 0x3f, 0x32, 0x30, 0x30, 0x34, 0x68};
const char *p = enterBracketedEscapeMode;
CS expectCs = {
  {0x1b, 0x5b, 0x3f, 0x32, 0x30, 0x30, 0x34},
  1,
  5,
  {2004},
  1,
  {1}
};

char byteArr1[] = {0x1b, 0x5b, 0x3f, 0x32, 0x30, 0x30, 0x34, 0x68, 0x73, 0x68, 0x2d, 0x35, 0x2e, 0x33, 0x24, 0x20};
int main(int argc, char *argv[])
{
  // assert(term.mode == 0);
  // assert(term.esc == 0);
  // vtParse(byteArr1[0]);
  // assert(term.mode == 0);
  // assert(term.esc == 1);
  //
  // vtParse(byteArr1[1]);
  // assert(term.esc == 3);
  //
  // vtParse(byteArr1[2]);
  // assert(term.esc == 3);
  // assert(cs.len == 1);
  // assert(cs.buf[0] == '?');
  //
  //
  // vtParse(byteArr1[3]);
  // assert(cs.len == 2);
  // assert(cs.buf[0] == '?');
  // assert(cs.buf[1] == '2');
  // vtParse(byteArr1[4]);
  // assert(cs.len == 3);
  // assert(cs.buf[0] == '?');
  // assert(cs.buf[1] == '2');
  // assert(cs.buf[2] == '0');
  //
  // vtParse(byteArr1[5]);
  // assert(cs.len == 4);
  // assert(cs.buf[0] == '?');
  // assert(cs.buf[1] == '2');
  // assert(cs.buf[2] == '0');
  // assert(cs.buf[3] == '0');
  //
  // vtParse(byteArr1[6]);
  // assert(cs.len == 5);
  // assert(cs.buf[0] == '?');
  // assert(cs.buf[1] == '2');
  // assert(cs.buf[2] == '0');
  // assert(cs.buf[3] == '0');
  // assert(cs.buf[4] == '4');
  //
  // vtParse(byteArr1[7]);
  // assert(cs.len == 6);
  // assert(cs.buf[0] == '?');
  // assert(cs.buf[1] == '2');
  // assert(cs.buf[2] == '0');
  // assert(cs.buf[3] == '0');
  // assert(cs.buf[4] == '4');
  // assert(cs.buf[5] == 'h');
  //
  // printf("Arg len: %u\n", cs.len);
  // printf("Buf: %s\n", cs.buf);
  //
  //
  // memset(&cs, 0, sizeof(cs));
  // memset(&term, 0, sizeof(term));
  //
  // const char *exp = "?2004h";
  // vtParse2(p, 8);
  // int same = strcmp(cs.buf, exp);
  // assert(strcmp(cs.buf, exp) == 0);
  // printf("cs.buf: %s\n", cs.buf);
  // printf("exp: %s\n", exp);
  // printf("are these the same or not %d\n", same);

  CS expectCS1 = {
    .buf = "?2004h",
    .len = 6,
    .priv = '?',
    .narg = 1,
    .arg = {2004},
    .mode = "h",
  };
  TestCase(p, 8, &expectCS1);

  const char randomThreeArgs[8] = {0x1b, 0x5b, 0x30, 0x3b, 0x34, 0x3b, 0x35, 0x6d};
  p = randomThreeArgs;
  CS expectCS2 = {
    .buf = "0;4;5m",
    .len = 6,
    .priv = '\0',
    .narg = 3,
    .arg = {0, 4, 5},
    .mode = "m",
  };
  TestCase(p, 8, &expectCS2);

  const char leaveBracketedEscapeMode[8] = {0x1b, 0x5b, 0x3f, 0x32, 0x30, 0x30, 0x34, 0x6c};
  p = leaveBracketedEscapeMode;
  CS expectCS3 = {
    .buf = "?2004l",
    .len = 6,
    .priv = '?',
    .narg = 1,
    .arg = {2004},
    .mode = "l",
  };
  TestCase(p, 8, &expectCS3);


  return 0;
}

/*
 * TestCase #1
 * What do I need
 * Input bytes, # bytes, expectation cs
 */
void TestCase(const char *bytes, int num_bytes, CS *expectCs) {
  memset(&term, 0, sizeof(term));
  memset(&cs, 0, sizeof(cs));
  memset(&cs.buf, 0xAA, sizeof(cs.buf));

  vtParse2(bytes, num_bytes);
  // assert(strcmp(cs.buf, expectCs->buf) == 0);
  // cs.buf[6] = 'a';
  printf("first %x\n", cs.buf[6]);
  printf("second %x\n", expectCs->buf[6]);
  // assert(cs.len == expectCs->len);
  // Need to make these four pass
  // assert(cs.priv == expectCs->priv);
  // assert(cs.narg == expectCs->narg);
  // assert(memcmp(cs.arg, expectCs->arg, sizeof(cs.arg)) == 0);
  // assert(memcmp(cs.mode, expectCs->mode, sizeof(cs.mode)) == 0);
  csCompare(&cs, expectCs);
}


int csCompare(CS *cs1, CS *cs2) {
  assert(strcmp(cs1->buf, cs2->buf) == 0);
  assert(cs1->len == cs2->len);
  // Need to make these four pass
  assert(cs1->priv == cs2->priv);
  assert(cs1->narg == cs2->narg);
  assert(memcmp(cs1->arg, cs2->arg, sizeof(cs.arg)) == 0);
  assert(memcmp(cs1->mode, cs2->mode, sizeof(cs.mode)) == 0);
  return 1;
}
