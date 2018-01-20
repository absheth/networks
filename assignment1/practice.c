#include <stdio.h>
#include <string.h>
int main(void) {
  char st[] ="Where there is will, there is a way.";
  char *ch;
  ch = strtok(st, " ");
  while (ch != NULL) {
  printf("%s\n", ch);
  ch = strtok(NULL, " ,");
  }
  // getch();
  return 0;
}
