#include <stdio.h>
#include <math.h>

int main (int argc, char **argv)
{
  int i;

  for (i = 0; i < 8207; i ++) {
    float val = powf (i, 4.0 / 3.0);
    printf ("%.10f, ", val);

    if ((i % 4) == 3)
      printf ("\n");
  }
  printf ("\n");
  return 0;
}
