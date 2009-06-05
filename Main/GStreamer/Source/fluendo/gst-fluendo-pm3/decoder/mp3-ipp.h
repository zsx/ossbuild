/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 * Portions of this code are Copyright 2003-2005 Intel Corporation:
 *
 *                  INTEL CORPORATION PROPRIETARY INFORMATION
 *     This software is supplied under the terms of a license agreement or
 *     nondisclosure agreement with Intel Corporation and may not be copied
 *     or disclosed except in accordance with the terms of that agreement.
 *     Copyright(c) 2003-2005 Intel Corporation. All Rights Reserved.
 *
 */
#ifndef __MP3TL_IPP_H__
#define __MP3TL_IPP_H__

#include <ippdefs.h>
#include <ippcore.h>
#include <ipps.h>
#include <ippac.h>

#define MAX_GRAN           2       /* Maximum number of granules per channel */
#define MAX_CHAN           2       /* Maximum number of channels */
#define SCF_BANDS          4       /* Number of scalefactor bands per channel */
#define MAIN_DATA_BUF_SIZE 4096
#define PQMF_BANDS         32  /* Number of channels in the synthesis PQMF bank */
#define IMDCT_BLK_LEN      18  /* Long-block IMDCT length */

typedef struct mp3ipp_info mp3ipp_info;

struct mp3ipp_info {
  /* Unpacked per channel/granule side info */
  IppMP3SideInfo    SideInfo[MAX_GRAN][MAX_CHAN];
  
  Ipp8s             ScaleFactor[MAX_CHAN*IPP_MP3_SF_BUF_LEN];
  /* Scalefactor select information - 4 scalefactor bands per channel */
  int               Scfsi[MAX_CHAN*SCF_BANDS];

  /* Buffer used first for Huffman symbols (Is), 
   * then requantized IMDCT inputs (Xr) */   
  Ipp32s            IsXr[MAX_CHAN*IPP_MP3_GRANULE_LEN];
  /* Non-zero bounds on Huffman IMDCT coefficient set for each channel */
  int               NonZeroBound[MAX_CHAN];             

  /* Work space buffer required by requantization primitive */   
  Ipp32s            RequantBuf[IPP_MP3_GRANULE_LEN];
  /* Buffer used for IMDCT outputs/PQMF synthesis inputs */
  Ipp32s            Xs[MAX_CHAN*IPP_MP3_GRANULE_LEN];
  /* Overlap-add buffer required by IMDCT primitive */
  Ipp32s            OverlapAddBuf[MAX_CHAN*IPP_MP3_GRANULE_LEN]; 
  /* Number of IMDCTs computed on previous granule/frame */
  int               PreviousIMDCT[MAX_CHAN];

  /* "V" buffer - used by fast DCT-based algorithm for synthesis PQMF bank */
  Ipp32s            PQMF_V_Buf[MAX_CHAN*IPP_MP3_V_BUF_LEN]; 
  /* Index used by the PQMF for internal maintainence of the "V" buffer */
  int               PQMF_V_Indx[MAX_CHAN];

  /* Decoding buffer for "Main Data" portion of the MP3 stream. */
  Ipp8u             MainDataBuf[MAIN_DATA_BUF_SIZE];
  /* Main Data buffer end pointer */
  int               MainDataEnd;                  

  /* Number of channels */
  int               Channels;
  /* Length of pcm output buffer */
  int               pcmLen;                     

  /* Layer 1/2 state info */
  float filter_bfr[2][16][64];
  float smpl_sb[2][576];
  short m_even[2];
  short m_ptr[2][2];
  guchar *mdct_buffer;  
  IppsDCTFwdSpec_32f  *pDCTSpecFB;
};

gboolean mp3_ipp_init (mp3tl *tl);
void mp3_ipp_flush (mp3tl *tl);
void mp3_ipp_close (mp3tl *tl);

Mp3TlRetcode ipp_decode_mp3 (mp3tl *tl);

#endif
