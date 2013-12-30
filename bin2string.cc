#include <cstdio>

void out(int c)
{
  static int count = 0;
  if (count != 0) putchar(',');
  if (count % 16 == 0) putchar('\n');
  printf("%d", c);
  ++count;
}

int main(int argc, char *argv[])
{
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "Usage: bin2string <source-code-varname> [null]");
    return 1;
  }

  printf("const char %s[] = {", argv[1]);
  int c;
  while ((c = getchar()) != EOF)
    out(c);
  if (argc == 3)
    out(0);
  printf("\n};\n");

  return 0;
}
