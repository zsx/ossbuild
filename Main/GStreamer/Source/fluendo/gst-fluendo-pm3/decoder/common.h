/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 /*********************************************************************
 * Adapted from dist10 reference code and used under the license therein:
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
 **********************************************************************/

#ifndef COMMON_H
#define COMMON_H

/***********************************************************************
*
*  Global Conditional Compile Switches
*
***********************************************************************/

#define SYNC_WORD_LNGTH G_GUINT64_CONSTANT(11)
#define HEADER_LNGTH 21
#define SYNC_WORD ((guint32)(0x7ff))

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "bitstream.h"

/***********************************************************************
*
*  Global Definitions
*
***********************************************************************/

#ifdef __GNUC__
#define ATTR_UNUSED __attribute__ ((unused))
#else
#define ATTR_UNUSED 
#endif

/* General Definitions */

#define         FLOAT                  float 
#define         NULL_CHAR               '\0'

#ifndef PI
#define         PI                      3.141592653589793115997963468544185161590576171875
#endif

#define         PI4                     PI/4
#define         PI64                    PI/64
#define         LN_TO_LOG10             0.2302585093

#define         VOL_REF_NUM             0
#define         MAC_WINDOW_SIZE         24

#define         MONO                    1
#define         STEREO                  2
#define         WORD                    16
#define         MAX_NAME_SIZE           81

/* Maximum samples per sub-band */
#define         SSLIMIT                 18
/* Maximum number of sub-bands */
#define         SBLIMIT                 32

#define         FFT_SIZE                1024
#define         HAN_SIZE                512
#define         SCALE_BLOCK             12
#define         SCALE_RANGE             64
#define         SCALE                   32768
#define         CRC16_POLYNOMIAL        0x8005

/* MPEG Header Definitions - ID Bit Values */
#define         MPEG_VERSION_1          0x03
#define         MPEG_VERSION_2          0x02
#define         MPEG_VERSION_2_5        0x00

/* MPEG Header Definitions - Mode Values */
#define         MPG_MD_STEREO           0
#define         MPG_MD_JOINT_STEREO     1
#define         MPG_MD_DUAL_CHANNEL     2
#define         MPG_MD_MONO             3

/* Mode Extension */
#define         MPG_MD_LR_LR             0
#define         MPG_MD_LR_I              1
#define         MPG_MD_MS_LR             2
#define         MPG_MD_MS_I              3

/***********************************************************************
*
*  Global Type Definitions
*
***********************************************************************/

/* Structure for Reading Layer II Allocation Tables from File */
typedef struct {
    unsigned int    steps;
    unsigned int    bits;
    unsigned int    group;
    unsigned int    quant;
} sb_alloc;

typedef sb_alloc al_table[SBLIMIT][16];

/* Header Information Structure */
typedef struct {
  /* Stuff read straight from the MPEG header */
  guint    version;
  guint    layer;
  gboolean error_protection;

  gint     bitrate_idx;   /* Index into the bitrate tables */
  guint    srate_idx;     /* Index into the sample rate table */

  gboolean padding;
  gboolean extension;
  guint    mode;
  guint    mode_ext;
  gboolean copyright;
  gboolean original;
  guint    emphasis;

  /* Derived attributes */
  guint    bitrate;       /* Bitrate of the frame, kbps */
  guint    sample_rate;   /* sample rate in Hz */
  guint    sample_size;   /* in bits */
  guint    frame_samples; /* Number of samples per channels in this
                             frame */
  guint    channels;      /* Number of channels in the frame */

  guint    bits_per_slot; /* Number of bits per slot */
  guint    frame_slots;   /* Total number of data slots in this frame */
  guint    main_slots;    /* Slots of main data in this frame */
  guint    frame_bits;    /* Number of bits in the frame, including header
                             and sync word */
  guint    side_info_slots; /* Number of slots of side info in the frame */
} fr_header;

/* Layer III side information. */
typedef struct {
  guint part2_3_length;
  guint big_values;
  guint global_gain;
  guint scalefac_compress;
  guint window_switching_flag;
  guint block_type;
  guint mixed_block_flag;
  guint table_select[3];
  guint subblock_gain[3];
  guint region0_count;
  guint region1_count;
  guint preflag;
  guint scalefac_scale;
  guint count1table_select;
} gr_info_t ;

typedef struct {
  guint main_data_begin;
  guint private_bits;
  guint scfsi[4][2];  /* [band?][channel] */
  gr_info_t gr[2][2]; /* [group][channel] */
} III_side_info_t;

/* Layer III scale factors. */
typedef struct {
  int l[22];            /* [cb] */
  int s[3][13];         /* [window][cb] */
} III_scalefac_t[2];  /* [ch] */

/* top level structure for frame decoding */
typedef struct {
    fr_header   header;         /* raw header information */
    int         actual_mode;    /* when writing IS, may forget if 0 chs */
    int         stereo;         /* 1 for mono, 2 for stereo */
    int         jsbound;        /* first band of joint stereo coding */
    int         sblimit;        /* total number of sub bands */
    const al_table *alloc;      /* bit allocation table for the frame */
    
    /* Synthesis workspace */
    float   filter[64][32];
    float   synbuf[2][2 * HAN_SIZE];
    gint    bufOffset[2];

    /* scale data */
    guint scalefac_buffer[54];
} frame_params;

/***********************************************************************
*
*  Global Variable External Declarations
*
***********************************************************************/
extern const gint      s_rates[4][4];      /* Array of sample rates */
extern const gint      bitrates_v1[3][15]; /* Array of bitrates for 11172-3 */
extern const gint      bitrates_v2[3][15]; /* Array of bitrates for 13818-3 */

/***********************************************************************
*
*  Global Function Prototype Declarations
*
***********************************************************************/

/* Set up the frame params from the header */
void    hdr_to_frps (frame_params *fr_ps);
void    I_CRC_calc (const frame_params *fr_ps, 
            guint bit_alloc[2][SBLIMIT], guint *crc);
void    II_CRC_calc (const frame_params *fr_ps, 
            guint bit_alloc[2][SBLIMIT], guint scfsi[2][SBLIMIT], 
            guint *crc);
void    II_pick_table (frame_params *fr_ps);

void    update_CRC (const guint data, const guint length, guint *crc);

#endif

