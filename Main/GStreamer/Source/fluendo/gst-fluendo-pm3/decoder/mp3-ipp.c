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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gst/gst.h>

#include "mp3tl.h"
#include "mp3-ipp.h"
#include "mp3tl-priv.h"
#include "table-dewindow.h"
#include "table-dewindow-ipp.h"

GST_DEBUG_CATEGORY_EXTERN (flump3debug);
#define GST_CAT_DEFAULT flump3debug

#if 0
Mp3TlRetcode
decode_mp2 (mp3tl * tl)
{
  for (j = k = 0; k < IMDCT_BLK_LEN; k++, j += (PQMF_BANDS * channels)) {
    status = ippsSynthPQMF_MP3_32s16s (&(Xs[(k * PQMF_BANDS) + offset]),
        (Ipp16s *) & (tl->sample_buf[j + ch + pcmblock]),
        &(PQMF_V_Buf[ch * IPP_MP3_V_BUF_LEN]), &(PQMF_V_Indx[ch]), channels);

    if (ippStsNoErr != status) {
      GST_DEBUG ("ippsSynthPQMF_MP3_32s16s() failed: %d [%s]\n",
          status, ippGetStatusString (status));
      return MP3TL_ERR_STREAM;
    }
  }
}
#endif

Mp3TlRetcode
ipp_decode_mp3 (mp3tl * tl)
{
#if 0
  int nSlots;
#endif
  int gr, ch, j, k;
  gboolean MainDataOK;
  fr_header *hdr;
  IppStatus status;
  Ipp8u hdrBuf[64];
  Ipp8u *pHdrBuf = hdrBuf;
  int MainDataBegin;
  int NextMainDataBit = 0;
  int HuffmanLength;
  Ipp8u *FirstSclFacByte;
  int FirstSclFacBit;
  int SclFacLength;
  int PrivateBits;
  int offset;
  int pcmblock;
  gint channels;
  Ipp8s ScaleFactor[MAX_CHAN * IPP_MP3_SF_BUF_LEN];
  int *MainDataEnd = &(tl->ipp.MainDataEnd);
  Ipp8u *MainDataBuf = tl->ipp.MainDataBuf;
  Ipp8u *MainDataPtr = tl->ipp.MainDataBuf;
  IppMP3SideInfo *SideInfoPtr = (IppMP3SideInfo *) tl->ipp.SideInfo;
  Ipp32s *IsXr = tl->ipp.IsXr;
  int *NonZeroBound = tl->ipp.NonZeroBound;
  Ipp32s *RequantBuf = tl->ipp.RequantBuf;
  Ipp32s *Xs = tl->ipp.Xs;
  Ipp32s *OverlapAddBuf = tl->ipp.OverlapAddBuf;
  int *PreviousIMDCT = tl->ipp.PreviousIMDCT;
  Ipp32s *PQMF_V_Buf = tl->ipp.PQMF_V_Buf;
  int *PQMF_V_Indx = tl->ipp.PQMF_V_Indx;
  IppMP3FrameHeader frameHdr;

  hdr = &tl->fr_ps.header;

  if (hdr->version == MPEG_VERSION_1)
    frameHdr.id = 1;
  else
    frameHdr.id = 0;

  frameHdr.layer = 4 - hdr->layer;
  frameHdr.protectionBit = hdr->error_protection;
  frameHdr.bitRate = hdr->bitrate_idx;
  frameHdr.samplingFreq = hdr->srate_idx;
  frameHdr.paddingBit = hdr->padding;
  frameHdr.privateBit = hdr->extension;
  frameHdr.mode = hdr->mode;
  frameHdr.modeExt = hdr->mode_ext;
  frameHdr.copyright = hdr->copyright;
  frameHdr.originalCopy = hdr->original;
  frameHdr.emphasis = hdr->emphasis;
  frameHdr.CRCWord = tl->old_crc;

  /* Check enough side_info data available */
  if (bs_bits_avail (tl->bs) < hdr->side_info_slots)
    return MP3TL_ERR_NEED_DATA;

  /* Copy side info into buffer for decode */
#if 0
  for (nSlots = 0; nSlots < hdr->side_info_slots; nSlots++)
    hdrBuf[nSlots] = bs_getbits (tl->bs, 8);
#else
  bs_getbytes (tl->bs, hdrBuf, hdr->side_info_slots);
#endif

  status = ippsUnpackSideInfo_MP3 (&pHdrBuf,
      SideInfoPtr, &MainDataBegin, &PrivateBits, tl->ipp.Scfsi, &frameHdr);

  if (ippStsNoErr != status) {
    GST_DEBUG ("Could not unpack layer 3 side info");
    return MP3TL_ERR_BAD_FRAME;
  }

  /* Check enough main_data available */
  if (bs_bits_avail (tl->bs) < hdr->main_slots * 8)
    return MP3TL_ERR_NEED_DATA;

  /* Verify that sufficient main_data was extracted from */
  /* the previous sync interval */
  MainDataOK = ((*MainDataEnd - MainDataBegin) >= 0);

  /* If so, copy the main data left over from previous sync interval */
  /* to start of main_data buffer. Note that MainDataBegin is the negative */
  /* offset pointer (side info) described in ISO/IEC 11172-3 */
  if (MainDataOK) {
    if ((*MainDataEnd - MainDataBegin) > 0) {
      memmove (MainDataBuf, MainDataBuf + (*MainDataEnd - MainDataBegin),
          MainDataBegin);
      *MainDataEnd = MainDataBegin;
    }
  }

  /* Append main data from current frame to the bit reservoir */
#if 0
  for (nSlots = hdr->main_slots; nSlots > 0; nSlots--)
    MainDataBuf[(*MainDataEnd)++] = bs_getbits (tl->bs, 8);
#else
  bs_getbytes (tl->bs, MainDataBuf + tl->ipp.MainDataEnd, hdr->main_slots);
  tl->ipp.MainDataEnd += hdr->main_slots;
#endif

  if (!MainDataOK) {
    GST_DEBUG ("Bad frame - not enough main data bits");
    return MP3TL_ERR_BAD_FRAME;
  }

  channels = hdr->channels;

  /* Decode the frame granules and channels, */
  /* starting with scalefactors and Huffman symbols */
  /* Granule loop:  Decode 1 or 2 channels for each 576-sample granule */
  for (gr = 0; gr < tl->n_granules; gr++) {
    /* Channel loop, part A:  Decode scalefactors and Huffman symbols */
    for (ch = 0; ch < channels; ch++) {
      /* Point to the correct side information, */
      /* given the number of channels; */
      /* the table of side information structures */
      /* returned by ippsUnpackSideInfo_MP3 is organized as follows: */
      /* index  one channel   two channels */
      /*  ------------------------------------- */
      /*  0 0   gr0       gr0/ch0 */
      /*  0 1   gr1       gr0/ch1 */
      /*  1 0   empty     gr1/ch0 */
      /*  1 1   empty     gr1/ch1 */

      if (hdr->channels == 2)
        SideInfoPtr = &(tl->ipp.SideInfo[gr][ch]);
      else
        SideInfoPtr = &(tl->ipp.SideInfo[0][gr]);

      /* Unpack scalefactors and measure legnth */
      /* of scalefactor data block in the bitstream */

      FirstSclFacByte = MainDataPtr;
      FirstSclFacBit = NextMainDataBit;

      /* IPP MP3 decoder primitive #3: */
      /* Unpack scalefactors for one channel of one granule */
      status = ippsUnpackScaleFactors_MP3_1u8s (&MainDataPtr,
          &NextMainDataBit,
          &(ScaleFactor[ch * IPP_MP3_SF_BUF_LEN]),
          SideInfoPtr, &(tl->ipp.Scfsi[ch * 4]), &frameHdr, gr, ch);

      if (ippStsNoErr != status) {
        GST_DEBUG ("Could not unpack layer 3 scale factors");
        return MP3TL_ERR_BAD_FRAME;
      }

      /* IPP MP3 decoder primitive #4: */
      /* Decode Huffman symbols for one channel of one granule */
      /* Measure length of scalefactor block, in bits */

      SclFacLength = 8 * (MainDataPtr - FirstSclFacByte) -
          FirstSclFacBit + NextMainDataBit;

      /* Compute Huffman data length given scalefactor block length */
      /* and part23len from side info */
      HuffmanLength = SideInfoPtr->part23Len - SclFacLength;

      /* Invoke the Huffman unpacking primitive */
      status = ippsHuffmanDecode_MP3_1u32s (&MainDataPtr,
          &NextMainDataBit,
          &(IsXr[ch * IPP_MP3_GRANULE_LEN]),
          &(NonZeroBound[ch]), SideInfoPtr, &frameHdr, HuffmanLength);

      if (ippStsNoErr != status) {
        GST_DEBUG ("ippsHuffmanDecode_MP3_1u32s() failed: %d [%s]\n",
            status, ippGetStatusString (status));
        return MP3TL_ERR_BAD_FRAME;
      }
    }                           /* end of the channel loop, part A */

    /* IPP MP3 decoder primitive #5: */
    /* Requantize Huffman symbols for all channels of current granule */
    if (channels == 2)
      SideInfoPtr = &(tl->ipp.SideInfo[gr][0]);
    else
      SideInfoPtr = &(tl->ipp.SideInfo[0][gr]);

    status = ippsReQuantize_MP3_32s_I (IsXr,
        NonZeroBound, ScaleFactor, SideInfoPtr, &frameHdr, RequantBuf);

    if (ippStsNoErr != status) {
      GST_DEBUG ("ippsReQuantize_MP3_32s_I() failed: %d [%s]\n",
          status, ippGetStatusString (status));
      return MP3TL_ERR_STREAM;
    }

    /* Channel loop, part B:  */
    /* apply hybrid synthesis filter bank on both channels */
    /* of current granule */
    for (ch = 0; ch < channels; ch++) {
      /* Select the correct side information, */
      /* given the number of channels (see explanation above) */

      if (channels == 2)
        SideInfoPtr = &(tl->ipp.SideInfo[gr][ch]);
      else
        SideInfoPtr = &(tl->ipp.SideInfo[0][gr]);

      /* IPP MP3 decoder primitive #6: */
      /* Apply IMDCT (first stage of the hybrid filter bank) */

      status = ippsMDCTInv_MP3_32s (&(IsXr[ch * IPP_MP3_GRANULE_LEN]),
          &(Xs[ch * IPP_MP3_GRANULE_LEN]),
          &(OverlapAddBuf[ch * IPP_MP3_GRANULE_LEN]),
          NonZeroBound[ch],
          &(PreviousIMDCT[ch]),
          SideInfoPtr->blockType, SideInfoPtr->mixedBlock);

      if (ippStsNoErr != status) {
        GST_DEBUG ("ippsMDCTInv_MP3_32s() failed: %d [%s]\n",
            status, ippGetStatusString (status));
        return MP3TL_ERR_STREAM;
      }

      offset = ch * IPP_MP3_GRANULE_LEN;
      pcmblock = gr * IPP_MP3_GRANULE_LEN * channels;

      for (j = k = 0; k < IMDCT_BLK_LEN; k++, j += (PQMF_BANDS * channels)) {
        status = ippsSynthPQMF_MP3_32s16s (&(Xs[(k * PQMF_BANDS) + offset]),
            (Ipp16s *) & (tl->sample_buf[j + ch + pcmblock]),
            &(PQMF_V_Buf[ch * IPP_MP3_V_BUF_LEN]),
            &(PQMF_V_Indx[ch]), channels);

        if (ippStsNoErr != status) {
          GST_DEBUG ("ippsSynthPQMF_MP3_32s16s() failed: %d [%s]\n",
              status, ippGetStatusString (status));
          return MP3TL_ERR_STREAM;
        }
      }
    }                           /* end of the channel loop, part B */
  }                             /* end of the granule processing loop */

  return MP3TL_ERR_OK;
}

#if 1                           /* Define to use IPP optimised synth for layers 1 and 2 */
void
mp3_SubBandSynthesis (mp3tl * tl, frame_params * fr_ps,
    float *bandPtr, gint ch, short *outSamples)
{
  int i, odd, odd_idx, idx, even_idx, evn;

  const float *wnd_ptr_odd, *wnd_ptr_even;
  float *flt_ptr_even, *flt_ptr_odd;

  float *out = tl->ipp.smpl_sb[ch];
  float *samples = bandPtr;
  float (*filter_bfr)[16][64] = tl->ipp.filter_bfr;
  short *m_even = tl->ipp.m_even;
  short (*m_ptr)[2] = tl->ipp.m_ptr;

  // float *tmp_bfr = tl->ipp.tmp_bfr;
  float tmp_bfr[32];

  m_even[ch] = (++m_even[ch] & 0x1);

  m_ptr[ch][m_even[ch]] = (m_ptr[ch][m_even[ch]] + 7) & 0x7;

  ippsDCTFwd_32f (samples, tmp_bfr, tl->ipp.pDCTSpecFB, tl->ipp.mdct_buffer);
  ippsMulC_32f_I (4, tmp_bfr, 32);
  tmp_bfr[0] *= 1.41421356f;

  idx = m_even[ch] * 8 + m_ptr[ch][m_even[ch]];

  for (i = 0; i < 16; i++) {
    filter_bfr[ch][idx][i] = tmp_bfr[i + 16];
  }
  filter_bfr[ch][idx][16] = 0;
  for (i = 17; i < 48; i++) {
    filter_bfr[ch][idx][i] = -tmp_bfr[48 - i];
  }
  for (i = 48; i < 64; i++) {
    filter_bfr[ch][idx][i] = -tmp_bfr[i - 48];
  }

  // calculate variables
  odd = ((m_even[ch] + 1) & 0x1);
  odd_idx = odd << 3;
  odd = (8 - m_ptr[ch][odd]) & 7;

  evn = (8 - m_ptr[ch][m_even[ch]]) & 7;
  even_idx = m_even[ch] << 3;

  wnd_ptr_even = mp3dec_Dcoef[0] + evn;
  wnd_ptr_odd = mp3dec_Dcoef[0] + odd + 15;

  flt_ptr_even = &filter_bfr[ch][even_idx][0];
  flt_ptr_odd = &filter_bfr[ch][odd_idx][32];

  // calculate samples
  for (i = 0; i < 32; i++) {
    out[i] =
        flt_ptr_even[0 << 6] * wnd_ptr_even[0] +
        flt_ptr_even[1 << 6] * wnd_ptr_even[1] +
        flt_ptr_even[2 << 6] * wnd_ptr_even[2] +
        flt_ptr_even[3 << 6] * wnd_ptr_even[3] +
        flt_ptr_even[4 << 6] * wnd_ptr_even[4] +
        flt_ptr_even[5 << 6] * wnd_ptr_even[5] +
        flt_ptr_even[6 << 6] * wnd_ptr_even[6] +
        flt_ptr_even[7 << 6] * wnd_ptr_even[7] +
        flt_ptr_odd[0 << 6] * wnd_ptr_odd[0] +
        flt_ptr_odd[1 << 6] * wnd_ptr_odd[1] +
        flt_ptr_odd[2 << 6] * wnd_ptr_odd[2] +
        flt_ptr_odd[3 << 6] * wnd_ptr_odd[3] +
        flt_ptr_odd[4 << 6] * wnd_ptr_odd[4] +
        flt_ptr_odd[5 << 6] * wnd_ptr_odd[5] +
        flt_ptr_odd[6 << 6] * wnd_ptr_odd[6] +
        flt_ptr_odd[7 << 6] * wnd_ptr_odd[7];

    flt_ptr_even++;
    flt_ptr_odd++;

    wnd_ptr_even += 30;
    wnd_ptr_odd += 30;
  }

  ippsConvert_32f16s_Sfs (out, outSamples, 32, ippRndNear, -15);
}

#else
void
mp3_SubBandSynthesis (mp3tl * tl, frame_params * fr_ps,
    gfloat * bandPtr, gint channel, short *samples)
{
  int i, j, k;
  float *bufOffsetPtr;
  float sum;
  gint *bufOffset;
  float *synbuf = fr_ps->synbuf[channel];
  gfloat tmp[32];

  bufOffset = fr_ps->bufOffset;

  bufOffset[channel] = (bufOffset[channel] - 64) & 0x3ff;

  bufOffsetPtr = synbuf + bufOffset[channel];

  for (i = 0; i < 64; i++) {
    gfloat *f_row = fr_ps->filter[i];

    ippsMul_32f (bandPtr, f_row, tmp, 32);
    ippsSum_32f (tmp, 32, &sum, ippAlgHintAccurate);

    bufOffsetPtr[i] = sum;
  }

  /*  S(i,j) = D(j+32i) * U(j+32i+((i+1)>>1)*64)  */
  /*  samples(i,j) = MWindow(j+32i) * bufPtr(j+32i+((i+1)>>1)*64)  */
  for (j = 0; j < 32; j++) {
    sum = dewindow[j] * synbuf[(j + bufOffset[channel]) & 0x3ff];

    for (i = 64; i < 512; i += 64) {
      k = j + i;
      sum += dewindow[k] * synbuf[(k + i + bufOffset[channel]) & 0x3ff];
      sum +=
          dewindow[k - 32] * synbuf[(k - 32 + i + bufOffset[channel]) & 0x3ff];
    }

    tmp[j] = sum;
  }

  ippsConvert_32f16s_Sfs (tmp, samples, 32, ippRndNear, -15);
  return 0;
}
#endif

gboolean
mp3_ipp_init (mp3tl * tl)
{
  int size_dct;

  memset (&tl->ipp, 0, sizeof (tl->ipp));
  if (ippStaticInit () < ippStsNoErr) {
    GST_DEBUG ("Failed to initialise IPP");
    return FALSE;
  }

  ippsDCTFwdInitAlloc_32f (&(tl->ipp.pDCTSpecFB), 32, ippAlgHintNone);
  ippsDCTFwdGetBufSize_32f (tl->ipp.pDCTSpecFB, &size_dct);

  tl->ipp.mdct_buffer = ippsMalloc_8u (size_dct);
  return TRUE;
}

void
mp3_ipp_flush (mp3tl * tl)
{
  tl->ipp.MainDataEnd = 0;
}

void
mp3_ipp_close (mp3tl * tl)
{
  if (tl->ipp.pDCTSpecFB) {
    ippsDCTFwdFree_32f (tl->ipp.pDCTSpecFB);
    tl->ipp.pDCTSpecFB = NULL;
  }
  if (tl->ipp.mdct_buffer) {
    ippsFree (tl->ipp.mdct_buffer);
    tl->ipp.mdct_buffer = NULL;
  }
}
