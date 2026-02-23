#include <stdio.h>

void printBinary(int n)
{
  char s[33];
  int orig = n;

  if(n == 0) {
    s[0] = '0';
    s[1] = '\0';
    return;
  }

  int i = 0;
  while(n > 0) {
    s[i++] = n%2 ? '1' : '0';
    n /= 2;
  }
  s[i] = '\0';

  for(int j = 0; j < i/2; j++) {
    char temp = s[j];
    s[j] = s[i-j-1];
    s[i-j-1] = temp;
  }

  printf("%d: %s\n", orig, s);
  return;
}

int main(int argc, char *argv[])
{
  int n1 = 5;
  int n2 = 15;
  int n3 = 123;
  int n4 = 127;
  int n5 = 65;

  printBinary(n1);
  printBinary(n2);
  printBinary(n3);
  printBinary(n4);
  printBinary(n5);

  return 0;
}
