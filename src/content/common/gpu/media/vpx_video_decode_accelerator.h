// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_GPU_MEDIA_VPX_VIDEO_DECODE_ACCELERATOR_H_
#define CONTENT_COMMON_GPU_MEDIA_VPX_VIDEO_DECODE_ACCELERATOR_H_

#include <d3d9.h>

#include <list>
#include <map>
#include <mfidl.h>
#include <vector>

#include "base/compiler_specific.h"
#include "base/memory/linked_ptr.h"
#include "base/threading/non_thread_safe.h"
#include "base/win/scoped_comptr.h"
#include "content/common/content_export.h"
#include "content/common/gpu/media/video_decode_accelerator_impl.h"
#include "media/video/video_decode_accelerator.h"

// VPX
#include "base/callback.h"
#include "base/memory/weak_ptr.h"
#include "media/base/demuxer_stream.h"
#include "media/base/video_decoder.h"
#include "media/base/video_decoder_config.h"
#include "media/base/video_frame.h"

#define VPX_CODEC_DISABLE_COMPAT 1
extern "C" {
#include "third_party/libvpx/source/libvpx/vpx/vpx_decoder.h"
#include "third_party/libvpx/source/libvpx/vpx/vp8dx.h"
}

#define AMD_ACCELERATED

#ifdef AMD_ACCELERATED
#define vpx_codec_ctx vpx_codec_ctx_ex
#endif

interface IMFSample;
interface IDirect3DSurface9;

namespace content {

  struct VpxSample {
    std::vector<uint8> data_;
    int id_;

    void init(const uint8 *data, unsigned long length, int id){
      data_.resize(length);
      memcpy(&data_[0], data, length);
      id_ = id;
    };
  };

class CONTENT_EXPORT VPXVideoDecodeAccelerator
    : public VideoDecodeAcceleratorImpl,
      NON_EXPORTED_BASE(public base::NonThreadSafe) {


 public:
  enum State {
    kUninitialized,   // un-initialized.
    kNormal,          // normal playing state.
    kResetting,       // upon received Reset(), before ResetDone()
    kStopped,         // upon output EOS received.
    kFlushing,        // upon flush request received.
  };

  bool CanDecodeOnIOThread() { return false; }

  // Does not take ownership of |client| which must outlive |*this|.
  explicit VPXVideoDecodeAccelerator(
      media::VideoDecodeAccelerator::Client* client,
      const base::Callback<bool(void)>& make_context_current);
  virtual ~VPXVideoDecodeAccelerator();

  // media::VideoDecodeAccelerator implementation.
  virtual bool Initialize(media::VideoCodecProfile profile) OVERRIDE;
  virtual void Decode(const media::BitstreamBuffer& bitstream_buffer) OVERRIDE;
  virtual void AssignPictureBuffers(
      const std::vector<media::PictureBuffer>& buffers) OVERRIDE;
  virtual void ReusePictureBuffer(int32 picture_buffer_id) OVERRIDE;
  virtual void Flush() OVERRIDE;
  virtual void Reset() OVERRIDE;
  virtual void Destroy() OVERRIDE;

  // Initialization work needed before the process is sandboxed.
  // This includes:-
  // 1. Loads the dlls like mf/mfplat/d3d9, etc required for decoding.
  // 2. Setting up the device manager instance which is shared between all
  //    decoder instances.
  static void PreSandboxInitialization();

 private:
  // VPX specific start
  //scoped_refptr<base::MessageLoopProxy> message_loop_;
  //base::WeakPtrFactory<VPXVideoDecodeAccelerator> weak_factory_;
  //base::WeakPtr<VPXVideoDecodeAccelerator> weak_this_;

  State state_;

  //DecodeCB decode_cb_;
  base::Closure reset_cb_;

  //VideoDecoderConfig config_;

  vpx_codec_ctx* vpx_codec_;
  vpx_codec_ctx* vpx_codec_alpha_;
  // VPX specific end

  typedef void* EGLConfig;
  typedef void* EGLSurface;
  // Creates and initializes an instance of the D3D device and the
  // corresponding device manager. The device manager instance is eventually
  // passed to the IMFTransform interface implemented by the h.264 decoder.
  static bool CreateD3DDevManager();

  // Creates, initializes and sets the media types for the h.264 decoder.
  bool InitDecoder(media::VideoCodecProfile profile);

  // Validates whether the h.264 decoder supports hardware video acceleration.
  bool CheckDecoderVpxSupport();

  // Returns information about the input and output streams. This includes
  // alignment information, decoder support flags, minimum sample size, etc.
  bool GetStreamsInfoAndBufferReqs();

  // Registers the input and output media types on the h.264 decoder. This
  // includes the expected input and output formats.
  bool SetDecoderMediaTypes();

  // Registers the input media type for the h.264 decoder.
  bool SetDecoderInputMediaType();

  // Registers the output media type for the h.264 decoder.
  bool SetDecoderOutputMediaType(const GUID& subtype);

  // Passes a command message to the decoder. This includes commands like
  // start of stream, end of stream, flush, drain the decoder, etc.
  bool SendMFTMessage(MFT_MESSAGE_TYPE msg, int32 param);

  // The bulk of the decoding happens here. This function handles errors,
  // format changes and processes decoded output.
  void DoDecode();

  // Invoked when we have a valid decoded output sample. Retrieves the D3D
  // surface and maintains a copy of it which is passed eventually to the
  // client when we have a picture buffer to copy the surface contents to.
  bool ProcessOutputSample(IMFSample* sample);

  // Processes pending output samples by copying them to available picture
  // slots.
  void ProcessPendingSamples(IDirect3DSurface9 *direct_output_surface = NULL);

  // Helper function to notify the accelerator client about the error.
  void StopOnError(media::VideoDecodeAccelerator::Error error);

  // Transitions the decoder to the uninitialized state. The decoder will stop
  // accepting requests in this state.
  void Invalidate();

  // Notifies the client that the input buffer identifed by input_buffer_id has
  // been processed.
  void NotifyInputBufferRead(int input_buffer_id);

  // Notifies the client that initialize was completed.
  void NotifyInitializeDone();

  // Notifies the client that the decoder was flushed.
  void NotifyFlushDone();

  // Notifies the client that the decoder was reset.
  void NotifyResetDone();

  // Requests picture buffers from the client.
  void RequestPictureBuffers(int width, int height);

  // Notifies the client about the availability of a picture.
  void NotifyPictureReady(const media::Picture& picture);

  // Sends pending input buffer processed acks to the client if we don't have
  // output samples waiting to be processed.
  void NotifyInputBuffersDropped();

  // Decodes pending input buffers.
  void DecodePendingInputBuffers();

  // Helper for handling the Flush operation.
  void FlushInternal();

  // Helper for handling the Decode operation.
  void DecodeInternal(VpxSample& sample, bool pending);

  // Handles mid stream resolution changes.
  void HandleResolutionChanged(int width, int height);

  struct VPXPictureBuffer;
  typedef std::map<int32, linked_ptr<VPXPictureBuffer> > OutputBuffers;

  // Tells the client to dismiss the stale picture buffers passed in.
  void DismissStaleBuffers(const OutputBuffers& picture_buffers);

  // To expose client callbacks from VideoDecodeAccelerator.
  media::VideoDecodeAccelerator::Client* client_;

  base::win::ScopedComPtr<IMFTransform> decoder_;

  // These interface pointers are initialized before the process is sandboxed.
  // They are not released when the GPU process exits. This is ok for now
  // because the GPU process does not exit normally on Windows. It is always
  // terminated. The device manager instance is shared among all decoder
  // instances. This is OK because there is internal locking performed by the
  // device manager.
  //static IDirect3DDeviceManager9* device_manager_;
  static IDirect3DDevice9Ex* device_;
  static IDirect3DQuery9* query_;
  static IDirect3D9Ex* d3d9_;

  // The EGL config to use for decoded frames.
  EGLConfig egl_config_;

  // Current state of the decoder.
  //State state_;

  MFT_INPUT_STREAM_INFO input_stream_info_;
  MFT_OUTPUT_STREAM_INFO output_stream_info_;

  // Contains information about a decoded sample.
  struct PendingSampleInfo {
    PendingSampleInfo(int32 buffer_id, vpx_image_t* sample);
    ~PendingSampleInfo();

    int32 input_buffer_id;
    vpx_image_t *output_sample;
  };

  typedef std::list<PendingSampleInfo> PendingOutputSamples;

  // List of decoded output samples.
  PendingOutputSamples pending_output_samples_;

  // This map maintains the picture buffers passed the client for decoding.
  // The key is the picture buffer id.
  OutputBuffers output_picture_buffers_;

  // Set to true if we requested picture slots from the client.
  bool pictures_requested_;

  // Ideally the reset token would be a stack variable which is used while
  // creating the device manager. However it seems that the device manager
  // holds onto the token and attempts to access it if the underlying device
  // changes.
  // TODO(ananta): This needs to be verified.
  static uint32 dev_manager_reset_token_;

  // Counter which holds the number of input packets before a successful
  // decode.
  int inputs_before_decode_;

  // Set to true if all necessary initialization needed before the GPU process
  // is sandboxed is done.
  // This includes the following:
  // 1. All required decoder dlls were successfully loaded.
  // 2. The device manager initialization completed.
  static bool pre_sandbox_init_done_;

  // List of input samples waiting to be processed.
  typedef std::list<VpxSample/*base::win::ScopedComPtr<IMFSample>*/> PendingInputs;
  PendingInputs pending_input_buffers_;

  IDirect3DSurface9 *temp_surface_yv12_;
  IDirect3DSurface9 *temp_surface_rgba_;
  HANDLE temp_surface_rgba_shared_handle_;

  IDirect3DSurface9 *g_4k_temp_surface_rgba;
  HANDLE g_4k_temp_surface_rgba_shared_handle;
  IDirect3DSurface9 *g_hd_temp_surface_rgba;
  HANDLE g_hd_temp_surface_rgba_shared_handle;

  bool restart_, buffering_;

  unsigned int num_available_picture_buffers_;
  struct MyPicture {
    int id;
    int32 input_buffer_id;
    MyPicture() : id(0), input_buffer_id(0) {}
    MyPicture(int a, int32 b) : id(a), input_buffer_id(b) {}
  };
  std::vector<MyPicture> buffered_picture_buffers_;

  unsigned int width_, height_;

  // Callback to set the correct gl context.
  base::Callback<bool(void)> make_context_current_;
};

}  // namespace content

#endif  // CONTENT_COMMON_GPU_MEDIA_VPX_VIDEO_DECODE_ACCELERATOR_H_
