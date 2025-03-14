/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef VP9_DECODER_VP9_DETOKENIZE_RECON_H_
#define VP9_DECODER_VP9_DETOKENIZE_RECON_H_

#include "vp9/decoder/vp9_onyxd_int.h"
#include "vp9/decoder/vp9_reader.h"

int vp9_decode_block_tokens_recon(VP9_DECODER_RECON *decoder_recon,
                            VP9_COMMON *cm, MACROBLOCKD *xd,
                            int plane, int block, BLOCK_SIZE plane_bsize,
                            int x, int y, TX_SIZE tx_size, int16_t offset, 
                            vp9_reader *r, uint8_t *token_cache);


#endif  // VP9_DECODER_VP9_DETOKENIZE_H_
