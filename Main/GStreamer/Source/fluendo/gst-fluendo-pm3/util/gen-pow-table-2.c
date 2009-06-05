#include <stdio.h>
#include <math.h>

int main (int argc, char **argv)
{
  int i;
  printf ("/*\n * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.\n */\n\n");
  printf ("#ifndef __TABLE_POWTABLE_2_HH__\n");
  printf ("#define __TABLE_POWTABLE_2_HH__\n\n");
  printf ("static const gfloat pow_2_table[] = {\n");

  for (i = 0; i < 326 + 45 + 1; i ++) {
    float val = powf (2.0, 0.25 * (i - 326));
    printf ("%.15f, ", val);

    if ((i % 4) == 3)
      printf ("\n");
  }
  printf ("};\n");
  printf ("\n#endif\n");
  return 0;
}
