/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP9_ENCODER_VP9_SEGMENTATION_H_
#define VP9_ENCODER_VP9_SEGMENTATION_H_

#include "vp9/common/vp9_blockd.h"
#include "vp9/encoder/vp9_onyx_int.h"

void vp9_enable_segmentation(VP9_PTR ptr);
void vp9_disable_segmentation(VP9_PTR ptr);

void vp9_disable_segfeature(struct segmentation *seg,
                            int segment_id,
                            SEG_LVL_FEATURES feature_id);
void vp9_clear_segdata(struct segmentation *seg,
                       int segment_id,
                       SEG_LVL_FEATURES feature_id);
// Valid values for a segment are 0 to 3
// Segmentation map is arrange as [Rows][Columns]
void vp9_set_segmentation_map(VP9_PTR ptr, unsigned char *segmentation_map);

// The values given for each segment can be either deltas (from the default
// value chosen for the frame) or absolute values.
//
// Valid range for abs values is (0-127 for MB_LVL_ALT_Q), (0-63 for
// SEGMENT_ALT_LF)
// Valid range for delta values are (+/-127 for MB_LVL_ALT_Q), (+/-63 for
// SEGMENT_ALT_LF)
//
// abs_delta = SEGMENT_DELTADATA (deltas) abs_delta = SEGMENT_ABSDATA (use
// the absolute values given).
void vp9_set_segment_data(VP9_PTR ptr, signed char *feature_data,
                          unsigned char abs_delta);

void vp9_choose_segmap_coding_method(VP9_COMP *cpi);

void vp9_reset_segment_features(struct segmentation *seg);

#endif  // VP9_ENCODER_VP9_SEGMENTATION_H_
