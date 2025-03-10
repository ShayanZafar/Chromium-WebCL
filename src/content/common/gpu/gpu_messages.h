// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included message file, hence no include guard here, but see below
// for a much smaller-than-usual include guard section.

#include <string>
#include <vector>

#include "base/memory/shared_memory.h"
#include "content/common/content_export.h"
#include "content/common/gpu/gpu_memory_uma_stats.h"
#include "content/common/gpu/gpu_process_launch_causes.h"
#include "content/common/gpu/gpu_rendering_stats.h"
#include "content/public/common/common_param_traits.h"
#include "content/public/common/gpu_memory_stats.h"
#include "gpu/command_buffer/common/command_buffer.h"
#include "gpu/command_buffer/common/constants.h"
#include "gpu/command_buffer/common/gpu_memory_allocation.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/config/gpu_info.h"
#include "gpu/ipc/gpu_command_buffer_traits.h"
#include "ipc/ipc_channel_handle.h"
#include "ipc/ipc_message_macros.h"
#include "media/base/video_frame.h"
#include "media/video/video_decode_accelerator.h"
#include "media/video/video_encode_accelerator.h"
#include "ui/events/latency_info.h"
#include "ui/gfx/gpu_memory_buffer.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/size.h"
#include "ui/gl/gpu_preference.h"

#if defined(OS_ANDROID)
#include "content/common/android/surface_texture_peer.h"
#endif

#if defined(OS_WIN)
#include <CL/OpenCL.h>
#endif

#define cl_point uint32
#define CL_SEND_IPC_MESSAGE_FAILURE 5911

#define IPC_MESSAGE_START GpuMsgStart

IPC_STRUCT_BEGIN(GPUCreateCommandBufferConfig)
  IPC_STRUCT_MEMBER(int32, share_group_id)
  IPC_STRUCT_MEMBER(std::vector<int>, attribs)
  IPC_STRUCT_MEMBER(GURL, active_url)
  IPC_STRUCT_MEMBER(gfx::GpuPreference, gpu_preference)
IPC_STRUCT_END()

#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT CONTENT_EXPORT
IPC_STRUCT_BEGIN(GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params)
  IPC_STRUCT_MEMBER(int32, surface_id)
  IPC_STRUCT_MEMBER(uint64, surface_handle)
  IPC_STRUCT_MEMBER(int32, route_id)
  IPC_STRUCT_MEMBER(std::string, mailbox_name)
  IPC_STRUCT_MEMBER(gfx::Size, size)
  IPC_STRUCT_MEMBER(float, scale_factor)
  IPC_STRUCT_MEMBER(ui::LatencyInfo, latency_info)
IPC_STRUCT_END()

IPC_STRUCT_BEGIN(GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params)
  IPC_STRUCT_MEMBER(int32, surface_id)
  IPC_STRUCT_MEMBER(uint64, surface_handle)
  IPC_STRUCT_MEMBER(int32, route_id)
  IPC_STRUCT_MEMBER(int, x)
  IPC_STRUCT_MEMBER(int, y)
  IPC_STRUCT_MEMBER(int, width)
  IPC_STRUCT_MEMBER(int, height)
  IPC_STRUCT_MEMBER(std::string, mailbox_name)
  IPC_STRUCT_MEMBER(gfx::Size, surface_size)
  IPC_STRUCT_MEMBER(float, surface_scale_factor)
  IPC_STRUCT_MEMBER(ui::LatencyInfo, latency_info)
IPC_STRUCT_END()
#undef IPC_MESSAGE_EXPORT
#define IPC_MESSAGE_EXPORT

IPC_STRUCT_BEGIN(GpuHostMsg_AcceleratedSurfaceRelease_Params)
  IPC_STRUCT_MEMBER(int32, surface_id)
IPC_STRUCT_END()

IPC_STRUCT_BEGIN(AcceleratedSurfaceMsg_BufferPresented_Params)
  IPC_STRUCT_MEMBER(std::string, mailbox_name)
  IPC_STRUCT_MEMBER(uint32, sync_point)
#if defined(OS_MACOSX)
  IPC_STRUCT_MEMBER(int32, renderer_id)
#endif
#if defined(OS_WIN)
  IPC_STRUCT_MEMBER(base::TimeTicks, vsync_timebase)
  IPC_STRUCT_MEMBER(base::TimeDelta, vsync_interval)
#endif
IPC_STRUCT_END()

IPC_STRUCT_BEGIN(GPUCommandBufferConsoleMessage)
  IPC_STRUCT_MEMBER(int32, id)
  IPC_STRUCT_MEMBER(std::string, message)
IPC_STRUCT_END()

#if defined(OS_ANDROID)
IPC_STRUCT_BEGIN(GpuStreamTextureMsg_MatrixChanged_Params)
  IPC_STRUCT_MEMBER(float, m00)
  IPC_STRUCT_MEMBER(float, m01)
  IPC_STRUCT_MEMBER(float, m02)
  IPC_STRUCT_MEMBER(float, m03)
  IPC_STRUCT_MEMBER(float, m10)
  IPC_STRUCT_MEMBER(float, m11)
  IPC_STRUCT_MEMBER(float, m12)
  IPC_STRUCT_MEMBER(float, m13)
  IPC_STRUCT_MEMBER(float, m20)
  IPC_STRUCT_MEMBER(float, m21)
  IPC_STRUCT_MEMBER(float, m22)
  IPC_STRUCT_MEMBER(float, m23)
  IPC_STRUCT_MEMBER(float, m30)
  IPC_STRUCT_MEMBER(float, m31)
  IPC_STRUCT_MEMBER(float, m32)
  IPC_STRUCT_MEMBER(float, m33)
IPC_STRUCT_END()
#endif

  IPC_STRUCT_TRAITS_BEGIN(gpu::DxDiagNode)
  IPC_STRUCT_TRAITS_MEMBER(values)
  IPC_STRUCT_TRAITS_MEMBER(children)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(gpu::GpuPerformanceStats)
  IPC_STRUCT_TRAITS_MEMBER(graphics)
  IPC_STRUCT_TRAITS_MEMBER(gaming)
  IPC_STRUCT_TRAITS_MEMBER(overall)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(gpu::GPUInfo::GPUDevice)
  IPC_STRUCT_TRAITS_MEMBER(vendor_id)
  IPC_STRUCT_TRAITS_MEMBER(device_id)
  IPC_STRUCT_TRAITS_MEMBER(vendor_string)
  IPC_STRUCT_TRAITS_MEMBER(device_string)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(gpu::GPUInfo)
  IPC_STRUCT_TRAITS_MEMBER(finalized)
  IPC_STRUCT_TRAITS_MEMBER(initialization_time)
  IPC_STRUCT_TRAITS_MEMBER(optimus)
  IPC_STRUCT_TRAITS_MEMBER(amd_switchable)
  IPC_STRUCT_TRAITS_MEMBER(lenovo_dcute)
  IPC_STRUCT_TRAITS_MEMBER(gpu)
  IPC_STRUCT_TRAITS_MEMBER(secondary_gpus)
  IPC_STRUCT_TRAITS_MEMBER(adapter_luid)
  IPC_STRUCT_TRAITS_MEMBER(driver_vendor)
  IPC_STRUCT_TRAITS_MEMBER(driver_version)
  IPC_STRUCT_TRAITS_MEMBER(driver_date)
  IPC_STRUCT_TRAITS_MEMBER(pixel_shader_version)
  IPC_STRUCT_TRAITS_MEMBER(vertex_shader_version)
  IPC_STRUCT_TRAITS_MEMBER(machine_model)
  IPC_STRUCT_TRAITS_MEMBER(gl_version)
  IPC_STRUCT_TRAITS_MEMBER(gl_version_string)
  IPC_STRUCT_TRAITS_MEMBER(gl_vendor)
  IPC_STRUCT_TRAITS_MEMBER(gl_renderer)
  IPC_STRUCT_TRAITS_MEMBER(gl_extensions)
  IPC_STRUCT_TRAITS_MEMBER(gl_ws_vendor)
  IPC_STRUCT_TRAITS_MEMBER(gl_ws_version)
  IPC_STRUCT_TRAITS_MEMBER(gl_ws_extensions)
  IPC_STRUCT_TRAITS_MEMBER(gl_reset_notification_strategy)
  IPC_STRUCT_TRAITS_MEMBER(can_lose_context)
  IPC_STRUCT_TRAITS_MEMBER(performance_stats)
  IPC_STRUCT_TRAITS_MEMBER(software_rendering)
  IPC_STRUCT_TRAITS_MEMBER(sandboxed)
#if defined(OS_WIN)
  IPC_STRUCT_TRAITS_MEMBER(dx_diagnostics)
#endif
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(content::GPUVideoMemoryUsageStats::ProcessStats)
  IPC_STRUCT_TRAITS_MEMBER(video_memory)
  IPC_STRUCT_TRAITS_MEMBER(has_duplicates)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(content::GPUVideoMemoryUsageStats)
  IPC_STRUCT_TRAITS_MEMBER(process_map)
  IPC_STRUCT_TRAITS_MEMBER(bytes_allocated)
  IPC_STRUCT_TRAITS_MEMBER(bytes_allocated_historical_max)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(content::GPUMemoryUmaStats)
  IPC_STRUCT_TRAITS_MEMBER(bytes_allocated_current)
  IPC_STRUCT_TRAITS_MEMBER(bytes_allocated_max)
  IPC_STRUCT_TRAITS_MEMBER(bytes_limit)
IPC_STRUCT_TRAITS_END()

IPC_STRUCT_TRAITS_BEGIN(gpu::MemoryAllocation)
  IPC_STRUCT_TRAITS_MEMBER(bytes_limit_when_visible)
  IPC_STRUCT_TRAITS_MEMBER(priority_cutoff_when_visible)
IPC_STRUCT_TRAITS_END()
IPC_ENUM_TRAITS(gpu::MemoryAllocation::PriorityCutoff)

IPC_STRUCT_TRAITS_BEGIN(gpu::ManagedMemoryStats)
  IPC_STRUCT_TRAITS_MEMBER(bytes_required)
  IPC_STRUCT_TRAITS_MEMBER(bytes_nice_to_have)
  IPC_STRUCT_TRAITS_MEMBER(bytes_allocated)
  IPC_STRUCT_TRAITS_MEMBER(backbuffer_requested)
IPC_STRUCT_TRAITS_END()

IPC_ENUM_TRAITS(gfx::SurfaceType)
IPC_STRUCT_TRAITS_BEGIN(gfx::GLSurfaceHandle)
  IPC_STRUCT_TRAITS_MEMBER(handle)
  IPC_STRUCT_TRAITS_MEMBER(transport_type)
  IPC_STRUCT_TRAITS_MEMBER(parent_gpu_process_id)
  IPC_STRUCT_TRAITS_MEMBER(parent_client_id)
IPC_STRUCT_TRAITS_END()

IPC_ENUM_TRAITS(content::CauseForGpuLaunch)
IPC_ENUM_TRAITS(gfx::GpuPreference)
IPC_ENUM_TRAITS(gpu::error::ContextLostReason)

IPC_ENUM_TRAITS(media::VideoCodecProfile)

IPC_STRUCT_TRAITS_BEGIN(content::GpuRenderingStats)
  IPC_STRUCT_TRAITS_MEMBER(global_texture_upload_count)
  IPC_STRUCT_TRAITS_MEMBER(global_total_texture_upload_time)
  IPC_STRUCT_TRAITS_MEMBER(texture_upload_count)
  IPC_STRUCT_TRAITS_MEMBER(total_texture_upload_time)
  IPC_STRUCT_TRAITS_MEMBER(global_total_processing_commands_time)
  IPC_STRUCT_TRAITS_MEMBER(total_processing_commands_time)
  IPC_STRUCT_TRAITS_MEMBER(global_video_memory_bytes_allocated)
IPC_STRUCT_TRAITS_END()

IPC_ENUM_TRAITS(media::VideoFrame::Format)

IPC_ENUM_TRAITS(media::VideoEncodeAccelerator::Error)

//------------------------------------------------------------------------------
// GPU Messages
// These are messages from the browser to the GPU process.

// Tells the GPU process to initialize itself. The browser explicitly
// requests this be done so that we are guaranteed that the channel is set
// up between the browser and GPU process before doing any work that might
// potentially crash the GPU process. Detection of the child process
// exiting abruptly is predicated on having the IPC channel set up.
IPC_MESSAGE_CONTROL0(GpuMsg_Initialize)

// Tells the GPU process to create a new channel for communication with a
// given client.  The channel name is returned in a
// GpuHostMsg_ChannelEstablished message.  The client ID is passed so that
// the GPU process reuses an existing channel to that process if it exists.
// This ID is a unique opaque identifier generated by the browser process.
IPC_MESSAGE_CONTROL2(GpuMsg_EstablishChannel,
                     int /* client_id */,
                     bool /* share_context */)

// Tells the GPU process to close the channel identified by IPC channel
// handle.  If no channel can be identified, do nothing.
IPC_MESSAGE_CONTROL1(GpuMsg_CloseChannel,
                     IPC::ChannelHandle /* channel_handle */)

// Tells the GPU process to create a new command buffer that renders directly
// to a native view. A corresponding GpuCommandBufferStub is created.
IPC_MESSAGE_CONTROL4(GpuMsg_CreateViewCommandBuffer,
                     gfx::GLSurfaceHandle, /* compositing_surface */
                     int32, /* surface_id */
                     int32, /* client_id */
                     GPUCreateCommandBufferConfig /* init_params */)

// Tells the GPU process to create a new image from a window. Images
// can be bound to textures using CHROMIUM_texture_from_image.
IPC_MESSAGE_CONTROL3(GpuMsg_CreateImage,
                     gfx::PluginWindowHandle, /* window */
                     int32, /* client_id */
                     int32 /* image_id */)

// Tells the GPU process to delete image.
IPC_MESSAGE_CONTROL3(GpuMsg_DeleteImage,
                     int32, /* client_id */
                     int32, /* image_id */
                     int32 /* sync_point */)

// Tells the GPU process to create a context for collecting graphics card
// information.
IPC_MESSAGE_CONTROL0(GpuMsg_CollectGraphicsInfo)

// Tells the GPU process to report video_memory information for the task manager
IPC_MESSAGE_CONTROL0(GpuMsg_GetVideoMemoryUsageStats)

// Tells the GPU process that the browser process has finished resizing the
// view.
IPC_MESSAGE_ROUTED0(AcceleratedSurfaceMsg_ResizeViewACK)

// Tells the GPU process that the browser process has handled the swap
// buffers or post sub-buffer request. A non-zero sync point means
// that we should wait for the sync point. The surface_handle identifies
// that buffer that has finished presented, i.e. the buffer being returned.
IPC_MESSAGE_ROUTED1(AcceleratedSurfaceMsg_BufferPresented,
                    AcceleratedSurfaceMsg_BufferPresented_Params)

// Tells the GPU process to remove all contexts.
IPC_MESSAGE_CONTROL0(GpuMsg_Clean)

// Tells the GPU process to crash.
IPC_MESSAGE_CONTROL0(GpuMsg_Crash)

// Tells the GPU process to hang.
IPC_MESSAGE_CONTROL0(GpuMsg_Hang)

// Tells the GPU process to disable the watchdog thread.
IPC_MESSAGE_CONTROL0(GpuMsg_DisableWatchdog)

//------------------------------------------------------------------------------
// GPU Host Messages
// These are messages to the browser.

// A renderer sends this when it wants to create a connection to the GPU
// process. The browser will create the GPU process if necessary, and will
// return a handle to the channel via a GpuChannelEstablished message.
IPC_SYNC_MESSAGE_CONTROL1_3(GpuHostMsg_EstablishGpuChannel,
                            content::CauseForGpuLaunch,
                            int /* client id */,
                            IPC::ChannelHandle /* handle to channel */,
                            gpu::GPUInfo /* stats about GPU process*/)

// A renderer sends this to the browser process when it wants to
// create a GL context associated with the given view_id.
IPC_SYNC_MESSAGE_CONTROL2_1(GpuHostMsg_CreateViewCommandBuffer,
                            int32, /* surface_id */
                            GPUCreateCommandBufferConfig, /* init_params */
                            int32 /* route_id */)

// Response from GPU to a GputMsg_Initialize message.
IPC_MESSAGE_CONTROL2(GpuHostMsg_Initialized,
                     bool /* result */,
                     ::gpu::GPUInfo /* gpu_info */)

// Response from GPU to a GpuHostMsg_EstablishChannel message.
IPC_MESSAGE_CONTROL1(GpuHostMsg_ChannelEstablished,
                     IPC::ChannelHandle /* channel_handle */)

// Message from GPU to notify to destroy the channel.
IPC_MESSAGE_CONTROL1(GpuHostMsg_DestroyChannel,
                     int32 /* client_id */)

// Message to cache the given shader information.
IPC_MESSAGE_CONTROL3(GpuHostMsg_CacheShader,
                     int32 /* client_id */,
                     std::string /* key */,
                     std::string /* shader */)

// Message to the GPU that a shader was loaded from disk.
IPC_MESSAGE_CONTROL1(GpuMsg_LoadedShader,
                     std::string /* encoded shader */)

// Respond from GPU to a GpuMsg_CreateViewCommandBuffer message.
IPC_MESSAGE_CONTROL1(GpuHostMsg_CommandBufferCreated,
                     int32 /* route_id */)

// Request from GPU to free the browser resources associated with the
// command buffer.
IPC_MESSAGE_CONTROL1(GpuHostMsg_DestroyCommandBuffer,
                     int32 /* surface_id */)

// Response from GPU to a GpuMsg_CreateImage message.
IPC_MESSAGE_CONTROL1(GpuHostMsg_ImageCreated,
                     gfx::Size /* size */)

// Response from GPU to a GpuMsg_CollectGraphicsInfo.
IPC_MESSAGE_CONTROL1(GpuHostMsg_GraphicsInfoCollected,
                     gpu::GPUInfo /* GPU logging stats */)

// Response from GPU to a GpuMsg_GetVideoMemory.
IPC_MESSAGE_CONTROL1(GpuHostMsg_VideoMemoryUsageStats,
                     content::GPUVideoMemoryUsageStats /* GPU memory stats */)

// Message from GPU to add a GPU log message to the about:gpu page.
IPC_MESSAGE_CONTROL3(GpuHostMsg_OnLogMessage,
                     int /*severity*/,
                     std::string /* header */,
                     std::string /* message */)

// Resize the window that is being drawn into. It's important that this
// resize be synchronized with the swapping of the front and back buffers.
IPC_MESSAGE_CONTROL3(GpuHostMsg_ResizeView,
                     int32 /* surface_id */,
                     int32 /* route_id */,
                     gfx::Size /* size */)

// Tells the browser that a frame with the specific latency info was drawn to
// the screen
IPC_MESSAGE_CONTROL1(GpuHostMsg_FrameDrawn,
                     ui::LatencyInfo /* latency_info */)

// Same as above with a rect of the part of the surface that changed.
IPC_MESSAGE_CONTROL1(GpuHostMsg_AcceleratedSurfaceBuffersSwapped,
                     GpuHostMsg_AcceleratedSurfaceBuffersSwapped_Params)

// This message notifies the browser process that the renderer
// swapped a portion of the buffers associated with the given "window", which
// should cause the browser to redraw the compositor's contents.
IPC_MESSAGE_CONTROL1(GpuHostMsg_AcceleratedSurfacePostSubBuffer,
                     GpuHostMsg_AcceleratedSurfacePostSubBuffer_Params)

// Tells the browser to release whatever resources are associated with
// the given surface. The browser must send an ACK once this operation
// is complete.
IPC_MESSAGE_CONTROL1(GpuHostMsg_AcceleratedSurfaceRelease,
                     GpuHostMsg_AcceleratedSurfaceRelease_Params)

// Tells the browser to release resources for the given surface until the next
// time swap buffers or post sub buffer is sent.
IPC_MESSAGE_CONTROL1(GpuHostMsg_AcceleratedSurfaceSuspend,
                     int32 /* surface_id */)

// Tells the browser about updated parameters for vsync alignment.
IPC_MESSAGE_CONTROL3(GpuHostMsg_UpdateVSyncParameters,
                     int32 /* surface_id */,
                     base::TimeTicks /* timebase */,
                     base::TimeDelta /* interval */)

IPC_MESSAGE_CONTROL1(GpuHostMsg_DidCreateOffscreenContext,
                     GURL /* url */)

IPC_MESSAGE_CONTROL3(GpuHostMsg_DidLoseContext,
                     bool /* offscreen */,
                     gpu::error::ContextLostReason /* reason */,
                     GURL /* url */)

IPC_MESSAGE_CONTROL1(GpuHostMsg_DidDestroyOffscreenContext,
                     GURL /* url */)

// Tells the browser about GPU memory usage statistics for UMA logging.
IPC_MESSAGE_CONTROL1(GpuHostMsg_GpuMemoryUmaStats,
                     content::GPUMemoryUmaStats /* GPU memory UMA stats */)

//------------------------------------------------------------------------------
// GPU Channel Messages
// These are messages from a renderer process to the GPU process.

// Tells the GPU process to create a new command buffer that renders to an
// offscreen frame buffer.
IPC_SYNC_MESSAGE_CONTROL2_1(GpuChannelMsg_CreateOffscreenCommandBuffer,
                            gfx::Size, /* size */
                            GPUCreateCommandBufferConfig, /* init_params */
                            int32 /* route_id */)

// The CommandBufferProxy sends this to the GpuCommandBufferStub in its
// destructor, so that the stub deletes the actual CommandBufferService
// object that it's hosting.
IPC_SYNC_MESSAGE_CONTROL1_0(GpuChannelMsg_DestroyCommandBuffer,
                            int32 /* instance_id */)

// Generates n new unique mailbox names synchronously.
IPC_SYNC_MESSAGE_CONTROL1_1(GpuChannelMsg_GenerateMailboxNames,
                            unsigned, /* num */
                            std::vector<gpu::Mailbox> /* mailbox_names */)

// Generates n new unique mailbox names asynchronously.
IPC_MESSAGE_CONTROL1(GpuChannelMsg_GenerateMailboxNamesAsync,
                     unsigned /* num */)

// Reply to GpuChannelMsg_GenerateMailboxNamesAsync.
IPC_MESSAGE_CONTROL1(GpuChannelMsg_GenerateMailboxNamesReply,
                     std::vector<gpu::Mailbox> /* mailbox_names */)

// Create a new GPU-accelerated video encoder.
IPC_SYNC_MESSAGE_CONTROL0_1(GpuChannelMsg_CreateVideoEncoder,
                            int32 /* route_id */)

IPC_MESSAGE_CONTROL1(GpuChannelMsg_DestroyVideoEncoder, int32 /* route_id */)

#if defined(OS_ANDROID)
// Register the StreamTextureProxy class with the GPU process, so that
// the renderer process will get notified whenever a frame becomes available.
IPC_SYNC_MESSAGE_CONTROL1_1(GpuChannelMsg_RegisterStreamTextureProxy,
                            int32, /* stream_id */
                            int /* route_id */)

// Tells the GPU process create and send the java surface texture object to
// the renderer process through the binder thread.
IPC_MESSAGE_CONTROL3(GpuChannelMsg_EstablishStreamTexture,
                     int32, /* stream_id */
                     int32, /* primary_id */
                     int32 /* secondary_id */)

// Tells the GPU process to set the size of StreamTexture from the given
// stream Id.
IPC_MESSAGE_CONTROL2(GpuChannelMsg_SetStreamTextureSize,
                     int32, /* stream_id */
                     gfx::Size /* size */)
#endif

// Tells the GPU process to collect rendering stats.
IPC_SYNC_MESSAGE_CONTROL1_1(GpuChannelMsg_CollectRenderingStatsForSurface,
                            int32 /* surface_id */,
                            content::GpuRenderingStats /* stats */)

#if defined(OS_ANDROID)
//------------------------------------------------------------------------------
// Stream Texture Messages
// Inform the renderer that a new frame is available.
IPC_MESSAGE_ROUTED0(GpuStreamTextureMsg_FrameAvailable)

// Inform the renderer process that the transform matrix has changed.
IPC_MESSAGE_ROUTED1(GpuStreamTextureMsg_MatrixChanged,
                    GpuStreamTextureMsg_MatrixChanged_Params /* params */)
#endif

//------------------------------------------------------------------------------
// GPU Command Buffer Messages
// These are messages between a renderer process to the GPU process relating to
// a single OpenGL context.
// Initialize a command buffer with the given number of command entries.
// Returns the shared memory handle for the command buffer mapped to the
// calling process.
IPC_SYNC_MESSAGE_ROUTED1_1(GpuCommandBufferMsg_Initialize,
                           base::SharedMemoryHandle /* shared_state */,
                           bool /* result */)

// Sets the shared memory buffer used for commands.
IPC_SYNC_MESSAGE_ROUTED1_0(GpuCommandBufferMsg_SetGetBuffer,
                           int32 /* shm_id */)

// Produces the front buffer into a mailbox. This allows another context to draw
// the output of this context.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_ProduceFrontBuffer,
                    gpu::Mailbox /* mailbox */)

// Get the current state of the command buffer.
IPC_SYNC_MESSAGE_ROUTED0_1(GpuCommandBufferMsg_GetState,
                           gpu::CommandBuffer::State /* state */)

// Get the current state of the command buffer, as fast as possible.
IPC_SYNC_MESSAGE_ROUTED0_1(GpuCommandBufferMsg_GetStateFast,
                           gpu::CommandBuffer::State /* state */)

// Asynchronously synchronize the put and get offsets of both processes.
// Caller passes its current put offset. Current state (including get offset)
// is returned in shared memory.
IPC_MESSAGE_ROUTED2(GpuCommandBufferMsg_AsyncFlush,
                    int32 /* put_offset */,
                    uint32 /* flush_count */)

// Sends information about the latency of the current frame to the GPU
// process.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_SetLatencyInfo,
                    ui::LatencyInfo /* latency_info */)

// Asynchronously process any commands known to the GPU process. This is only
// used in the event that a channel is unscheduled and needs to be flushed
// again to process any commands issued subsequent to unscheduling. The GPU
// process actually sends it (deferred) to itself.
IPC_MESSAGE_ROUTED0(GpuCommandBufferMsg_Rescheduled)

// Sent by the GPU process to display messages in the console.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_ConsoleMsg,
                    GPUCommandBufferConsoleMessage /* msg */)

// Register an existing shared memory transfer buffer. The id that can be
// used to identify the transfer buffer from a command buffer.
IPC_MESSAGE_ROUTED3(GpuCommandBufferMsg_RegisterTransferBuffer,
                    int32 /* id */,
                    base::SharedMemoryHandle /* transfer_buffer */,
                    uint32 /* size */)

// Destroy a previously created transfer buffer.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_DestroyTransferBuffer,
                    int32 /* id */)

// Get the shared memory handle for a transfer buffer mapped to the callers
// process.
IPC_SYNC_MESSAGE_ROUTED1_2(GpuCommandBufferMsg_GetTransferBuffer,
                           int32 /* id */,
                           base::SharedMemoryHandle /* transfer_buffer */,
                           uint32 /* size */)

// Create and initialize a hardware video decoder, returning its new route_id.
// Created decoders should be freed with AcceleratedVideoDecoderMsg_Destroy when
// no longer needed.
IPC_SYNC_MESSAGE_ROUTED1_1(GpuCommandBufferMsg_CreateVideoDecoder,
                           media::VideoCodecProfile /* profile */,
                           int /* route_id */)

// Tells the proxy that there was an error and the command buffer had to be
// destroyed for some reason.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_Destroyed,
                    gpu::error::ContextLostReason /* reason */)

// Request that the GPU process reply with the given message. Reply may be
// delayed.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_Echo,
                    IPC::Message /* reply */)

// Response to a GpuChannelMsg_Echo message.
IPC_MESSAGE_ROUTED0(GpuCommandBufferMsg_EchoAck)

// Send to stub on surface visibility change.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_SetSurfaceVisible, bool /* visible */)

IPC_MESSAGE_ROUTED0(GpuCommandBufferMsg_DiscardBackbuffer)
IPC_MESSAGE_ROUTED0(GpuCommandBufferMsg_EnsureBackbuffer)

// Sent to proxy when the gpu memory manager changes its memory allocation.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_SetMemoryAllocation,
                    gpu::MemoryAllocation /* allocation */)

// Sent to stub from the proxy with statistics on managed memory usage and
// requirements.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_SendClientManagedMemoryStats,
                    gpu::ManagedMemoryStats /* stats */)

// Sent to stub when proxy is assigned a memory allocation changed callback.
IPC_MESSAGE_ROUTED1(
    GpuCommandBufferMsg_SetClientHasMemoryAllocationChangedCallback,
    bool /* has_callback */)

// Inserts a sync point into the channel. This is handled on the IO thread, so
// can be expected to be reasonably fast, but the sync point is actually
// retired in order with respect to the other calls. The sync point is shared
// across channels.
IPC_SYNC_MESSAGE_ROUTED0_1(GpuCommandBufferMsg_InsertSyncPoint,
                           uint32 /* sync_point */)

// Retires the sync point. Note: this message is not sent explicitly by the
// renderer, but is synthesized by the GPU process.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_RetireSyncPoint,
                    uint32 /* sync_point */)

// Makes this command buffer signal when a sync point is reached, by sending
// back a GpuCommandBufferMsg_SignalSyncPointAck message with the same
// signal_id.
IPC_MESSAGE_ROUTED2(GpuCommandBufferMsg_SignalSyncPoint,
                    uint32 /* sync_point */,
                    uint32 /* signal_id */)

// Response to GpuCommandBufferMsg_SignalSyncPoint.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_SignalSyncPointAck,
                    uint32 /* signal_id */)

// Makes this command buffer signal when a query is reached, by sending
// back a GpuCommandBufferMsg_SignalSyncPointAck message with the same
// signal_id.
IPC_MESSAGE_ROUTED2(GpuCommandBufferMsg_SignalQuery,
                    uint32 /* query */,
                    uint32 /* signal_id */)

// Register an existing gpu memory buffer. The id that can be
// used to identify the gpu memory buffer from a command buffer.
IPC_MESSAGE_ROUTED5(GpuCommandBufferMsg_RegisterGpuMemoryBuffer,
                    int32 /* id */,
                    gfx::GpuMemoryBufferHandle /* gpu_memory_buffer */,
                    uint32 /* width */,
                    uint32 /* height */,
                    uint32 /* internalformat */)

// Destroy a previously created gpu memory buffer.
IPC_MESSAGE_ROUTED1(GpuCommandBufferMsg_DestroyGpuMemoryBuffer,
                    int32 /* id */)

//------------------------------------------------------------------------------
// Accelerated Video Decoder Messages
// These messages are sent from Renderer process to GPU process.

// Send input buffer for decoding.
IPC_MESSAGE_ROUTED3(AcceleratedVideoDecoderMsg_Decode,
                    base::SharedMemoryHandle, /* input_buffer_handle */
                    int32, /* bitstream_buffer_id */
                    uint32) /* size */

// Sent from Renderer process to the GPU process to give the texture IDs for
// the textures the decoder will use for output.
IPC_MESSAGE_ROUTED2(AcceleratedVideoDecoderMsg_AssignPictureBuffers,
                    std::vector<int32>,  /* Picture buffer ID */
                    std::vector<uint32>) /* Texture ID */

// Send from Renderer process to the GPU process to recycle the given picture
// buffer for further decoding.
IPC_MESSAGE_ROUTED1(AcceleratedVideoDecoderMsg_ReusePictureBuffer,
                    int32) /* Picture buffer ID */

// Send flush request to the decoder.
IPC_MESSAGE_ROUTED0(AcceleratedVideoDecoderMsg_Flush)

// Send reset request to the decoder.
IPC_MESSAGE_ROUTED0(AcceleratedVideoDecoderMsg_Reset)

// Send destroy request to the decoder.
IPC_MESSAGE_ROUTED0(AcceleratedVideoDecoderMsg_Destroy)

//------------------------------------------------------------------------------
// Accelerated Video Decoder Host Messages
// These messages are sent from GPU process to Renderer process.
// Inform AcceleratedVideoDecoderHost that AcceleratedVideoDecoder has been
// created.

// Accelerated video decoder has consumed input buffer from transfer buffer.
IPC_MESSAGE_ROUTED1(AcceleratedVideoDecoderHostMsg_BitstreamBufferProcessed,
                    int32) /* Processed buffer ID */

// Allocate video frames for output of the hardware video decoder.
IPC_MESSAGE_ROUTED3(
    AcceleratedVideoDecoderHostMsg_ProvidePictureBuffers,
    int32, /* Number of video frames to generate */
    gfx::Size, /* Requested size of buffer */
    uint32 ) /* Texture target */

// Decoder reports that a picture is ready and buffer does not need to be passed
// back to the decoder.
IPC_MESSAGE_ROUTED1(AcceleratedVideoDecoderHostMsg_DismissPictureBuffer,
                    int32) /* Picture buffer ID */

// Decoder reports that a picture is ready.
IPC_MESSAGE_ROUTED2(AcceleratedVideoDecoderHostMsg_PictureReady,
                    int32,  /* Picture buffer ID */
                    int32)  /* Bitstream buffer ID */

// Confirm decoder has been flushed.
IPC_MESSAGE_ROUTED0(AcceleratedVideoDecoderHostMsg_FlushDone)

// Confirm decoder has been reset.
IPC_MESSAGE_ROUTED0(AcceleratedVideoDecoderHostMsg_ResetDone)

// Video decoder has encountered an error.
IPC_MESSAGE_ROUTED1(AcceleratedVideoDecoderHostMsg_ErrorNotification,
                    uint32) /* Error ID */

//------------------------------------------------------------------------------
// Accelerated Video Encoder Messages
// These messages are sent from the Renderer process to GPU process.

// Initialize the accelerated encoder.
IPC_MESSAGE_ROUTED4(AcceleratedVideoEncoderMsg_Initialize,
                    media::VideoFrame::Format /* input_format */,
                    gfx::Size /* input_visible_size */,
                    media::VideoCodecProfile /* output_profile */,
                    uint32 /* initial_bitrate */)

// Queue a input buffer to the encoder to encode. |frame_id| will be returned by
// AcceleratedVideoEncoderHostMsg_NotifyEncodeDone.
IPC_MESSAGE_ROUTED4(AcceleratedVideoEncoderMsg_Encode,
                    int32 /* frame_id */,
                    base::SharedMemoryHandle /* buffer_handle */,
                    uint32 /* buffer_size */,
                    bool /* force_keyframe */)

// Queue a buffer to the encoder for use in returning output.  |buffer_id| will
// be returned by AcceleratedVideoEncoderHostMsg_BitstreamBufferReady.
IPC_MESSAGE_ROUTED3(AcceleratedVideoEncoderMsg_UseOutputBitstreamBuffer,
                    int32 /* buffer_id */,
                    base::SharedMemoryHandle /* buffer_handle */,
                    uint32 /* buffer_size */)

// Request a runtime encoding parameter change.
IPC_MESSAGE_ROUTED2(AcceleratedVideoEncoderMsg_RequestEncodingParametersChange,
                    uint32 /* bitrate */,
                    uint32 /* framerate */)

//------------------------------------------------------------------------------
// Accelerated Video Encoder Host Messages
// These messages are sent from GPU process to Renderer process.

// Notify of the completion of initialization.
IPC_MESSAGE_ROUTED0(AcceleratedVideoEncoderHostMsg_NotifyInitializeDone)

// Notify renderer of the input/output buffer requirements of the encoder.
IPC_MESSAGE_ROUTED3(AcceleratedVideoEncoderHostMsg_RequireBitstreamBuffers,
                    uint32 /* input_count */,
                    gfx::Size /* input_coded_size */,
                    uint32 /* output_buffer_size */)

// Notify the renderer that the encoder has finished using an input buffer.
// There is no congruent entry point in the media::VideoEncodeAccelerator
// interface, in VEA this same done condition is indicated by dropping the
// reference to the media::VideoFrame passed to VEA::Encode().
IPC_MESSAGE_ROUTED1(AcceleratedVideoEncoderHostMsg_NotifyInputDone,
                    int32 /* frame_id */)

// Notify the renderer that an output buffer has been filled with encoded data.
IPC_MESSAGE_ROUTED3(AcceleratedVideoEncoderHostMsg_BitstreamBufferReady,
                    int32 /* bitstream_buffer_id */,
                    uint32 /* payload_size */,
                    bool /* key_frame */)

// Report error condition.
IPC_MESSAGE_ROUTED1(AcceleratedVideoEncoderHostMsg_NotifyError,
                    media::VideoEncodeAccelerator::Error /* error */)

//------------------------------------------------------------------------------
// OpenCL Channel Messages
// These are messages from a renderer process to the OpenCL/GPU process.
// Calling OpenCL API from a renderer process, then run in OpenCL/GPU process.

// Call and respond OpenCL API clGetPlatformIDs using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL2_3(OpenCLChannelMsg_GetPlatformIDs,
                            cl_uint,
                            std::vector<bool>,
                            std::vector<cl_point>,
                            cl_uint,
                            cl_int)

// Call and respond OpenCL API clGetDeviceIDs using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetDeviceIDs,
                            cl_point,
                            cl_device_type,
                            cl_uint,
                            std::vector<bool>,
                            std::vector<cl_point>,
                            cl_uint,
                            cl_int)

// Call and respond OpenCL API clCreateSubDevices using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_CreateSubDevices,
                            cl_point,
                            std::vector<cl_device_partition_property>,
                            cl_uint,
                            std::vector<bool>,
                            std::vector<cl_point>,
                            cl_uint,
                            cl_int)

// Call and respond OpenCL API clRetainDevice using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_RetainDevice,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clReleaseDevice using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_ReleaseDevice,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clCreateContext using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_CreateContext,
                            std::vector<cl_context_properties>,
                            cl_uint,
                            std::vector<cl_point>,
                            std::vector<cl_point>,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clCreateContextFromType using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_CreateContextFromType,
                            std::vector<cl_context_properties>,
                            cl_device_type,
                            cl_point,
                            cl_point,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clRetainContext using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_RetainContext,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clReleaseContext using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_ReleaseContext,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clCreateCommandQueue using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_2(OpenCLChannelMsg_CreateCommandQueue,
                            cl_point,
                            cl_point,
                            cl_command_queue_properties,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clRetainCommandQueue using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_RetainCommandQueue,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clReleaseCommandQueue using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_ReleaseCommandQueue,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clCreateBuffer using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_CreateBuffer,
                            cl_point,
                            cl_mem_flags,
                            size_t,
                            cl_point,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clCreateSubBuffer using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_CreateSubBuffer,
                            cl_point,
                            cl_mem_flags,
                            cl_buffer_create_type,
                            cl_point,std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clChannelMsg_CreateImage
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_CreateImage,
                            cl_point,
                            cl_mem_flags,
                            std::vector<cl_uint>,
                            cl_point,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clRetainMemObject using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_RetainMemObject,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clReleaseMemObject using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_ReleaseMemObject,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clGetSupportedImageFormats
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetSupportedImageFormats,
                            cl_point,
                            cl_mem_flags,
                            cl_mem_object_type,
                            cl_uint,
                            std::vector<bool>,
                            std::vector<cl_uint>,
                            cl_uint,
                            cl_int)

// Call and respond OpenCL API clSetMemObjectDestructorCallback
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL3_1(OpenCLChannelMsg_SetMemObjectDestructorCallback,
                            cl_point,
                            cl_point,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clCreateSampler using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_CreateSampler,
                            cl_point,
                            cl_bool,
                            cl_addressing_mode,
                            cl_filter_mode,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clRetainSampler using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_RetainSampler,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clReleaseSampler using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_ReleaseSampler,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clCreateProgramWithSource
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_CreateProgramWithSource,
                            cl_point,
                            cl_uint,
                            std::vector<std::string>,
                            std::vector<size_t>,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clCreateProgramWithBinary
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_CreateProgramWithBinary,
                            cl_point,
                            cl_uint,
                            std::vector<cl_point>,
                            std::vector<size_t>,
                            std::vector<std::vector<unsigned char>>,
                            std::vector<cl_int>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clCreateProgramWithBuiltInKernels
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_2(OpenCLChannelMsg_CreateProgramWithBuiltInKernels,
                            cl_point,
                            cl_uint,
                            std::vector<cl_point>,
                            std::string,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clRetainProgram using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_RetainProgram,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clReleaseProgram using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_ReleaseProgram,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clBuildProgram using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_1(OpenCLChannelMsg_BuildProgram,
                            cl_point,
                            cl_uint,
                            std::vector<cl_point>,
                            std::string,
                            std::vector<cl_point>,
                            cl_int)

// Call and respond OpenCL API clCompileProgram using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_1(OpenCLChannelMsg_CompileProgram,
                            std::vector<cl_point>,
                            std::vector<cl_uint>,
                            std::vector<cl_point>,
                            std::vector<std::string>,
                            std::vector<cl_point>,
                            cl_int)

// Call and respond OpenCL API clLinkProgram using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_LinkProgram,
                            std::vector<cl_point>,
                            std::vector<cl_uint>,
                            std::vector<cl_point>,
                            std::vector<cl_point>,
                            std::string,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API UnloadPlatformCompiler using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_UnloadPlatformCompiler,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clCreateKernel using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL3_2(OpenCLChannelMsg_CreateKernel,
                            cl_point,
                            std::string,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clCreateKernelsInProgram
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_2(OpenCLChannelMsg_CreateKernelsInProgram,
                            cl_point,
                            cl_uint,
                            std::vector<cl_point>,
                            std::vector<bool>,
                            cl_uint,
                            cl_int)

// Call and respond OpenCL API clRetainKernel using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_RetainKernel,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clReleaseKernel using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_ReleaseKernel,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clSetKernelArg using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_1(OpenCLChannelMsg_SetKernelArg,
                            cl_point,
                            cl_uint,
                            size_t,
                            cl_point,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL3_1(OpenCLChannelMsg_SetKernelArg_vector,
                            cl_point,
                            cl_uint,
                            std::vector<unsigned char>,
                            cl_int)

// Call and respond OpenCL API clWaitForEvents using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL2_1(OpenCLChannelMsg_WaitForEvents,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_int)

// Call and respond OpenCL API clCreateUserEvent using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL2_2(OpenCLChannelMsg_CreateUserEvent,
                            cl_point,
                            std::vector<bool>,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clRetainEvent using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_RetainEvent,
                            cl_point, cl_int)

// Call and respond OpenCL API clReleaseEvent using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_ReleaseEvent,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clSetUserEventStatus using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL2_1(OpenCLChannelMsg_SetUserEventStatus,
                            cl_point,
                            cl_int,
                            cl_int)

// Call and respond OpenCL API clSetEventCallback using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_1(OpenCLChannelMsg_SetEventCallback,
                            cl_point,
                            cl_int,
                            cl_point,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clFlush using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_Flush,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clFinish using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL1_1(OpenCLChannelMsg_Finish,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueReadBuffer using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_EnqueueReadBuffer,
                            std::vector<cl_point>,
                            cl_bool,
                            std::vector<size_t>,
                            cl_uint,
                            std::vector<unsigned char>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueReadBufferRect using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueReadBufferRect,
                            std::vector<cl_point>,
                            cl_bool,
                            std::vector<size_t>,
                            cl_point,
                            cl_uint,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueWriteBuffer using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueWriteBuffer,
                            std::vector<cl_point>,
                            cl_bool,
                            std::vector<size_t>,
                            std::vector<unsigned char>,
                            cl_uint,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueWriteBufferRect
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueWriteBufferRect,
                            std::vector<cl_point>,
                            cl_bool,
                            std::vector<size_t>,
                            cl_point,
                            cl_uint,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueFillBuffer using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueFillBuffer,
                            std::vector<cl_point>,
                            cl_point,
                            std::vector<size_t>,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueCopyBuffer using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_2(OpenCLChannelMsg_EnqueueCopyBuffer,
                            std::vector<cl_point>,
                            std::vector<size_t>,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueCopyBufferRect using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueCopyBufferRect,
                            std::vector<cl_point>,
                            std::vector<size_t>,
                            std::vector<size_t>,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueReadImage using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueReadImage,
                            std::vector<cl_point>,
                            cl_bool,
                            std::vector<size_t>,
                            cl_point,
                            cl_uint,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueWriteImage using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueWriteImage,
                            std::vector<cl_point>,
                            cl_bool,
                            std::vector<size_t>,
                            cl_point,
                            cl_uint,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueFillImage using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueFillImage,
                            std::vector<cl_point>,
                            cl_point,
                            std::vector<size_t>,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueCopyImage using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_2(OpenCLChannelMsg_EnqueueCopyImage,
                            std::vector<cl_point>,
                            std::vector<size_t>,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueCopyImageToBuffer
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueCopyImageToBuffer,
                            std::vector<cl_point>,
                            std::vector<size_t>,
                            size_t,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueCopyBufferToImage
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueCopyBufferToImage,
                            std::vector<cl_point>,
                            size_t,
                            std::vector<size_t>,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueMapBuffer using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_EnqueueMapBuffer,
                            std::vector<cl_point>,
                            cl_bool,
                            cl_map_flags,
                            std::vector<size_t>,
                            cl_point,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clEnqueueMapImage using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_EnqueueMapImage,
                            std::vector<cl_point>,
                            cl_bool,
                            cl_map_flags,
                            std::vector<size_t>,
                            cl_uint,
                            cl_point,
                            cl_int,
                            cl_point)

// Call and respond OpenCL API clUnmapMemObject using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueUnmapMemObject,
                            cl_point,
                            cl_point,
                            cl_point,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueMigrateMemObjects
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueMigrateMemObjects,
                            cl_point,
                            std::vector<cl_uint>,
                            std::vector<cl_point>,
                            cl_mem_migration_flags,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueNDRangeKernel using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_2(OpenCLChannelMsg_EnqueueNDRangeKernel,
                            std::vector<cl_point>,
                            cl_int,
                            std::vector<size_t>,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueTask using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_2(OpenCLChannelMsg_EnqueueTask,
                            cl_point,
                            cl_point,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueNativeKernel using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_EnqueueNativeKernel,
                            std::vector<cl_point>,
                            size_t,
                            std::vector<cl_uint>,
                            cl_point,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueMarkerWithWaitList
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL3_2(OpenCLChannelMsg_EnqueueMarkerWithWaitList,
                            cl_point,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clEnqueueBarrierWithWaitList
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL3_2(OpenCLChannelMsg_EnqueueBarrierWithWaitList,
                            cl_point,
                            cl_uint,
                            std::vector<cl_point>,
                            cl_point,
                            cl_int)

// Call and respond OpenCL API clGetPlatformInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetPlatformInfo_string,
                            cl_point,
                            cl_platform_info,
                            size_t,
                            std::vector<bool>,
                            std::string,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetDeviceInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetDeviceInfo_cl_uint,
                            cl_point,
                            cl_device_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetDeviceInfo_size_t_list,
                            cl_point,
                            cl_device_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<size_t>,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetDeviceInfo_size_t,
                            cl_point,
                            cl_device_info,
                            size_t,
                            std::vector<bool>,
                            size_t,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetDeviceInfo_cl_ulong,
                            cl_point,
                            cl_device_info,
                            size_t,
                            std::vector<bool>,
                            cl_ulong,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetDeviceInfo_string,
                            cl_point,
                            cl_device_info,
                            size_t,
                            std::vector<bool>,
                            std::string,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetDeviceInfo_cl_point,
                            cl_point,
                            cl_device_info,
                            size_t,
                            std::vector<bool>,
                            cl_point,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetDeviceInfo_intptr_t_list,
                            cl_point,
                            cl_device_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<intptr_t>,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetContextInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetContextInfo_cl_uint,
                            cl_point,
                            cl_context_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetContextInfo_cl_point_list,
                            cl_point,
                            cl_context_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<cl_point>,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetContextInfo_intptr_t_list,
                            cl_point,
                            cl_context_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<intptr_t>,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetCommandQueueInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetCommandQueueInfo_cl_point,
                            cl_point,
                            cl_command_queue_info,
                            size_t,
                            std::vector<bool>,
                            cl_point,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetCommandQueueInfo_cl_uint,
                            cl_point,
                            cl_command_queue_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetCommandQueueInfo_cl_ulong,
                            cl_point,
                            cl_command_queue_info,
                            size_t,
                            std::vector<bool>,
                            cl_ulong,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetMemObjectInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetMemObjectInfo_cl_uint,
                            cl_point,
                            cl_mem_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetMemObjectInfo_cl_ulong,
                            cl_point,
                            cl_mem_info,
                            size_t,
                            std::vector<bool>,
                            cl_ulong,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetMemObjectInfo_size_t,
                            cl_point,
                            cl_mem_info,
                            size_t,
                            std::vector<bool>,
                            size_t,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetMemObjectInfo_cl_point,
                            cl_point,
                            cl_mem_info,
                            size_t,
                            std::vector<bool>,
                            cl_point,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetImageInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetImageInfo_cl_image_format,
                            cl_point,
                            cl_image_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<cl_uint>,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetImageInfo_size_t,
                            cl_point,
                            cl_image_info,
                            size_t,
                            std::vector<bool>,
                            size_t,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetImageInfo_cl_point,
                            cl_point,
                            cl_image_info,
                            size_t,
                            std::vector<bool>,
                            cl_point,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetImageInfo_cl_uint,
                            cl_point,
                            cl_image_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetSamplerInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetSamplerInfo_cl_uint,
                            cl_point,
                            cl_sampler_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetSamplerInfo_cl_point,
                            cl_point,
                            cl_sampler_info,
                            size_t,
                            std::vector<bool>,
                            cl_point,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetProgramInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetProgramInfo_cl_uint,
                            cl_point,
                            cl_program_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetProgramInfo_cl_point,
                            cl_point,
                            cl_program_info,
                            size_t,
                            std::vector<bool>,
                            cl_point,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetProgramInfo_cl_point_list,
                            cl_point,
                            cl_program_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<cl_point>,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetProgramInfo_string,
                            cl_point,
                            cl_program_info,
                            size_t,
                            std::vector<bool>,
                            std::string,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetProgramInfo_size_t_list,
                            cl_point,
                            cl_program_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<size_t>,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetProgramInfo_string_list,
                            cl_point,
                            cl_program_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<std::string>,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetProgramInfo_size_t,
                            cl_point,
                            cl_program_info,
                            size_t,
                            std::vector<bool>,
                            size_t,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetProgramBuildInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetProgramBuildInfo_cl_int,
                            cl_point,
                            cl_point,
                            cl_program_build_info,
                            size_t,
                            std::vector<bool>,
                            cl_int,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetProgramBuildInfo_string,
                            cl_point,
                            cl_point,
                            cl_program_build_info,
                            size_t,
                            std::vector<bool>,
                            std::string,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetProgramBuildInfo_cl_uint,
                            cl_point,
                            cl_point,
                            cl_program_build_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetKernelInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetKernelInfo_string,
                            cl_point,
                            cl_kernel_info,
                            size_t,
                            std::vector<bool>,
                            std::string,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetKernelInfo_cl_uint,
                            cl_point,
                            cl_kernel_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetKernelInfo_cl_point,
                            cl_point,
                            cl_kernel_info,
                            size_t,
                            std::vector<bool>,
                            cl_point,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetKernelArgInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetKernelArgInfo_cl_uint,
                            cl_point,
                            cl_uint,
                            cl_kernel_arg_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetKernelArgInfo_string,
                            cl_point,
                            cl_uint,
                            cl_kernel_arg_info,
                            size_t,
                            std::vector<bool>,
                            std::string,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetKernelArgInfo_cl_ulong,
                            cl_point,
                            cl_uint,
                            cl_kernel_arg_info,
                            size_t,
                            std::vector<bool>,
                            cl_ulong,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetKernelWorkGroupInfo
// using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL5_3(
    OpenCLChannelMsg_GetKernelWorkGroupInfo_size_t_list,
                            cl_point,
                            cl_point,
                            cl_kernel_work_group_info,
                            size_t,
                            std::vector<bool>,
                            std::vector<size_t>,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetKernelWorkGroupInfo_size_t,
                            cl_point,
                            cl_point,
                            cl_kernel_work_group_info,
                            size_t,
                            std::vector<bool>,
                            size_t,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL5_3(OpenCLChannelMsg_GetKernelWorkGroupInfo_cl_ulong,
                            cl_point,
                            cl_point,
                            cl_kernel_work_group_info,
                            size_t,
                            std::vector<bool>,
                            cl_ulong,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetEventInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetEventInfo_cl_point,
                            cl_point,
                            cl_event_info,
                            size_t,
                            std::vector<bool>,
                            cl_point,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetEventInfo_cl_uint,
                            cl_point,
                            cl_event_info,
                            size_t,
                            std::vector<bool>,
                            cl_uint,
                            size_t,
                            cl_int)

IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetEventInfo_cl_int,
                            cl_point,
                            cl_event_info,
                            size_t,
                            std::vector<bool>,
                            cl_int,
                            size_t,
                            cl_int)

// Call and respond OpenCL API clGetEventProfilingInfo using Sync IPC Message
IPC_SYNC_MESSAGE_CONTROL4_3(OpenCLChannelMsg_GetEventProfilingInfo_cl_ulong,
                            cl_point,
                            cl_profiling_info,
                            size_t,
                            std::vector<bool>,
                            cl_ulong,
                            size_t,
                            cl_int)

//ScalableVision	

IPC_SYNC_MESSAGE_CONTROL3_2(OpenCLChannelMsg_CreateFromGLBuffer,
                            cl_point, // context
                            cl_uint, // flags
                            cl_uint, // bufobj
                            cl_int,  // errcode_ret
							cl_point) // ret

IPC_SYNC_MESSAGE_CONTROL5_2(OpenCLChannelMsg_CreateFromGLTexture,
							cl_point      /* context */,
							cl_uint    /* flags */,
							cl_uint       /* target */,
							cl_GLint        /* miplevel */,
							cl_GLuint       /* texture */,
							cl_int        /* errcode_ret */,
							cl_point) // func ret

IPC_SYNC_MESSAGE_CONTROL3_2(OpenCLChannelMsg_EnqueueAcquireGLObjects,
	cl_point, // cmdqueue
	std::vector<cl_point>, // mem_objects
	std::vector<cl_point>, // event_wait_list
	cl_point, // event ret
	cl_int) // func ret

IPC_SYNC_MESSAGE_CONTROL3_2(OpenCLChannelMsg_EnqueueReleaseGLObjects,
	cl_point, // cmdqueue
	std::vector<cl_point>, // mem_objects
	std::vector<cl_point>, // event_wait_list
	cl_point, // event ret
	cl_int) // func ret
