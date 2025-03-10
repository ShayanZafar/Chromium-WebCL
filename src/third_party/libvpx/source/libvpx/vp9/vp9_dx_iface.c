/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <stdlib.h>
#include <string.h>
#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#include "vpx/internal/vpx_codec_internal.h"
#include "./vpx_version.h"
#include "vp9/decoder/vp9_onyxd.h"
#include "vp9/decoder/vp9_onyxd_int.h"
#include "vp9/decoder/vp9_read_bit_buffer.h"
#include "vp9/vp9_iface_common.h"
#include "vp9/common/inter_ocl/vp9_inter_ocl_init.h"
#include "vp9/decoder/vp9_copy_mip_ocl.h"
#include "vp9/common/inter_ocl/vp9_yuv2rgba.h"
#include "vpx_ports/vpx_timer.h"

#define VP9_CAP_POSTPROC (CONFIG_VP9_POSTPROC ? VPX_CODEC_CAP_POSTPROC : 0)
typedef vpx_codec_stream_info_t  vp9_stream_info_t;
extern int CpuFlag;

/* Structures for handling memory allocations */
typedef enum {
  VP9_SEG_ALG_PRIV = 256,
  VP9_SEG_MAX
} mem_seg_id_t;
#define NELEMENTS(x) ((int)(sizeof(x)/sizeof(x[0])))

static unsigned long priv_sz(const vpx_codec_dec_cfg_t *si,
                             vpx_codec_flags_t flags);

static const mem_req_t vp9_mem_req_segs[] = {
  {VP9_SEG_ALG_PRIV, 0, 8, VPX_CODEC_MEM_ZERO, priv_sz},
  {VP9_SEG_MAX, 0, 0, 0, NULL}
};

struct vpx_codec_alg_priv {
  vpx_codec_priv_t        base;
  vpx_codec_mmap_t        mmaps[NELEMENTS(vp9_mem_req_segs) - 1];
  vpx_codec_dec_cfg_t     cfg;
  vp9_stream_info_t       si;
  int                     defer_alloc;
  int                     decoder_init;
  VP9D_PTR                pbi;
  VP9D_PTR                storage_pbi[2];
  int                     postproc_cfg_set;
  vp8_postproc_cfg_t      postproc_cfg;
#if CONFIG_POSTPROC_VISUALIZER
  unsigned int            dbg_postproc_flag;
  int                     dbg_color_ref_frame_flag;
  int                     dbg_color_mb_modes_flag;
  int                     dbg_color_b_modes_flag;
  int                     dbg_display_mv_flag;
#endif
  vpx_image_t             img;
  int                     img_setup;
  int                     img_avail;
  int                     invert_tile_order;
  int                     fb_lru;

  /* External buffer info to save for VP9 common. */
  vpx_codec_frame_buffer_t *fb_list;  // External frame buffers
  int fb_count;  // Total number of frame buffers
  vpx_realloc_frame_buffer_cb_fn_t realloc_fb_cb;
  void *user_priv;  // Private data associated with the external frame buffers.
};

static unsigned long priv_sz(const vpx_codec_dec_cfg_t *si,
                             vpx_codec_flags_t flags) {
  /* Although this declaration is constant, we can't use it in the requested
   * segments list because we want to define the requested segments list
   * before defining the private type (so that the number of memory maps is
   * known)
   */
  (void)si;
  return sizeof(vpx_codec_alg_priv_t);
}

static void vp9_init_ctx(vpx_codec_ctx_t *ctx, const vpx_codec_mmap_t *mmap) {
  int i;

  ctx->priv = mmap->base;
  ctx->priv->sz = sizeof(*ctx->priv);
  ctx->priv->iface = ctx->iface;
  ctx->priv->alg_priv = mmap->base;

  for (i = 0; i < NELEMENTS(ctx->priv->alg_priv->mmaps); i++)
    ctx->priv->alg_priv->mmaps[i].id = vp9_mem_req_segs[i].id;

  ctx->priv->alg_priv->mmaps[0] = *mmap;
  ctx->priv->alg_priv->si.sz = sizeof(ctx->priv->alg_priv->si);
  ctx->priv->init_flags = ctx->init_flags;

  if (ctx->config.dec) {
    /* Update the reference to the config structure to an internal copy. */
    ctx->priv->alg_priv->cfg = *ctx->config.dec;
    ctx->config.dec = &ctx->priv->alg_priv->cfg;
  }
}

static void vp9_init_ctx_ex(vpx_codec_ctx_t_ex *ctx, const vpx_codec_mmap_t *mmap) {
  int i;

  ctx->priv = mmap->base;
  ctx->priv->sz = sizeof(*ctx->priv);
  ctx->priv->iface = ctx->iface;
  ctx->priv->alg_priv = mmap->base;

  for (i = 0; i < NELEMENTS(ctx->priv->alg_priv->mmaps); i++)
    ctx->priv->alg_priv->mmaps[i].id = vp9_mem_req_segs[i].id;

  ctx->priv->alg_priv->mmaps[0] = *mmap;
  ctx->priv->alg_priv->si.sz = sizeof(ctx->priv->alg_priv->si);
  ctx->priv->init_flags = ctx->init_flags;

  if (ctx->config.dec) {
    /* Update the reference to the config structure to an internal copy. */
    ctx->priv->alg_priv->cfg = *ctx->config.dec;
    ctx->config.dec = &ctx->priv->alg_priv->cfg;
  }
}

static void vp9_finalize_mmaps(vpx_codec_alg_priv_t *ctx) {
  /* nothing to clean up */
}

static vpx_codec_err_t vp9_init(vpx_codec_ctx_t *ctx,
                                vpx_codec_priv_enc_mr_cfg_t *data) {
  vpx_codec_err_t res = VPX_CODEC_OK;

  // This function only allocates space for the vpx_codec_alg_priv_t
  // structure. More memory may be required at the time the stream
  // information becomes known.
  if (!ctx->priv) {
    vpx_codec_mmap_t mmap;

    mmap.id = vp9_mem_req_segs[0].id;
    mmap.sz = sizeof(vpx_codec_alg_priv_t);
    mmap.align = vp9_mem_req_segs[0].align;
    mmap.flags = vp9_mem_req_segs[0].flags;

    res = vpx_mmap_alloc(&mmap);
    if (!res) {
      vp9_init_ctx(ctx, &mmap);

      ctx->priv->alg_priv->defer_alloc = 1;
    }
  }

#if USE_INTER_PREDICT_OCL
  // Initialize opencl for vp9
  vp9_init_ocl();
#if COPY_MIP_GPU
  create_cpy_mip_kernel(&ocl_cpy_mip_obj);
#endif
#endif // USE_INTER_PREDICT_OCL

  return res;
}

static vpx_codec_err_t vp9_init_ex(vpx_codec_ctx_t_ex *ctx,
                                vpx_codec_priv_enc_mr_cfg_t *data, void *interOp_context) {
  vpx_codec_err_t res = VPX_CODEC_OK;

  // This function only allocates space for the vpx_codec_alg_priv_t
  // structure. More memory may be required at the time the stream
  // information becomes known.
  if (!ctx->priv) {
    vpx_codec_mmap_t mmap;

    mmap.id = vp9_mem_req_segs[0].id;
    mmap.sz = sizeof(vpx_codec_alg_priv_t);
    mmap.align = vp9_mem_req_segs[0].align;
    mmap.flags = vp9_mem_req_segs[0].flags;

    res = vpx_mmap_alloc(&mmap);
    if (!res) {
      vp9_init_ctx_ex(ctx, &mmap);

      ctx->priv->alg_priv->defer_alloc = 1;
    }
  }

#if USE_INTER_PREDICT_OCL
  // Initialize opencl for vp9
  if (interOp_context != NULL) {
    vp9_init_ocl_ex(interOp_context);
    yuv2rgba_ocl_obj.use_ex_flag = 1;
    init_yuv2rgba_ocl_obj();
  } else {
    vp9_init_ocl();
    yuv2rgba_ocl_obj.use_ex_flag = 0;
  }
#endif // USE_INTER_PREDICT_OCL

  return res;
}

static vpx_codec_err_t vp9_destroy(vpx_codec_alg_priv_t *ctx) {
  int i;

  // vp9_remove_decompressor(ctx->pbi);
  vp9_remove_decompressor_recon(ctx->pbi, ctx->storage_pbi);

  for (i = NELEMENTS(ctx->mmaps) - 1; i >= 0; i--) {
    if (ctx->mmaps[i].dtor)
      ctx->mmaps[i].dtor(&ctx->mmaps[i]);
  }

  return VPX_CODEC_OK;
}

static vpx_codec_err_t vp9_peek_si(const uint8_t *data, unsigned int data_sz,
                                   vpx_codec_stream_info_t *si) {
  if (data_sz <= 8) return VPX_CODEC_UNSUP_BITSTREAM;
  if (data + data_sz <= data) return VPX_CODEC_INVALID_PARAM;

  si->is_kf = 0;
  si->w = si->h = 0;

  {
    struct vp9_read_bit_buffer rb = { data, data + data_sz, 0, NULL, NULL };
    const int frame_marker = vp9_rb_read_literal(&rb, 2);
    const int version = vp9_rb_read_bit(&rb) | (vp9_rb_read_bit(&rb) << 1);
    if (frame_marker != VP9_FRAME_MARKER)
      return VPX_CODEC_UNSUP_BITSTREAM;
#if CONFIG_NON420
    if (version > 1) return VPX_CODEC_UNSUP_BITSTREAM;
#else
    if (version != 0) return VPX_CODEC_UNSUP_BITSTREAM;
#endif

    if (vp9_rb_read_bit(&rb)) {  // show an existing frame
      return VPX_CODEC_OK;
    }

    si->is_kf = !vp9_rb_read_bit(&rb);
    if (si->is_kf) {
      const int sRGB = 7;
      int colorspace;

      rb.bit_offset += 1;  // show frame
      rb.bit_offset += 1;  // error resilient

      if (vp9_rb_read_literal(&rb, 8) != VP9_SYNC_CODE_0 ||
          vp9_rb_read_literal(&rb, 8) != VP9_SYNC_CODE_1 ||
          vp9_rb_read_literal(&rb, 8) != VP9_SYNC_CODE_2) {
        return VPX_CODEC_UNSUP_BITSTREAM;
      }

      colorspace = vp9_rb_read_literal(&rb, 3);
      if (colorspace != sRGB) {
        rb.bit_offset += 1;  // [16,235] (including xvycc) vs [0,255] range
        if (version == 1) {
          rb.bit_offset += 2;  // subsampling x/y
          rb.bit_offset += 1;  // has extra plane
        }
      } else {
        if (version == 1) {
          rb.bit_offset += 1;  // has extra plane
        } else {
          // RGB is only available in version 1
          return VPX_CODEC_UNSUP_BITSTREAM;
        }
      }

      // TODO(jzern): these are available on non-keyframes in intra only mode.
      si->w = vp9_rb_read_literal(&rb, 16) + 1;
      si->h = vp9_rb_read_literal(&rb, 16) + 1;
    }
  }

  return VPX_CODEC_OK;
}

static vpx_codec_err_t vp9_get_si(vpx_codec_alg_priv_t    *ctx,
                                  vpx_codec_stream_info_t *si) {
  const size_t sz = (si->sz >= sizeof(vp9_stream_info_t))
                       ? sizeof(vp9_stream_info_t)
                       : sizeof(vpx_codec_stream_info_t);
  memcpy(si, &ctx->si, sz);
  si->sz = sz;

  return VPX_CODEC_OK;
}


static vpx_codec_err_t update_error_state(vpx_codec_alg_priv_t *ctx,
                           const struct vpx_internal_error_info *error) {
  if (error->error_code)
    ctx->base.err_detail = error->has_detail ? error->detail : NULL;

  return error->error_code;
}

static vpx_codec_err_t decode_one(vpx_codec_alg_priv_t *ctx,
                                  const uint8_t **data, unsigned int data_sz,
                                  void *user_priv, int64_t deadline) {
  vpx_codec_err_t res = VPX_CODEC_OK;

  ctx->img_avail = 0;

  /* Determine the stream parameters. Note that we rely on peek_si to
   * validate that we have a buffer that does not wrap around the top
   * of the heap.
   */
  if (!ctx->si.h)
    res = ctx->base.iface->dec.peek_si(*data, data_sz, &ctx->si);


  /* Perform deferred allocations, if required */
  if (!res && ctx->defer_alloc) {
    int i;

    for (i = 1; !res && i < NELEMENTS(ctx->mmaps); i++) {
      vpx_codec_dec_cfg_t cfg;

      cfg.w = ctx->si.w;
      cfg.h = ctx->si.h;
      ctx->mmaps[i].id = vp9_mem_req_segs[i].id;
      ctx->mmaps[i].sz = vp9_mem_req_segs[i].sz;
      ctx->mmaps[i].align = vp9_mem_req_segs[i].align;
      ctx->mmaps[i].flags = vp9_mem_req_segs[i].flags;

      if (!ctx->mmaps[i].sz)
        ctx->mmaps[i].sz = vp9_mem_req_segs[i].calc_sz(&cfg,
                                                       ctx->base.init_flags);

      res = vpx_mmap_alloc(&ctx->mmaps[i]);
    }

    if (!res)
      vp9_finalize_mmaps(ctx);

    ctx->defer_alloc = 0;
  }

  /* Initialize the decoder instance on the first frame*/
  if (!res && !ctx->decoder_init) {
    res = vpx_validate_mmaps(&ctx->si, ctx->mmaps,
                             vp9_mem_req_segs, NELEMENTS(vp9_mem_req_segs),
                             ctx->base.init_flags);

    if (!res) {
      VP9D_CONFIG oxcf;
      VP9D_PTR optr;

      vp9_initialize_dec();

      oxcf.width = ctx->si.w;
      oxcf.height = ctx->si.h;
      oxcf.version = 9;
      oxcf.postprocess = 0;
      oxcf.max_threads = ctx->cfg.threads;
      oxcf.inv_tile_order = ctx->invert_tile_order;
      optr = vp9_create_decompressor(&oxcf);

      // If postprocessing was enabled by the application and a
      // configuration has not been provided, default it.
      if (!ctx->postproc_cfg_set &&
          (ctx->base.init_flags & VPX_CODEC_USE_POSTPROC)) {
        ctx->postproc_cfg.post_proc_flag = VP8_DEBLOCK | VP8_DEMACROBLOCK;
        ctx->postproc_cfg.deblocking_level = 4;
        ctx->postproc_cfg.noise_level = 0;
      }

      if (!optr) {
        res = VPX_CODEC_ERROR;
      } else {
        VP9D_COMP *const pbi = (VP9D_COMP*)optr;
        VP9_COMMON *const cm = &pbi->common;
        if (ctx->fb_list != NULL && ctx->realloc_fb_cb != NULL &&
            ctx->fb_count > 0) {
          cm->fb_list = ctx->fb_list;
          cm->fb_count = ctx->fb_count;
          cm->realloc_fb_cb = ctx->realloc_fb_cb;
          cm->user_priv = ctx->user_priv;
        } else {
          cm->fb_count = FRAME_BUFFERS;
        }
        cm->fb_lru = ctx->fb_lru;
        CHECK_MEM_ERROR(cm, cm->yv12_fb,
                        vpx_calloc(cm->fb_count, sizeof(*cm->yv12_fb)));
        CHECK_MEM_ERROR(cm, cm->fb_idx_ref_cnt,
                        vpx_calloc(cm->fb_count, sizeof(*cm->fb_idx_ref_cnt)));
        if (cm->fb_lru) {
          CHECK_MEM_ERROR(cm, cm->fb_idx_ref_lru,
                          vpx_calloc(cm->fb_count,
                                     sizeof(*cm->fb_idx_ref_lru)));
        }
        ctx->pbi = optr;
      }
    }

    ctx->decoder_init = 1;
  }

  if (!res && ctx->pbi) {
    YV12_BUFFER_CONFIG sd;
    int64_t time_stamp = 0, time_end_stamp = 0;
    vp9_ppflags_t flags = {0};

    if (ctx->base.init_flags & VPX_CODEC_USE_POSTPROC) {
      flags.post_proc_flag =
#if CONFIG_POSTPROC_VISUALIZER
          (ctx->dbg_color_ref_frame_flag ? VP9D_DEBUG_CLR_FRM_REF_BLKS : 0) |
          (ctx->dbg_color_mb_modes_flag ? VP9D_DEBUG_CLR_BLK_MODES : 0) |
          (ctx->dbg_color_b_modes_flag ? VP9D_DEBUG_CLR_BLK_MODES : 0) |
          (ctx->dbg_display_mv_flag ? VP9D_DEBUG_DRAW_MV : 0) |
#endif
          ctx->postproc_cfg.post_proc_flag;

      flags.deblocking_level = ctx->postproc_cfg.deblocking_level;
      flags.noise_level = ctx->postproc_cfg.noise_level;
#if CONFIG_POSTPROC_VISUALIZER
      flags.display_ref_frame_flag = ctx->dbg_color_ref_frame_flag;
      flags.display_mb_modes_flag = ctx->dbg_color_mb_modes_flag;
      flags.display_b_modes_flag = ctx->dbg_color_b_modes_flag;
      flags.display_mv_flag = ctx->dbg_display_mv_flag;
#endif
    }

    if (vp9_receive_compressed_data(ctx->pbi, data_sz, data, deadline)) {
      VP9D_COMP *pbi = (VP9D_COMP*)ctx->pbi;
      res = update_error_state(ctx, &pbi->common.error);
    }

    if (!res && 0 == vp9_get_raw_frame(ctx->pbi, &sd, &time_stamp,
                                       &time_end_stamp, &flags)) {
      yuvconfig2image(&ctx->img, &sd, user_priv);
      ctx->img_avail = 1;
    }
  }

  return res;
}

static vpx_codec_err_t decode_one_recon(vpx_codec_alg_priv_t *ctx,
                                  const uint8_t **data, unsigned int data_sz,
                                  void *user_priv, int64_t deadline) {
  vpx_codec_err_t res = VPX_CODEC_OK;

  VP9D_COMP *pbi;
  VP9D_COMP *pbi_storage;
  static int flag = 0;
  int i_is_last_frame = 0;
  int ret = -1;

  // ctx->img_avail = 0;

  if (data_sz == 0) {
    pbi = (VP9D_COMP *)ctx->pbi;
    if (!pbi->l_bufpool_flag_output) {
      return 0;
    }
  }

  /* Determine the stream parameters. Note that we rely on peek_si to
   * validate that we have a buffer that does not wrap around the top
   * of the heap.
   */
  if (!ctx->si.h)
    res = ctx->base.iface->dec.peek_si(*data, data_sz, &ctx->si);


  /* Perform deferred allocations, if required */
  if (!res && ctx->defer_alloc) {
    int i;

    for (i = 1; !res && i < NELEMENTS(ctx->mmaps); i++) {
      vpx_codec_dec_cfg_t cfg;

      cfg.w = ctx->si.w;
      cfg.h = ctx->si.h;
      ctx->mmaps[i].id = vp9_mem_req_segs[i].id;
      ctx->mmaps[i].sz = vp9_mem_req_segs[i].sz;
      ctx->mmaps[i].align = vp9_mem_req_segs[i].align;
      ctx->mmaps[i].flags = vp9_mem_req_segs[i].flags;

      if (!ctx->mmaps[i].sz)
        ctx->mmaps[i].sz = vp9_mem_req_segs[i].calc_sz(&cfg,
                                                       ctx->base.init_flags);

      res = vpx_mmap_alloc(&ctx->mmaps[i]);
    }

    if (!res)
      vp9_finalize_mmaps(ctx);

    ctx->defer_alloc = 0;
  }

  /* Initialize the decoder instance on the first frame*/
  if (!res && !ctx->decoder_init) {
    res = vpx_validate_mmaps(&ctx->si, ctx->mmaps,
                             vp9_mem_req_segs, NELEMENTS(vp9_mem_req_segs),
                             ctx->base.init_flags);

    if (!res) {
      VP9D_CONFIG oxcf;
      VP9D_PTR optr;

      VP9D_COMP *const new_pbi = vpx_memalign(32, sizeof(VP9D_COMP));
      VP9D_COMP *const new_pbi_two = vpx_memalign(32, sizeof(VP9D_COMP));

      vp9_initialize_dec();

      oxcf.width = ctx->si.w;
      oxcf.height = ctx->si.h;
      oxcf.version = 9;
      oxcf.postprocess = 0;
      oxcf.max_threads = ctx->cfg.threads;
      oxcf.inv_tile_order = ctx->invert_tile_order;
      optr = vp9_create_decompressor_recon(&oxcf);

      vp9_zero(*new_pbi);
      vp9_zero(*new_pbi_two);

      // If postprocessing was enabled by the application and a
      // configuration has not been provided, default it.
      if (!ctx->postproc_cfg_set &&
          (ctx->base.init_flags & VPX_CODEC_USE_POSTPROC)) {
        ctx->postproc_cfg.post_proc_flag = VP8_DEBLOCK | VP8_DEMACROBLOCK;
        ctx->postproc_cfg.deblocking_level = 4;
        ctx->postproc_cfg.noise_level = 0;
      }

      if (!optr) {
        res = VPX_CODEC_ERROR;
      } else {
        VP9D_COMP *const pbi = (VP9D_COMP*)optr;
        VP9_COMMON *const cm = &pbi->common;

        VP9_COMMON *const cm0 = &new_pbi->common;
        VP9_COMMON *const cm1 = &new_pbi_two->common;

        if (ctx->fb_list != NULL && ctx->realloc_fb_cb != NULL &&
            ctx->fb_count > 0) {
          cm->fb_list = ctx->fb_list;
          cm->fb_count = ctx->fb_count;
          cm->realloc_fb_cb = ctx->realloc_fb_cb;
          cm->user_priv = ctx->user_priv;
          CpuFlag = 1;
        } else {
          CpuFlag = 0;
          cm->fb_count = FRAME_BUFFERS;
        }
        cm->fb_lru = ctx->fb_lru;
        CHECK_MEM_ERROR(cm, cm->yv12_fb,
                        vpx_calloc(cm->fb_count, sizeof(*cm->yv12_fb)));
        CHECK_MEM_ERROR(cm, cm->fb_idx_ref_cnt,
                        vpx_calloc(cm->fb_count, sizeof(*cm->fb_idx_ref_cnt)));
        if (cm->fb_lru) {
          CHECK_MEM_ERROR(cm, cm->fb_idx_ref_lru,
                          vpx_calloc(cm->fb_count,
                                     sizeof(*cm->fb_idx_ref_lru)));
        }
        ctx->pbi = optr;

        ctx->storage_pbi[0] = new_pbi;
        ctx->storage_pbi[1] = new_pbi_two;

        // cm 0
        if (ctx->fb_list != NULL && ctx->realloc_fb_cb != NULL &&
            ctx->fb_count > 0) {
          cm0->fb_list = ctx->fb_list;
          cm0->fb_count = ctx->fb_count;
          cm0->realloc_fb_cb = ctx->realloc_fb_cb;
          cm0->user_priv = ctx->user_priv;
        } else {
          cm0->fb_count = FRAME_BUFFERS;
        }
        cm0->fb_lru = ctx->fb_lru;
        // CHECK_MEM_ERROR(cm, cm->yv12_fb,
                        // vpx_calloc(cm->fb_count, sizeof(*cm->yv12_fb)));
        CHECK_MEM_ERROR(cm0, cm0->fb_idx_ref_cnt,
                        vpx_calloc(cm0->fb_count, sizeof(*cm0->fb_idx_ref_cnt)));
        if (cm0->fb_lru) {
          CHECK_MEM_ERROR(cm0, cm0->fb_idx_ref_lru,
                          vpx_calloc(cm0->fb_count,
                                     sizeof(*cm0->fb_idx_ref_lru)));
        }

        // cm 1
        if (ctx->fb_list != NULL && ctx->realloc_fb_cb != NULL &&
            ctx->fb_count > 0) {
          cm1->fb_list = ctx->fb_list;
          cm1->fb_count = ctx->fb_count;
          cm1->realloc_fb_cb = ctx->realloc_fb_cb;
          cm1->user_priv = ctx->user_priv;
        } else {
          cm1->fb_count = FRAME_BUFFERS;
        }
        cm1->fb_lru = ctx->fb_lru;
        // CHECK_MEM_ERROR(cm, cm->yv12_fb,
                        // vpx_calloc(cm->fb_count, sizeof(*cm->yv12_fb)));
        CHECK_MEM_ERROR(cm1, cm1->fb_idx_ref_cnt,
                        vpx_calloc(cm1->fb_count, sizeof(*cm1->fb_idx_ref_cnt)));
        if (cm1->fb_lru) {
          CHECK_MEM_ERROR(cm1, cm1->fb_idx_ref_lru,
                          vpx_calloc(cm1->fb_count,
                                     sizeof(*cm1->fb_idx_ref_lru)));
        }

      }
    }

    ctx->decoder_init = 1;
  }

  if (!res && ctx->pbi) {
    YV12_BUFFER_CONFIG sd;
    int64_t time_stamp = 0, time_end_stamp = 0;
    vp9_ppflags_t flags = {0};

    if (ctx->base.init_flags & VPX_CODEC_USE_POSTPROC) {
      flags.post_proc_flag =
#if CONFIG_POSTPROC_VISUALIZER
          (ctx->dbg_color_ref_frame_flag ? VP9D_DEBUG_CLR_FRM_REF_BLKS : 0) |
          (ctx->dbg_color_mb_modes_flag ? VP9D_DEBUG_CLR_BLK_MODES : 0) |
          (ctx->dbg_color_b_modes_flag ? VP9D_DEBUG_CLR_BLK_MODES : 0) |
          (ctx->dbg_display_mv_flag ? VP9D_DEBUG_DRAW_MV : 0) |
#endif
          ctx->postproc_cfg.post_proc_flag;

      flags.deblocking_level = ctx->postproc_cfg.deblocking_level;
      flags.noise_level = ctx->postproc_cfg.noise_level;
#if CONFIG_POSTPROC_VISUALIZER
      flags.display_ref_frame_flag = ctx->dbg_color_ref_frame_flag;
      flags.display_mb_modes_flag = ctx->dbg_color_mb_modes_flag;
      flags.display_b_modes_flag = ctx->dbg_color_b_modes_flag;
      flags.display_mv_flag = ctx->dbg_display_mv_flag;
#endif
    }

#if 0
    if (vp9_receive_compressed_data(ctx->pbi, data_sz, data, deadline)) {
      VP9D_COMP *pbi = (VP9D_COMP*)ctx->pbi;
      res = update_error_state(ctx, &pbi->common.error);
    }

    if (!res && 0 == vp9_get_raw_frame(ctx->pbi, &sd, &time_stamp,
                                       &time_end_stamp, &flags)) {
      yuvconfig2image(&ctx->img, &sd, user_priv);
      ctx->img_avail = 1;
    }
#endif
    
    if (data_sz == 0) {
      i_is_last_frame = 1;
    }
    
    if (vp9_receive_compressed_data_recon(ctx->pbi, ctx->storage_pbi, data_sz, data,
      deadline, i_is_last_frame)) {
      pbi = (VP9D_COMP *)ctx->pbi;
      if (pbi->l_bufpool_flag_output == 0)
        pbi_storage = (VP9D_COMP *)ctx->storage_pbi[1];
      else
        pbi_storage = (VP9D_COMP *)ctx->storage_pbi[pbi->l_bufpool_flag_output & 1];
      res = update_error_state(ctx, &pbi_storage->common.error);
    }
    if (ctx->pbi) {
      pbi = (VP9D_COMP *)ctx->pbi;
      if (pbi->l_bufpool_flag_output) {
        ret = vp9_get_raw_frame(ctx->storage_pbi[pbi->l_bufpool_flag_output & 1],
              &sd, &time_stamp, &time_end_stamp, &flags);
        if (!pbi->res && 0 == ret ) {
          yuvconfig2image(&ctx->img, &sd, user_priv);
          ctx->img_avail = 1;
        }
      }
    
      pbi->res = res;
      pbi->l_bufpool_flag_output++;
    }
        
    pbi = (VP9D_COMP *)ctx->pbi;
    if (pbi->common.show_frame) {
      if (flag || (pbi->common.current_video_frame != 1))
        pbi->common.current_video_frame++;
      else
        flag = 1;
    }
    
    if (data_sz == 0) {
      pbi->l_bufpool_flag_output = 0;
      return 0;
    }

  }
  return res;
}

static vpx_codec_err_t decode_one_recon_ex(vpx_codec_alg_priv_t *ctx,
                                  const uint8_t **data, unsigned int data_sz,
                                  void *user_priv, int64_t deadline, void *texture) {
  vpx_codec_err_t res = VPX_CODEC_OK;

  VP9D_COMP *pbi;
  VP9D_COMP *pbi_storage;
  VP9D_COMP *my_pbi;
  static int flag = 0;
  int i_is_last_frame = 0;
  int ret = -1;

  struct vpx_usec_timer timer;
  unsigned long yuv2rgb_time = 0;
  unsigned long decode_time = 0;

  // ctx->img_avail = 0;
  vpx_usec_timer_start(&timer);
  if (data_sz == 0) {
    pbi = (VP9D_COMP *)ctx->pbi;
    if (!pbi->l_bufpool_flag_output) {
      return 0;
    }
  }

  /* Determine the stream parameters. Note that we rely on peek_si to
   * validate that we have a buffer that does not wrap around the top
   * of the heap.
   */
  if (!ctx->si.h)
    res = ctx->base.iface->dec.peek_si(*data, data_sz, &ctx->si);


  /* Perform deferred allocations, if required */
  if (!res && ctx->defer_alloc) {
    int i;

    for (i = 1; !res && i < NELEMENTS(ctx->mmaps); i++) {
      vpx_codec_dec_cfg_t cfg;

      cfg.w = ctx->si.w;
      cfg.h = ctx->si.h;
      ctx->mmaps[i].id = vp9_mem_req_segs[i].id;
      ctx->mmaps[i].sz = vp9_mem_req_segs[i].sz;
      ctx->mmaps[i].align = vp9_mem_req_segs[i].align;
      ctx->mmaps[i].flags = vp9_mem_req_segs[i].flags;

      if (!ctx->mmaps[i].sz)
        ctx->mmaps[i].sz = vp9_mem_req_segs[i].calc_sz(&cfg,
                                                       ctx->base.init_flags);

      res = vpx_mmap_alloc(&ctx->mmaps[i]);
    }

    if (!res)
      vp9_finalize_mmaps(ctx);

    ctx->defer_alloc = 0;
  }

  /* Initialize the decoder instance on the first frame*/
  if (!res && !ctx->decoder_init) {
    res = vpx_validate_mmaps(&ctx->si, ctx->mmaps,
                             vp9_mem_req_segs, NELEMENTS(vp9_mem_req_segs),
                             ctx->base.init_flags);

    if (!res) {
      VP9D_CONFIG oxcf;
      VP9D_PTR optr;

      VP9D_COMP *const new_pbi = vpx_memalign(32, sizeof(VP9D_COMP));
      VP9D_COMP *const new_pbi_two = vpx_memalign(32, sizeof(VP9D_COMP));

      vp9_initialize_dec();

      oxcf.width = ctx->si.w;
      oxcf.height = ctx->si.h;
      oxcf.version = 9;
      oxcf.postprocess = 0;
      oxcf.max_threads = ctx->cfg.threads;
      oxcf.inv_tile_order = ctx->invert_tile_order;
      optr = vp9_create_decompressor_recon(&oxcf);

      vp9_zero(*new_pbi);
      vp9_zero(*new_pbi_two);

      // If postprocessing was enabled by the application and a
      // configuration has not been provided, default it.
      if (!ctx->postproc_cfg_set &&
          (ctx->base.init_flags & VPX_CODEC_USE_POSTPROC)) {
        ctx->postproc_cfg.post_proc_flag = VP8_DEBLOCK | VP8_DEMACROBLOCK;
        ctx->postproc_cfg.deblocking_level = 4;
        ctx->postproc_cfg.noise_level = 0;
      }

      if (!optr) {
        res = VPX_CODEC_ERROR;
      } else {
        VP9D_COMP *const pbi = (VP9D_COMP*)optr;
        VP9_COMMON *const cm = &pbi->common;

        VP9_COMMON *const cm0 = &new_pbi->common;
        VP9_COMMON *const cm1 = &new_pbi_two->common;

        if (ctx->fb_list != NULL && ctx->realloc_fb_cb != NULL &&
            ctx->fb_count > 0) {
          cm->fb_list = ctx->fb_list;
          cm->fb_count = ctx->fb_count;
          cm->realloc_fb_cb = ctx->realloc_fb_cb;
          cm->user_priv = ctx->user_priv;
          CpuFlag = 1;
        } else {
          CpuFlag = 0;
          cm->fb_count = FRAME_BUFFERS;
        }
        cm->fb_lru = ctx->fb_lru;
        CHECK_MEM_ERROR(cm, cm->yv12_fb,
                        vpx_calloc(cm->fb_count, sizeof(*cm->yv12_fb)));
        CHECK_MEM_ERROR(cm, cm->fb_idx_ref_cnt,
                        vpx_calloc(cm->fb_count, sizeof(*cm->fb_idx_ref_cnt)));
        if (cm->fb_lru) {
          CHECK_MEM_ERROR(cm, cm->fb_idx_ref_lru,
                          vpx_calloc(cm->fb_count,
                                     sizeof(*cm->fb_idx_ref_lru)));
        }
        ctx->pbi = optr;

        ctx->storage_pbi[0] = new_pbi;
        ctx->storage_pbi[1] = new_pbi_two;

        // cm 0
        if (ctx->fb_list != NULL && ctx->realloc_fb_cb != NULL &&
            ctx->fb_count > 0) {
          cm0->fb_list = ctx->fb_list;
          cm0->fb_count = ctx->fb_count;
          cm0->realloc_fb_cb = ctx->realloc_fb_cb;
          cm0->user_priv = ctx->user_priv;
        } else {
          cm0->fb_count = FRAME_BUFFERS;
        }
        cm0->fb_lru = ctx->fb_lru;
        // CHECK_MEM_ERROR(cm, cm->yv12_fb,
                        // vpx_calloc(cm->fb_count, sizeof(*cm->yv12_fb)));
        CHECK_MEM_ERROR(cm0, cm0->fb_idx_ref_cnt,
                        vpx_calloc(cm0->fb_count, sizeof(*cm0->fb_idx_ref_cnt)));
        if (cm0->fb_lru) {
          CHECK_MEM_ERROR(cm0, cm0->fb_idx_ref_lru,
                          vpx_calloc(cm0->fb_count,
                                     sizeof(*cm0->fb_idx_ref_lru)));
        }

        // cm 1
        if (ctx->fb_list != NULL && ctx->realloc_fb_cb != NULL &&
            ctx->fb_count > 0) {
          cm1->fb_list = ctx->fb_list;
          cm1->fb_count = ctx->fb_count;
          cm1->realloc_fb_cb = ctx->realloc_fb_cb;
          cm1->user_priv = ctx->user_priv;
        } else {
          cm1->fb_count = FRAME_BUFFERS;
        }
        cm1->fb_lru = ctx->fb_lru;
        // CHECK_MEM_ERROR(cm, cm->yv12_fb,
                        // vpx_calloc(cm->fb_count, sizeof(*cm->yv12_fb)));
        CHECK_MEM_ERROR(cm1, cm1->fb_idx_ref_cnt,
                        vpx_calloc(cm1->fb_count, sizeof(*cm1->fb_idx_ref_cnt)));
        if (cm1->fb_lru) {
          CHECK_MEM_ERROR(cm1, cm1->fb_idx_ref_lru,
                          vpx_calloc(cm1->fb_count,
                                     sizeof(*cm1->fb_idx_ref_lru)));
        }

      }
    }

    ctx->decoder_init = 1;
  }

  if (!res && ctx->pbi) {
    YV12_BUFFER_CONFIG sd;
    int64_t time_stamp = 0, time_end_stamp = 0;
    vp9_ppflags_t flags = {0};

    if (ctx->base.init_flags & VPX_CODEC_USE_POSTPROC) {
      flags.post_proc_flag =
#if CONFIG_POSTPROC_VISUALIZER
          (ctx->dbg_color_ref_frame_flag ? VP9D_DEBUG_CLR_FRM_REF_BLKS : 0) |
          (ctx->dbg_color_mb_modes_flag ? VP9D_DEBUG_CLR_BLK_MODES : 0) |
          (ctx->dbg_color_b_modes_flag ? VP9D_DEBUG_CLR_BLK_MODES : 0) |
          (ctx->dbg_display_mv_flag ? VP9D_DEBUG_DRAW_MV : 0) |
#endif
          ctx->postproc_cfg.post_proc_flag;

      flags.deblocking_level = ctx->postproc_cfg.deblocking_level;
      flags.noise_level = ctx->postproc_cfg.noise_level;
#if CONFIG_POSTPROC_VISUALIZER
      flags.display_ref_frame_flag = ctx->dbg_color_ref_frame_flag;
      flags.display_mb_modes_flag = ctx->dbg_color_mb_modes_flag;
      flags.display_b_modes_flag = ctx->dbg_color_b_modes_flag;
      flags.display_mv_flag = ctx->dbg_display_mv_flag;
#endif
    }

#if 0
    if (vp9_receive_compressed_data(ctx->pbi, data_sz, data, deadline)) {
      VP9D_COMP *pbi = (VP9D_COMP*)ctx->pbi;
      res = update_error_state(ctx, &pbi->common.error);
    }

    if (!res && 0 == vp9_get_raw_frame(ctx->pbi, &sd, &time_stamp,
                                       &time_end_stamp, &flags)) {
      yuvconfig2image(&ctx->img, &sd, user_priv);
      ctx->img_avail = 1;
    }
#endif
    
    if (data_sz == 0) {
      i_is_last_frame = 1;
    }
    
    if (vp9_receive_compressed_data_recon(ctx->pbi, ctx->storage_pbi, data_sz, data,
      deadline, i_is_last_frame)) {
      pbi = (VP9D_COMP *)ctx->pbi;
      if (pbi->l_bufpool_flag_output == 0)
        pbi_storage = (VP9D_COMP *)ctx->storage_pbi[1];
      else
        pbi_storage = (VP9D_COMP *)ctx->storage_pbi[pbi->l_bufpool_flag_output & 1];
      res = update_error_state(ctx, &pbi_storage->common.error);
    }
	vpx_usec_timer_mark(&timer);
    decode_time = (unsigned int)vpx_usec_timer_elapsed(&timer);
    if (ctx->pbi) {
      pbi = (VP9D_COMP *)ctx->pbi;
      if (pbi->l_bufpool_flag_output) {
        ret = vp9_get_raw_frame(ctx->storage_pbi[pbi->l_bufpool_flag_output & 1],
              &sd, &time_stamp, &time_end_stamp, &flags);
        if (!pbi->res && 0 == ret ) {
          //for render
          my_pbi = (VP9D_COMP *)(ctx->storage_pbi[pbi->l_bufpool_flag_output & 1]);
          yuv2rgba_ocl_obj.y_plane_offset = my_pbi->common.frame_to_show->y_buffer - 
                                                inter_ocl_obj.buffer_pool_map_ptr;
          yuv2rgba_ocl_obj.u_plane_offset = my_pbi->common.frame_to_show->u_buffer - 
                                                inter_ocl_obj.buffer_pool_map_ptr;
          yuv2rgba_ocl_obj.v_plane_offset = my_pbi->common.frame_to_show->v_buffer - 
                                                inter_ocl_obj.buffer_pool_map_ptr;
 
          yuv2rgba_ocl_obj.Y_stride =  my_pbi->common.frame_to_show->y_stride;
          yuv2rgba_ocl_obj.UV_stride =  my_pbi->common.frame_to_show->uv_stride;
          yuv2rgba_ocl_obj.globalThreads[0] =  my_pbi->common.width >> 1;
          yuv2rgba_ocl_obj.globalThreads[1] =  my_pbi->common.height >> 1;
 
		  vpx_usec_timer_start(&timer);
          vp9_yuv2rgba(&yuv2rgba_ocl_obj, texture);
		  vpx_usec_timer_mark(&timer);
          yuv2rgb_time = (unsigned int)vpx_usec_timer_elapsed(&timer);
	      fprintf(pLog, "decode one frame time(without YUV to RGB): %lu us\n"
			            "the whole time of YUV to RGB:  %lu us\n", decode_time, yuv2rgb_time);
          // for render end
          yuvconfig2image(&ctx->img, &sd, user_priv);
          ctx->img_avail = 1;
        }
      }
    
      pbi->res = res;
      pbi->l_bufpool_flag_output++;
    }
        
    pbi = (VP9D_COMP *)ctx->pbi;
    if (pbi->common.show_frame) {
      if (flag || (pbi->common.current_video_frame != 1))
        pbi->common.current_video_frame++;
      else
        flag = 1;
    }
    
    if (data_sz == 0) {
      pbi->l_bufpool_flag_output = 0;
      return 0;
    }

  }

  return res;
}

static void parse_superframe_index(const uint8_t *data, size_t data_sz,
                                   uint32_t sizes[8], int *count) {
  uint8_t marker;

  assert(data_sz);
  marker = data[data_sz - 1];
  *count = 0;

  if ((marker & 0xe0) == 0xc0) {
    const uint32_t frames = (marker & 0x7) + 1;
    const uint32_t mag = ((marker >> 3) & 0x3) + 1;
    const size_t index_sz = 2 + mag * frames;

    if (data_sz >= index_sz && data[data_sz - index_sz] == marker) {
      // found a valid superframe index
      uint32_t i, j;
      const uint8_t *x = data + data_sz - index_sz + 1;

      for (i = 0; i < frames; i++) {
        uint32_t this_sz = 0;

        for (j = 0; j < mag; j++)
          this_sz |= (*x++) << (j * 8);
        sizes[i] = this_sz;
      }

      *count = frames;
    }
  }
}

static vpx_codec_err_t vp9_decode(vpx_codec_alg_priv_t  *ctx,
                                  const uint8_t         *data,
                                  unsigned int           data_sz,
                                  void                  *user_priv,
                                  long                   deadline) {
  const uint8_t *data_start = data;
  const uint8_t *data_end = data + data_sz;
  vpx_codec_err_t res = 0;
  uint32_t sizes[8];
  int frames_this_pts, frame_count = 0;

  // if (data == NULL || data_sz == 0) return VPX_CODEC_INVALID_PARAM;
  if (data == NULL || data_sz == 0) {
    return decode_one_recon(ctx, &data_start, data_sz, user_priv, deadline);
  }

  parse_superframe_index(data, data_sz, sizes, &frames_this_pts);

  do {
    // Skip over the superframe index, if present
    if (data_sz && (*data_start & 0xe0) == 0xc0) {
      const uint8_t marker = *data_start;
      const uint32_t frames = (marker & 0x7) + 1;
      const uint32_t mag = ((marker >> 3) & 0x3) + 1;
      const uint32_t index_sz = 2 + mag * frames;

      if (data_sz >= index_sz && data_start[index_sz - 1] == marker) {
        data_start += index_sz;
        data_sz -= index_sz;
        if (data_start < data_end)
          continue;
        else
          break;
      }
    }

    // Use the correct size for this frame, if an index is present.
    if (frames_this_pts) {
      uint32_t this_sz = sizes[frame_count];

      if (data_sz < this_sz) {
        ctx->base.err_detail = "Invalid frame size in index";
        return VPX_CODEC_CORRUPT_FRAME;
      }

      data_sz = this_sz;
      frame_count++;
    }

    res = decode_one_recon(ctx, &data_start, data_sz, user_priv, deadline);
    assert(data_start >= data);
    assert(data_start <= data_end);

    /* Early exit if there was a decode error */
    if (res)
      break;

    /* Account for suboptimal termination by the encoder. */
    while (data_start < data_end && *data_start == 0)
      data_start++;

    data_sz = data_end - data_start;
  } while (data_start < data_end);
  return res;
}

static vpx_codec_err_t vp9_decode_ex(vpx_codec_alg_priv_t  *ctx,
                                  const uint8_t         *data,
                                  unsigned int           data_sz,
                                  void                  *user_priv,
                                  long                   deadline,
                                  void *texture) {           
  const uint8_t *data_start = data;
  const uint8_t *data_end = data + data_sz;
  vpx_codec_err_t res = 0;
  uint32_t sizes[8];
  int frames_this_pts, frame_count = 0;
 
  // if (data == NULL || data_sz == 0) return VPX_CODEC_INVALID_PARAM;
  if (data == NULL || data_sz == 0) {
    if (texture == NULL)      
      return decode_one_recon(ctx, &data_start, data_sz, user_priv, deadline); 
    else  
      return decode_one_recon_ex(ctx, &data_start, data_sz, user_priv, deadline, texture);
    
  }


  parse_superframe_index(data, data_sz, sizes, &frames_this_pts);

  do {
    // Skip over the superframe index, if present
    if (data_sz && (*data_start & 0xe0) == 0xc0) {
      const uint8_t marker = *data_start;
      const uint32_t frames = (marker & 0x7) + 1;
      const uint32_t mag = ((marker >> 3) & 0x3) + 1;
      const uint32_t index_sz = 2 + mag * frames;

      if (data_sz >= index_sz && data_start[index_sz - 1] == marker) {
        data_start += index_sz;
        data_sz -= index_sz;
        if (data_start < data_end)
          continue;
        else
          break;
      }
    }
    
    // Use the correct size for this frame, if an index is present.
    if (frames_this_pts) {
      uint32_t this_sz = sizes[frame_count];

      if (data_sz < this_sz) {
        ctx->base.err_detail = "Invalid frame size in index";
        return VPX_CODEC_CORRUPT_FRAME;
      }

      data_sz = this_sz;
      frame_count++;
    }
      
    if (texture == NULL)
      res = decode_one_recon(ctx, &data_start, data_sz, user_priv, deadline);
    else
       res = decode_one_recon_ex(ctx, &data_start, data_sz, user_priv, deadline, texture);
    
    assert(data_start >= data);
    assert(data_start <= data_end);

    /* Early exit if there was a decode error */
    if (res)
      break;

    /* Account for suboptimal termination by the encoder. */
    while (data_start < data_end && *data_start == 0)
      data_start++;

    data_sz = data_end - data_start;
  } while (data_start < data_end);
 
  return res;
}

static vpx_image_t *vp9_get_frame(vpx_codec_alg_priv_t  *ctx,
                                  vpx_codec_iter_t      *iter) {
  vpx_image_t *img = NULL;

  if (ctx->img_avail) {
    /* iter acts as a flip flop, so an image is only returned on the first
     * call to get_frame.
     */
    if (!(*iter)) {
      img = &ctx->img;
      *iter = img;
    }
  }
  ctx->img_avail = 0;

  return img;
}

static vpx_codec_err_t vp9_set_frame_buffers(
    vpx_codec_alg_priv_t *ctx,
    vpx_codec_frame_buffer_t *fb_list, int fb_count,
    vpx_realloc_frame_buffer_cb_fn_t cb, void *user_priv) {
  if (fb_count < (VP9_MAXIMUM_REF_BUFFERS + VPX_MAXIMUM_WORK_BUFFERS)) {
    /* The application must pass in at least VP9_MAXIMUM_REF_BUFFERS +
     * VPX_MAXIMUM_WORK_BUFFERS frame buffers. */
    return VPX_CODEC_INVALID_PARAM;
  } else if (!ctx->pbi) {
    /* If the decoder has already been initialized, do not accept external
     * frame buffers.
     */
    ctx->fb_list = fb_list;
    ctx->fb_count = fb_count;
    ctx->realloc_fb_cb = cb;
    ctx->user_priv = user_priv;
    return VPX_CODEC_OK;
  }

  return VPX_CODEC_ERROR;
}

static vpx_codec_err_t vp9_xma_get_mmap(const vpx_codec_ctx_t *ctx,
                                        vpx_codec_mmap_t *mmap,
                                        vpx_codec_iter_t *iter) {
  vpx_codec_err_t res;
  const mem_req_t *seg_iter = *iter;

  /* Get address of next segment request */
  do {
    if (!seg_iter)
      seg_iter = vp9_mem_req_segs;
    else if (seg_iter->id != VP9_SEG_MAX)
      seg_iter++;

    *iter = (vpx_codec_iter_t)seg_iter;

    if (seg_iter->id != VP9_SEG_MAX) {
      mmap->id = seg_iter->id;
      mmap->sz = seg_iter->sz;
      mmap->align = seg_iter->align;
      mmap->flags = seg_iter->flags;

      if (!seg_iter->sz)
        mmap->sz = seg_iter->calc_sz(ctx->config.dec, ctx->init_flags);

      res = VPX_CODEC_OK;
    } else {
      res = VPX_CODEC_LIST_END;
    }
  } while (!mmap->sz && res != VPX_CODEC_LIST_END);

  return res;
}

static vpx_codec_err_t vp9_xma_set_mmap(vpx_codec_ctx_t *ctx,
                                        const vpx_codec_mmap_t  *mmap) {
  vpx_codec_err_t res = VPX_CODEC_MEM_ERROR;
  int i, done;

  if (!ctx->priv) {
    if (mmap->id == VP9_SEG_ALG_PRIV) {
      if (!ctx->priv) {
        vp9_init_ctx(ctx, mmap);
        res = VPX_CODEC_OK;
      }
    }
  }

  done = 1;

  if (!res && ctx->priv->alg_priv) {
    for (i = 0; i < NELEMENTS(ctx->priv->alg_priv->mmaps); i++) {
      if (ctx->priv->alg_priv->mmaps[i].id == mmap->id)
        if (!ctx->priv->alg_priv->mmaps[i].base) {
          ctx->priv->alg_priv->mmaps[i] = *mmap;
          res = VPX_CODEC_OK;
        }

      done &= (ctx->priv->alg_priv->mmaps[i].base != NULL);
    }
  }

  if (done && !res) {
    vp9_finalize_mmaps(ctx->priv->alg_priv);
    res = ctx->iface->init(ctx, NULL);
  }

  return res;
}

static vpx_codec_err_t set_reference(vpx_codec_alg_priv_t *ctx, int ctr_id,
                                     va_list args) {
  vpx_ref_frame_t *data = va_arg(args, vpx_ref_frame_t *);

  if (data) {
    vpx_ref_frame_t *frame = (vpx_ref_frame_t *)data;
    YV12_BUFFER_CONFIG sd;

    image2yuvconfig(&frame->img, &sd);
    return vp9_set_reference_dec(ctx->pbi,
                                 (VP9_REFFRAME)frame->frame_type, &sd);
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t copy_reference(vpx_codec_alg_priv_t *ctx, int ctr_id,
                                      va_list args) {
  vpx_ref_frame_t *data = va_arg(args, vpx_ref_frame_t *);

  if (data) {
    vpx_ref_frame_t *frame = (vpx_ref_frame_t *)data;
    YV12_BUFFER_CONFIG sd;

    image2yuvconfig(&frame->img, &sd);

    return vp9_copy_reference_dec(ctx->pbi,
                                  (VP9_REFFRAME)frame->frame_type, &sd);
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t get_reference(vpx_codec_alg_priv_t *ctx, int ctr_id,
                                     va_list args) {
  vp9_ref_frame_t *data = va_arg(args, vp9_ref_frame_t *);

  if (data) {
    YV12_BUFFER_CONFIG* fb;

    vp9_get_reference_dec(ctx->pbi, data->idx, &fb);
    yuvconfig2image(&data->img, fb, NULL);
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t set_postproc(vpx_codec_alg_priv_t *ctx, int ctr_id,
                                    va_list args) {
#if CONFIG_VP9_POSTPROC
  vp8_postproc_cfg_t *data = va_arg(args, vp8_postproc_cfg_t *);

  if (data) {
    ctx->postproc_cfg_set = 1;
    ctx->postproc_cfg = *((vp8_postproc_cfg_t *)data);
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
#else
  return VPX_CODEC_INCAPABLE;
#endif
}

static vpx_codec_err_t set_dbg_options(vpx_codec_alg_priv_t *ctx, int ctrl_id,
                                       va_list args) {
#if CONFIG_POSTPROC_VISUALIZER && CONFIG_POSTPROC
  int data = va_arg(args, int);

#define MAP(id, var) case id: var = data; break;

  switch (ctrl_id) {
      MAP(VP8_SET_DBG_COLOR_REF_FRAME,   ctx->dbg_color_ref_frame_flag);
      MAP(VP8_SET_DBG_COLOR_MB_MODES,    ctx->dbg_color_mb_modes_flag);
      MAP(VP8_SET_DBG_COLOR_B_MODES,     ctx->dbg_color_b_modes_flag);
      MAP(VP8_SET_DBG_DISPLAY_MV,        ctx->dbg_display_mv_flag);
  }

  return VPX_CODEC_OK;
#else
  return VPX_CODEC_INCAPABLE;
#endif
}

static vpx_codec_err_t get_last_ref_updates(vpx_codec_alg_priv_t *ctx,
                                            int ctrl_id, va_list args) {
  int *update_info = va_arg(args, int *);
  VP9D_COMP *pbi = (VP9D_COMP*)ctx->pbi;

  if (update_info) {
    *update_info = pbi->refresh_frame_flags;

    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}


static vpx_codec_err_t get_frame_corrupted(vpx_codec_alg_priv_t *ctx,
                                           int ctrl_id, va_list args) {
  int *corrupted = va_arg(args, int *);

  if (corrupted) {
    VP9D_COMP *pbi = (VP9D_COMP*)ctx->pbi;
    if (pbi)
      *corrupted = pbi->common.frame_to_show->corrupted;
    else
      return VPX_CODEC_ERROR;
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t get_frame_corrupted_recon(vpx_codec_alg_priv_t *ctx,
                                           int ctrl_id, va_list args) {
  VP9D_COMP *pbi_new;
  int *corrupted = va_arg(args, int *);

  if (corrupted) {
    VP9D_COMP *pbi = (VP9D_COMP*)ctx->pbi;
    // if (pbi)
      // *corrupted = pbi->common.frame_to_show->corrupted;
    if (pbi->l_bufpool_flag_output != 1)
      pbi_new = (VP9D_COMP *)ctx->storage_pbi[(pbi->l_bufpool_flag_output - 1) & 1];
    else
      pbi_new = (VP9D_COMP *)ctx->storage_pbi[pbi->l_bufpool_flag_output & 1];
    if (pbi_new)
      *corrupted = pbi_new->common.frame_to_show->corrupted;
    else
      return VPX_CODEC_ERROR;
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t get_display_size(vpx_codec_alg_priv_t *ctx,
                                        int ctrl_id, va_list args) {
  int *const display_size = va_arg(args, int *);

  if (display_size) {
    const VP9D_COMP *const pbi = (VP9D_COMP*)ctx->pbi;
    if (pbi) {
      display_size[0] = pbi->common.display_width;
      display_size[1] = pbi->common.display_height;
    } else {
      return VPX_CODEC_ERROR;
    }
    return VPX_CODEC_OK;
  } else {
    return VPX_CODEC_INVALID_PARAM;
  }
}

static vpx_codec_err_t set_invert_tile_order(vpx_codec_alg_priv_t *ctx,
                                             int ctr_id,
                                             va_list args) {
  ctx->invert_tile_order = va_arg(args, int);
  return VPX_CODEC_OK;
}

static vpx_codec_err_t set_frame_buffer_lru_cache(vpx_codec_alg_priv_t *ctx,
                                                  int ctr_id,
                                                  va_list args) {
  VP9D_COMP *const pbi = (VP9D_COMP*)ctx->pbi;

  // Save for later to pass into vp9 common.
  ctx->fb_lru = va_arg(args, int);

  if (pbi) {
    VP9_COMMON *const cm = &pbi->common;
    cm->fb_lru = ctx->fb_lru;
  }
  return VPX_CODEC_OK;
}

static vpx_codec_ctrl_fn_map_t ctf_maps[] = {
  {VP8_SET_REFERENCE,             set_reference},
  {VP8_COPY_REFERENCE,            copy_reference},
  {VP8_SET_POSTPROC,              set_postproc},
  {VP8_SET_DBG_COLOR_REF_FRAME,   set_dbg_options},
  {VP8_SET_DBG_COLOR_MB_MODES,    set_dbg_options},
  {VP8_SET_DBG_COLOR_B_MODES,     set_dbg_options},
  {VP8_SET_DBG_DISPLAY_MV,        set_dbg_options},
  {VP8D_GET_LAST_REF_UPDATES,     get_last_ref_updates},
  // {VP8D_GET_FRAME_CORRUPTED,      get_frame_corrupted},
  {VP8D_GET_FRAME_CORRUPTED,      get_frame_corrupted_recon},
  {VP9_GET_REFERENCE,             get_reference},
  {VP9D_GET_DISPLAY_SIZE,         get_display_size},
  {VP9_INVERT_TILE_DECODE_ORDER,  set_invert_tile_order},
  {VP9D_SET_FRAME_BUFFER_LRU_CACHE, set_frame_buffer_lru_cache},
  { -1, NULL},
};


#ifndef VERSION_STRING
#define VERSION_STRING
#endif
#if 1
CODEC_INTERFACE_EX(vpx_codec_vp9_dx) = {
  "WebM Project VP9 Decoder" VERSION_STRING,
  VPX_CODEC_INTERNAL_ABI_VERSION,
  VPX_CODEC_CAP_DECODER | VP9_CAP_POSTPROC |
      VPX_CODEC_CAP_EXTERNAL_FRAME_BUFFER,
  /* vpx_codec_caps_t          caps; */
  vp9_init_ex,         /* vpx_codec_init_fn_t       init; */
  vp9_destroy,      /* vpx_codec_destroy_fn_t    destroy; */
  ctf_maps,         /* vpx_codec_ctrl_fn_map_t  *ctrl_maps; */
  vp9_xma_get_mmap, /* vpx_codec_get_mmap_fn_t   get_mmap; */
  vp9_xma_set_mmap, /* vpx_codec_set_mmap_fn_t   set_mmap; */
  { // NOLINT
    vp9_peek_si,      /* vpx_codec_peek_si_fn_t    peek_si; */
    vp9_get_si,       /* vpx_codec_get_si_fn_t     get_si; */
    vp9_decode_ex,       /* vpx_codec_decode_fn_t     decode; */
    vp9_get_frame,    /* vpx_codec_frame_get_fn_t  frame_get; */
    vp9_set_frame_buffers,    /* vpx_codec_set_frame_buffers_fn_t  set_fb; */
  },
  { // NOLINT
    /* encoder functions */
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED
  }
};
#else
CODEC_INTERFACE_EX(vpx_codec_vp9_dx_ex) = {
  "WebM Project VP9 Decoder" VERSION_STRING,
  VPX_CODEC_INTERNAL_ABI_VERSION,
  VPX_CODEC_CAP_DECODER | VP9_CAP_POSTPROC |
      VPX_CODEC_CAP_EXTERNAL_FRAME_BUFFER,
  /* vpx_codec_caps_t          caps; */
  vp9_init_ex,         /* vpx_codec_init_fn_t       init; */
  vp9_destroy,      /* vpx_codec_destroy_fn_t    destroy; */
  ctf_maps,         /* vpx_codec_ctrl_fn_map_t  *ctrl_maps; */
  vp9_xma_get_mmap, /* vpx_codec_get_mmap_fn_t   get_mmap; */
  vp9_xma_set_mmap, /* vpx_codec_set_mmap_fn_t   set_mmap; */
  { // NOLINT
    vp9_peek_si,      /* vpx_codec_peek_si_fn_t    peek_si; */
    vp9_get_si,       /* vpx_codec_get_si_fn_t     get_si; */
    vp9_decode_ex,       /* vpx_codec_decode_fn_t     decode; */
    vp9_get_frame,    /* vpx_codec_frame_get_fn_t  frame_get; */
    vp9_set_frame_buffers,    /* vpx_codec_set_frame_buffers_fn_t  set_fb; */
  },
  { // NOLINT
    /* encoder functions */
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED,
    NOT_IMPLEMENTED
  }
};

#endif