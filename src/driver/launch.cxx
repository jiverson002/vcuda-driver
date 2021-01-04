// SPDX-License-Identifier: MIT
#include <iostream>

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
  if (static_cast<decltype(streams.size())>(hstream) >= streams.size())
    return CUDA_ERROR_INVALID_VALUE;

  // record reference to the stream #hstream
  const auto &stream = find(streams, hstream);
  if (stream == streams.end())
    return CUDA_ERROR_INVALID_VALUE;

  // add a unit to the work queue of stream #hstream
  try {
    (*stream).add_work(Stream::unit( devices[adev]
                                   , &Device::launchKernel
                                   , f.argSize
                                   , (const void**)kernelParams
                                   , dim3(gridDimX, gridDimY, gridDimZ)
                                   , dim3(blockDimX, blockDimY, blockDimZ)
                                   , sharedMemBytes
                                   , f.fn
                                   , f.argc
                                   ));
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
