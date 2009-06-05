/* Copyright 2006, 2007, 2008 Fluendo S.A. 
 * Authors: Jan Schmidt <jan@fluendo.com>
 *          Kapil Agrawal <kapil@fluendo.com>
 *          Julien Moutte <julien@fluendo.com>
 * See the COPYING file in the top-level directory.
 */
 
#ifndef __FLUTSMUX_H264_H__
#define __FLUTSMUX_H264_H__
 
#include "flumpegtsmux.h"

GstBuffer * flutsmux_prepare_h264 (GstBuffer * buf, FluTsPadData * data,
    FluTsMux * mux);
 
#endif /* __FLUTSMUX_H264_H__ */
