/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef TOOLS_COMMON_H_
#define TOOLS_COMMON_H_

#include <stdio.h>

#include "./vpx_config.h"
#include "vpx/vpx_image.h"
#include "vpx/vpx_integer.h"

#if CONFIG_ENCODERS
#include "./y4minput.h"
#endif

#if defined(_MSC_VER)
/* MSVS doesn't define off_t, and uses _f{seek,tell}i64. */
typedef __int64 off_t;
#define fseeko _fseeki64
#define ftello _ftelli64
#elif defined(_WIN32)
/* MinGW defines off_t as long and uses f{seek,tell}o64/off64_t for large
 * files. */
#define fseeko fseeko64
#define ftello ftello64
#define off_t off64_t
#endif  /* _WIN32 */

#if CONFIG_OS_SUPPORT
#if defined(_MSC_VER)
#include <io.h>  /* NOLINT */
#define snprintf _snprintf
#define isatty   _isatty
#define fileno   _fileno
#else
#include <unistd.h>  /* NOLINT */
#endif  /* _MSC_VER */
#endif  /* CONFIG_OS_SUPPORT */

/* Use 32-bit file operations in WebM file format when building ARM
 * executables (.axf) with RVCT. */
#if !CONFIG_OS_SUPPORT
typedef long off_t;  /* NOLINT */
#define fseeko fseek
#define ftello ftell
#endif  /* CONFIG_OS_SUPPORT */

#define LITERALU64(hi, lo) ((((uint64_t)hi) << 32) | lo)

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

#define IVF_FRAME_HDR_SZ (4 + 8)  /* 4 byte size + 8 byte timestamp */
#define IVF_FILE_HDR_SZ 32

#define RAW_FRAME_HDR_SZ sizeof(uint32_t)

#define VP8_FOURCC 0x30385056
#define VP9_FOURCC 0x30395056

enum VideoFileType {
  FILE_TYPE_RAW,
  FILE_TYPE_IVF,
  FILE_TYPE_Y4M,
  FILE_TYPE_WEBM
};

struct FileTypeDetectionBuffer {
  char buf[4];
  size_t buf_read;
  size_t position;
};

struct VpxRational {
  int numerator;
  int denominator;
};

struct VpxInputContext {
  const char *filename;
  FILE *file;
  off_t length;
  struct FileTypeDetectionBuffer detect;
  enum VideoFileType file_type;
  uint32_t width;
  uint32_t height;
  int use_i420;
  int only_i420;
  uint32_t fourcc;
  struct VpxRational framerate;
#if CONFIG_ENCODERS
  y4m_input y4m;
#endif
};

#ifdef __cplusplus
//extern "C" {
#endif

/* Sets a stdio stream into binary mode */
FILE *set_binary_mode(FILE *stream);

void die(const char *fmt, ...);
void fatal(const char *fmt, ...);
void warn(const char *fmt, ...);

/* The tool including this file must define usage_exit() */
void usage_exit();

uint16_t mem_get_le16(const void *data);
uint32_t mem_get_le32(const void *data);

int read_yuv_frame(struct VpxInputContext *input_ctx, vpx_image_t *yuv_frame);

#ifdef __cplusplus
//}  /* extern "C" */
#endif

#endif  // TOOLS_COMMON_H_
