
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <schroedinger/schro.h>
#include <schroedinger/schrowavelet.h>
#include <schroedinger/schrooil.h>

#define OIL_ENABLE_UNSTABLE_API
#include <liboil/liboil.h>
#include <liboil/liboilprofile.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define N 10000

int16_t a[N];
int16_t b[N];
int16_t c[N];

int16_t table[65536];

int oil_profile_get_min (OilProfile *prof)
{
  int i;
  int min;
  min = prof->hist_time[0];
  for(i=0;i<10;i++){
    if (prof->hist_count[i] > 0) {
      if (prof->hist_time[i] < min) {
        min = prof->hist_time[i];
      }
    }
  }
  return min;
}

#if 0
static int
__schro_dequantise (int q, int quant_factor, int quant_offset)
{
  if (q == 0) return 0;
  if (q < 0) {
    return -((-q * quant_factor + quant_offset + 2)>>2);
  } else {
    return (q * quant_factor + quant_offset + 2)>>2;
  }
}
#endif

#if 0
static int
__schro_quantise_shift (int value, int quant_shift)
{
  unsigned int x;

  if (value == 0) return 0;
  if (value < 0) {
    x = (-value)>>quant_shift;
    value = -x;
  } else {
    x = value>>quant_shift;
    value = x;
  }
  return value;
}
#endif

#if 0
static int
__schro_quantise_table (int value, int16_t *table)
{
  return table[value];
}
#endif

void
schro_dequantise_s16_ref (int16_t *dest, int16_t *src, int quant_index,
    schro_bool is_intra, int n)
{
  int i;
  int quant_factor = schro_table_quant[quant_index];
  int quant_offset = is_intra ? schro_table_offset_1_2[quant_index] :
    schro_table_offset_3_8[quant_index];

  for(i=0;i<n;i++){
    if (src[i] == 0) {
      dest[i] = 0;
    } else if (src[i] < 0) {
      dest[i] = -((-src[i] * quant_factor + quant_offset + 2)>>2);
    } else {
      dest[i] = (src[i] * quant_factor + quant_offset + 2)>>2;
    }
  }
}

void
schro_dequantise_s16_table (int16_t *dest, int16_t *src, int quant_index,
    schro_bool is_intra, int n)
{
  int i;

  for(i=0;i<n;i++){
    dest[i] = table[src[i] + 32768];
  }
}

#if 0
void
schro_quantise_s16_shift (int16_t *dest, int16_t *src, int quant_index,
    schro_bool is_intra, int n)
{
  if ((quant_index & 3) == 0) {
    int i;

    for(i=0;i<n;i++){
      dest[i] = __schro_quantise_shift (src[i], quant_index >> 2);
    }
  } else {
    schro_quantise_s16_ref (dest, src, quant_index, is_intra, n);
  }
}
#endif

void
schro_dequantise_s16_test (int16_t *dest, int16_t *src, int quant_index,
    schro_bool is_intra, int n)
{
  int quant_factor = schro_table_quant[quant_index];
  int quant_offset = is_intra ? schro_table_offset_1_2[quant_index] :
    schro_table_offset_3_8[quant_index];
  int i;

  quant_offset += 2;

  for(i=0;i<n;i++){
    if (src[i] == 0) {
      dest[i] = 0;
    } else if (src[i] < 0) {
      dest[i] = -((-src[i] * quant_factor + quant_offset)>>2);
    } else {
      dest[i] = (src[i] * quant_factor + quant_offset)>>2;
    }
  }
}

void
generate_table (int16_t *table, int quant_index, schro_bool is_intra)
{
  int i;
  for(i=0;i<65536;i++){
    table[i] = i-32768;
  }
  schro_dequantise_s16_ref (table, table, quant_index, is_intra, 65536);
}

void
quantise_speed (int quant_index)
{
  OilProfile prof1;
  OilProfile prof2;
  OilProfile prof3;
  double ave1;
  double ave2;
  double ave3;
  int i;
  int j;
  schro_bool is_intra = TRUE;

  oil_profile_init (&prof1);
  oil_profile_init (&prof2);
  oil_profile_init (&prof3);

  for(i=0;i<10;i++) {
    for(j=0;j<N;j++){
      a[j] = (rand () & 0xff) - 128;
    }
    generate_table (table, quant_index, is_intra);

    oil_profile_start (&prof1);
    schro_dequantise_s16_ref (b, a, quant_index, FALSE, N);
    oil_profile_stop (&prof1);
    oil_profile_start (&prof2);
    schro_dequantise_s16_table (c, a, quant_index, FALSE, N);
    oil_profile_stop (&prof2);
    oil_profile_start (&prof3);
    schro_dequantise_s16_test (c, a, quant_index, FALSE, N);
    oil_profile_stop (&prof3);

#if 0
    for(j=0;j<N;j++){
      printf("%d %d: %d %d %d\n", quant_index, j, a[j], b[j], c[j]);
      if (b[j] != c[j]) {
        fprintf(stderr, "mismatch\n");
        exit (1);
      }
    }
#endif
  }

  ave1 = oil_profile_get_min (&prof1);
  ave2 = oil_profile_get_min (&prof2);
  ave3 = oil_profile_get_min (&prof3);
  printf("%d %g %g %g\n", quant_index, ave1, ave2, ave3);
}


int
main (int argc, char *argv[])
{
  int i;

  oil_init();

  for(i=0;i<60;i++){
    quantise_speed (i);
  }

  return 0;
}

