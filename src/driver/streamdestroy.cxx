// SPDX-License-Identifier: MIT
#include "vcuda/core.h"
#include "vcuda/driver.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Driver::streamDestroy(CUstream hstream) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;
  if (0 == hstream) // XXX: Cannot destroy default stream
    return CUDA_ERROR_INVALID_VALUE;

  // record reference to the stream #hstream
  const auto &stream = find(streams, hstream);
  if (stream == streams.end())
    return CUDA_ERROR_INVALID_VALUE;

  CUresult res;

  // synchronize the stream
  // TODO: Should the stream be synchronized before being destroyed?
  if (CUDA_SUCCESS != (res = (*stream).synchronize()))
    return res;

  // erase the stream
  streams.erase(stream);

  return CUDA_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
cuStreamDestroy(CUstream hstream) {
  return driver.streamDestroy(hstream);
}
