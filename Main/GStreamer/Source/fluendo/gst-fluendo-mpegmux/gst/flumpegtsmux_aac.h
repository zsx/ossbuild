/* Copyright 2006, 2007, 2008 Fluendo S.A. 
 * Authors: Jan Schmidt <jan@fluendo.com>
 *          Kapil Agrawal <kapil@fluendo.com>
 *          Julien Moutte <julien@fluendo.com>
 * See the COPYING file in the top-level directory.
 */
 
#ifndef __FLUTSMUX_AAC_H__
#define __FLUTSMUX_AAC_H__
 
#include "flumpegtsmux.h"

GstBuffer * flutsmux_prepare_aac (GstBuffer * buf, FluTsPadData * data,
    FluTsMux * mux);
 
#endif /* __FLUTSMUX_AAC_H__ */
