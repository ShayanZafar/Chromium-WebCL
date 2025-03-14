/*
 *  Copyright (c) 2013 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VP9_DECODER_VP9_TILE_INFO_H_
#define VP9_DECODER_VP9_TILE_INFO_H_

typedef struct TileBuffer {
  const uint8_t *data;
  size_t size;
  int col;
} TileBuffer;

#endif  // VP9_DECODER_VP9_TILE_INFO_H_
