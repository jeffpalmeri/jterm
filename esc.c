#include <stdio.h>
#include <unistd.h>

int main() {
  // printf("^[B");

  // Source - https://stackoverflow.com/a/7415229
  // Posted by Baltasarq, modified by community. See post 'Timeline' for change
  // history Retrieved 2026-02-01, License - CC BY-SA 3.0

  static const char *CSI = "\33[";
  // static const char *CSI = "\x1b[";
  // printf("%s%s", CSI, "A");
  // printf("%s%s", CSI, "A");
  // printf("%s%s", CSI, "A");
  // printf("%s%s", CSI, "A");
  printf("\33A");
  printf("\33A");
  printf("\33A");
  printf("\33A");
  fflush(stdout);
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s%s", CSI, "C");
  printf("%s", "\007"); // bell?
  fflush(stdout);
  // printf("\033#6");
  printf("%s%s", CSI, "31m"); // RED
  printf("hello\n");
  sleep(5);
  return 0;
}

/* 1    | 2    |
 * ESC  | [    | ?
 * 0x1b | 0x5b | 0x3f
 * 27   | 91   | 63
 * 033  | 0133 |
 *
 *
 *
 *
 *
 *
 *
 *
 */
