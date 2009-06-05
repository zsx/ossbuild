/* 
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 /*********************************************************************
 * Adapted from dist10 reference code and used under the license therein:
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
 **********************************************************************/

#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#include <glib.h>
#include <gst/gst.h>

/* Accumulator optimization on bitstream management */
#define ENABLE_OPT_BS 0


/* Bit stream reader definitions */
#define         MAX_LENGTH      32   /* Maximum length of word written or
                                        read from bit stream */
#define         BS_BYTE_SIZE    8

typedef void (*bs_release_func) (void *ref);

/* list structure for each buffer in the bitstream */
typedef struct BSBuffer 
{
  guint8 *data;
  guint32 size;
        
  /* Caller Reference, passed to the free function */
  void *ref;

  /* Clock time, if any, associated with this buffer. Returned
   * from the decode for the first frame whose sync word starts in
   * this buffer
   */
  GstClockTime ts;
} BSBuffer;

typedef struct BSReader
{
  guint64     bitpos;      /* Number of bits read so far */

  guint64     buf_size;    /* Number of bytes in the buffer list */
  GList      *buflist;     /* List of data buffers to read from */

  BSBuffer   *cur;         /* Current data buffer */
  guint8     *cur_byte;    /* ptr to the current byte in the cur buffer */
  guint8      cur_bit;     /* the next bit to be used in the current byte,
                            * numbered from 8 down to 1 */
  guint       cur_used;    /* Number of bytes _completely_ consumed out of
                            * the 'cur buffer' */
} BSReader;

typedef struct Bit_stream_struc {
 bs_release_func free_cb;    /* Buffer release callback */

 BSReader    master;         /* Master tracking position, advanced
                              * by bs_consume() */
 BSReader    read;           /* Current read position, set back to the 
                              * master by bs_reset() */
} Bit_stream_struc;

/* Create and initialise a new bitstream reader */
Bit_stream_struc *bs_new ();

/* Release a bitstream reader */
void bs_free (Bit_stream_struc *bs);

/* Pass the callback to release a buffer when finished with it */
void bs_set_release_func (Bit_stream_struc *bs, bs_release_func cb);

/* Append a data buffer to the stream for processing */
gboolean bs_add_buffer (Bit_stream_struc *bs, guchar *data,
  guint32 size, void *ref, GstClockTime ts);

/* Reset the current read position to the master position */
void bs_reset (Bit_stream_struc *bs);

/* Advance the master position by Nbits (frees buffers) and 
 * reset the read ptr */
void bs_consume (Bit_stream_struc *bs, guint32 Nbits);

/* Number of bits available for reading */
#define bs_bits_avail(bs) ((guint)((bs)->read.buf_size - \
    (bs)->read.cur_used) * 8 + (bs)->read.cur_bit - 8)

/* Return the clock time associated with the current data buffer */
GstClockTime bs_buf_time (Bit_stream_struc *bs);

/* Private func to move the read pointer to the next buffer in the stream */
gboolean bs_nextbuf (Bit_stream_struc * bs, BSReader *read, gboolean release);

/* Read 1 bit from the stream */
static inline guint32 bs_get1bit (Bit_stream_struc *bs);
/* Read N bits from the stream */
static inline guint32 bs_getbits (Bit_stream_struc *bs, guint32 N);

/* Extract N bytes from the bitstream into the out array. */
void bs_getbytes (Bit_stream_struc *bs, guint8 *out, guint32 N);

/* Advance the read pointer by N bits */
void bs_skipbits (Bit_stream_struc *bs, guint32 N);

/* read the next byte aligned N bits from the bit stream */
static inline guint32 bs_getbits_aligned (Bit_stream_struc *bs, guint32 N);

/* Current bitstream position in bits */
static inline guint64 bs_pos (Bit_stream_struc *bs)
{
  return bs->master.bitpos;
}

static inline guint64 bs_read_pos(Bit_stream_struc *bs)
{
  return bs->read.bitpos;
}

gboolean bs_seek_sync (Bit_stream_struc *bs, GstClockTime *frame_ts);
void bs_flush (Bit_stream_struc *bs);

/* Huffman decoder bit buffer decls */
#define HDBB_BUFSIZE 4096

typedef struct
{
  guint avail;
  guint totbit;
  guint buf_byte_idx;
  guint buf_bit_idx;
#if ENABLE_OPT_BS
  guint remaining; 
  guint accumulator;
#endif
  guint8 *buf;
} huffdec_bitbuf;

/* Huffman Decoder bit buffer functions */
void    h_setbuf (huffdec_bitbuf *bb, guint8* buf, guint size);

#if ENABLE_OPT_BS
static inline gulong h_get1bit (huffdec_bitbuf *bb);
static inline gulong h_flushbits (huffdec_bitbuf *bb, guint N);
#else
#define h_get1bit(bb) (h_getbits ((bb), 1))
#define h_flushbits(bb,bits) (h_getbits ((bb), (bits)))
#endif

/* read N bits from the bit stream */
static inline gulong  h_getbits (huffdec_bitbuf *bb, guint N);

void    h_rewindNbits (huffdec_bitbuf *bb, guint N);
void    h_rewindNbytes (huffdec_bitbuf *bb, guint N);
static inline void h_byte_align (huffdec_bitbuf *bb);
static inline guint  h_bytes_avail (huffdec_bitbuf *bb);

/* Return the current bit stream position (in bits) */
#define h_sstell(bb) ((bb)->totbit)

static inline guint32 bs_get1bit (Bit_stream_struc *bs)
{
  /* TODO: optimise this if it turns out to be slow */
  return bs_getbits (bs, 1);
}

/* bs - bit stream structure */
/* N  - number of bits to read from the bit stream */
/* v  - output value */
static inline guint32 bs_getbits (Bit_stream_struc *bs, guint32 N)
{
  guint32 val = 0;
  gint j = N;

  g_assert (N <= MAX_LENGTH);

  while (j > 0) {
    gint tmp;
    gint k;
    gint mask;

    /* Move to the next byte if we consumed the current one */
    if (bs->read.cur && bs->read.cur_bit == 0) {
      bs->read.cur_bit = 8;
      bs->read.cur_used++;
      bs->read.cur_byte++;
    }

    /* If the current buffer is empty, move on to the next one */
    if ((bs->read.cur == NULL) || (bs->read.cur_used >= bs->read.cur->size))  {
      bs_nextbuf (bs, &bs->read, FALSE);
      if (!bs->read.cur) {
        g_warning ("Attempted to read beyond buffer\n");
        /* Return the bits we got so far */
        return val;
      }
    }
    /* Take as many bits as we can from the current byte */
    k = MIN (j, bs->read.cur_bit);

    /* We want the k bits from the current byte, starting from
     * the cur_bit. Mask out the top 'already used' bits, then shift
     * the bits we want down to the bottom */
    mask = (1 << bs->read.cur_bit) - 1;
    tmp = bs->read.cur_byte[0] & mask;

    /* Trim off the bits we're leaving for next time */
    tmp = tmp >> (bs->read.cur_bit - k);

    /* Adjust our tracking vars */
    bs->read.cur_bit -= k;
    j -= k;
    bs->read.bitpos += k;

    /* Put these bits in the right spot in the output */
    val |= tmp << j;
  }

  return val;
}

static inline guint32 bs_getbits_aligned (Bit_stream_struc *bs, guint32 N)
{
  guint32 align;

  align = bs->read.cur_bit;
  if (align != 8 && align != 0)
    bs_getbits (bs, align);

  return bs_getbits (bs, N);
}

#if ENABLE_OPT_BS
/* This optimizazion assumes that N will be lesser than 32 */
static inline gulong
h_getbits (huffdec_bitbuf *bb, guint N)
{
  gulong val = 0;
//  g_printf("h_getbits in:N=%d, bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d)\n", N, bb->buf_bit_idx, bb->accumulator, bb->remaining);
 
  if (N == 0)
    return 0;

  bb->totbit += N;  
      
  /* Most common case will be when accumulatior has enough bits */
  if (G_LIKELY(N <= bb->buf_bit_idx)) {
    /* first reduce buf_bit_idx by the number of bits that are taken */
    bb->buf_bit_idx -= N;      
    /* Displace to right and mask to extract the desired number of bits */
    val = (bb->accumulator >> bb->buf_bit_idx) & ((guint)0xffffffff)>>(32-N);
//    g_printf("h_getbits case 0 out:N=%d, bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d, val=%8x)\n", N, bb->buf_bit_idx, bb->accumulator, bb->remaining, val);
    return (val);
  }
  
  /* Next cases will be when there's not enough data on the accumulator and there's 
    atleast 4 bytes in the */
  if (bb->remaining >= 4) {
    /* First take all remaining bits */
    if (bb->buf_bit_idx>0)
      val = (bb->accumulator & ((guint)0xffffffff)>>(32-bb->buf_bit_idx));
    /* calculate how many more bits are required */
    N-=bb->buf_bit_idx;
    /* relad the accumulator */
    bb->buf_bit_idx = 32 - N; /* subtract the remaining required bits */
    bb->remaining -= 4;

    /* we need reverse the byte order */
#if defined(HAVE_CPU_I386)
    register int tmp = *((guint*)(bb->buf + bb->buf_byte_idx));
    __asm__  ( "bswap %0 \n\t" :  "=r" (tmp) : "0" (tmp) );                
    bb->accumulator = tmp;
#else    
    bb->accumulator = (guint)bb->buf[bb->buf_byte_idx + 3];
    bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 2]) << 8;
    bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 1]) << 16;  
    bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 0]) << 24;  
#endif
    bb->buf_byte_idx+=4;

    val = (val << N) | ((bb->accumulator >> bb->buf_bit_idx) & ((guint)0xffffffff)>>(32-N));
//    g_printf("h_getbits case 1 out:N=%d, bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d val=%8x)\n", N, bb->buf_bit_idx, bb->accumulator, bb->remaining, val);
    return (val);
  } 
  
  /* Next case when remains less that one word on the buffer */
  if (bb->remaining > 0) { 
    /* First take all remaining bits */
    if (bb->buf_bit_idx>0)    
      val = (bb->accumulator & ((guint)0xffffffff)>>(32-bb->buf_bit_idx));
    /* calculate how many more bits are required */
    N-=bb->buf_bit_idx;
    /* relad the accumulator */
    bb->buf_bit_idx = (bb->remaining * 8) - N; /* subtract the remaining required bits */
     
    bb->accumulator = 0; 
    /* load remaining bytes into the accumulator in the right order */
    for (; bb->remaining > 0; bb->remaining--)
      bb->accumulator = (bb->accumulator << 8) | (guint)bb->buf[bb->buf_byte_idx++];

    val = (val << N) | ((bb->accumulator >> bb->buf_bit_idx) & ((guint)0xffffffff)>>(32-N));
//    g_printf("h_getbits case 2 out:N=%d, bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d val=%8x)\n", N, bb->buf_bit_idx, bb->accumulator, bb->remaining, val);    
    return (val);
  } 
  
  return 0;
}

static inline gulong
h_get1bit (huffdec_bitbuf *bb)
{
  gulong val = 0;
 
  bb->totbit++;  
//  g_printf("h_get1bit  bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d)\n", bb->buf_bit_idx, bb->accumulator, bb->remaining);      
  /* Most common case will be when accumulatior has enough bits */
  if (G_LIKELY(1 <= bb->buf_bit_idx)) {
    /* first reduce buf_bit_idx by the number of bits that are taken */
    bb->buf_bit_idx--;      
    /* Displace to right and mask to extract the desired number of bits */
    val = (bb->accumulator >> bb->buf_bit_idx) & ((guint)0x1);
//    g_printf("h_get1bit case 0 out: bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d), val=%8x\n", bb->buf_bit_idx, bb->accumulator, bb->remaining, val);      
    return (val);
  }
  
  /* Next cases will be when there's not enough data on the accumulator and there's 
    atleast 4 bytes in the */
  if (bb->remaining >= 4) {
    /* relad the accumulator */
    bb->buf_bit_idx = 31; /* subtract 1 bit */
    bb->remaining -= 4;

    /* we need reverse the byte order */
#if defined(HAVE_CPU_I386)
    register int tmp = *((guint*)(bb->buf + bb->buf_byte_idx));
    __asm__  ( "bswap %0 \n\t" :  "=r" (tmp) : "0" (tmp) );                
    bb->accumulator = tmp;
#else    
    bb->accumulator = (guint)bb->buf[bb->buf_byte_idx + 3];
    bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 2]) << 8;
    bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 1]) << 16;  
    bb->accumulator |= (guint)(bb->buf[bb->buf_byte_idx + 0]) << 24;  
#endif
    bb->buf_byte_idx+=4;

    val = (bb->accumulator >> bb->buf_bit_idx) & ((guint)0x1);
//    g_printf("h_get1bit case 1 out: bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d), val=%8x\n", bb->buf_bit_idx, bb->accumulator, bb->remaining, val);      
    return (val);
  } 
  
  /* Next case when remains less that one word on the buffer */
  if (bb->remaining > 0) { 
    /* relad the accumulator */
    bb->buf_bit_idx = (bb->remaining * 8) - 1; /* subtract 1 bit  */
     
    bb->accumulator = 0; 
    /* load remaining bytes into the accumulator in the right order */
    for (; bb->remaining > 0; bb->remaining--)
      bb->accumulator = (bb->accumulator << 8) | (guint)bb->buf[bb->buf_byte_idx++];

    val = (bb->accumulator >> bb->buf_bit_idx) & ((guint)0x1);
//    g_printf("h_get1bit case 2 out: bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d), val=%8x\n", bb->buf_bit_idx, bb->accumulator, bb->remaining, val);      
    return (val);
  } 
  
  return 0;
}

static inline gulong
h_flushbits (huffdec_bitbuf *bb, guint N)
{
  guint bits = N % 8;
  guint bytes = N / 8;
  guint acc_bytes;

//  g_printf("h_flushbits in:N=%d, bits=%d, bytes=%d, bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d)\n", N, bits, bytes, bb->buf_bit_idx, bb->accumulator, bb->remaining);
  bb->totbit += N;
  
  if (bb->buf_bit_idx >= bits)
    bb->buf_bit_idx -= bits; 
  
  acc_bytes = bb->buf_bit_idx / 8;
  
  if (acc_bytes >= bytes)
    bb->buf_bit_idx -= bytes * 8;
  else {
    bb->buf_bit_idx -= acc_bytes * 8;
    bytes -= acc_bytes;
    if (bytes <= bb->remaining) {
      bytes--;
      bits = 8 - bb->buf_bit_idx;
      if (bytes > 0) {
        bb->buf += bytes;
        bb->remaining -= bytes;  
      }
      bb->buf_bit_idx = 0;
      bb->totbit -= bits;
      h_getbits (bb, bits);
    } else {
      bb->remaining = 0;
      bb->buf_bit_idx = 0;
    }
  }
//  g_printf("h_flushbits out:N=%d, bits=%d, bytes=%d, bb->buf_bit_idx=%d, bb->accumulator=%8x, bb->remaining=%d)\n", N, bits, bytes, bb->buf_bit_idx, bb->accumulator, bb->remaining);  
}

#else
static inline gulong
h_getbits (huffdec_bitbuf *bb, guint N)
{
  gulong val = 0;
  guint j = N;
  guint k, tmp;
  guint mask;

  bb->totbit += N;
  while (j > 0) {
    if (!bb->buf_bit_idx) {
      bb->buf_bit_idx = 8;
      bb->buf_byte_idx++;
      if (bb->buf_byte_idx > bb->avail) {
        return 0;
      }
    }
    k = MIN (j, bb->buf_bit_idx);

    mask = (1 << (bb->buf_bit_idx)) - 1;
    tmp = bb->buf[bb->buf_byte_idx % HDBB_BUFSIZE] & mask;
    tmp = tmp >> (bb->buf_bit_idx - k);
    val |= tmp << (j - k);
    bb->buf_bit_idx -= k;
    j -= k;
  }
  return (val);
}
#endif
#if 0
/* Replaced by a macro call to h_getbits */
guint
h_get1bit (huffdec_bitbuf *bb)
{
  return hgetbits (bb, 1);
}
#endif

/* If not on a byte boundary, skip remaining bits in this byte */
static inline void h_byte_align (huffdec_bitbuf *bb)
{
#if ENABLE_OPT_BS
//  g_printf("h_byte_align, in(bb->buf_bit_idx)=%d", bb->buf_bit_idx);
  if ((bb->buf_bit_idx % 8) != 0) {
    bb->buf_bit_idx -= (bb->buf_bit_idx % 8);
  }
//  g_printf(", out(bb->buf_bit_idx)=%d\n", bb->buf_bit_idx);  
#else
  /* If buf_bit_idx == 0 or == 8 then we're already byte aligned. Check by looking at the
   * bottom bits of the byte */
  if (bb->buf_byte_idx <= bb->avail && (bb->buf_bit_idx & 0x07) != 0) {
    bb->buf_bit_idx = 8;
    bb->buf_byte_idx++;
  }
#endif  
}

static inline guint h_bytes_avail (huffdec_bitbuf *bb)
{
  if (bb->avail >= bb->buf_byte_idx) {
    if (bb->buf_bit_idx != 8)
      return bb->avail - bb->buf_byte_idx - 1;
    else
      return bb->avail - bb->buf_byte_idx;
  }

  return 0;
}

#endif
