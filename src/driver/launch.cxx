// SPDX-License-Identifier: MIT
#include <iostream>
#include <shared_mutex>

#include "vcuda/core.h"
#include "vcuda/device.h"
#include "vcuda/driver.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Driver::launchKernel(
  CUfunction   f,
  unsigned int gridDimX,
  unsigned int gridDimY,
  unsigned int gridDimZ,
  unsigned int blockDimX,
  unsigned int blockDimY,
  unsigned int blockDimZ,
  unsigned int sharedMemBytes,
  CUstream     hstream,
  void **      kernelParams,
  void **      extra
)
{
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  // find and lock the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context_lock);

  // find and lock the selected stream
  const auto &[stream, stream_lock] = context->find_stream(hstream);
  if (!stream_lock)
    return CUDA_ERROR_INVALID_VALUE;

  // add a work to the work queue of the selected stream
  try {
    stream->add_work({ &Device::launchKernel
                     , f.argSize
                     , (const void**)kernelParams
                     , dim3(gridDimX, gridDimY, gridDimZ)
                     , dim3(blockDimX, blockDimY, blockDimZ)
                     , sharedMemBytes
                     , f.fn
                     , f.argc
                     });
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    return CUDA_ERROR_LAUNCH_FAILED;
  }

  // XXX: do not wait for device to complete.

  return CUDA_SUCCESS;

  (void)extra;
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
cuLaunchKernel(
  CUfunction   f,
  unsigned int gridDimX,
  unsigned int gridDimY,
  unsigned int gridDimZ,
  unsigned int blockDimX,
  unsigned int blockDimY,
  unsigned int blockDimZ,
  unsigned int sharedMemBytes,
  CUstream     hstream,
  void **      kernelParams,
  void **      extra
)
{
  return driver.launchKernel(f, gridDimX, gridDimY, gridDimZ, blockDimX,
                              blockDimY, blockDimZ, sharedMemBytes, hstream,
                              kernelParams, extra);
}
