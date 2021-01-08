// SPDX-License-Identifier: MIT
#include "vcuda/core.h"
#include "vcuda/driver/context.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Context::streamDestroy(CUstream hstream) {
  // find and lock the requested stream
  const auto &[stream, stream_lock] = find_stream(hstream);
  if (!stream_lock)
    return CUDA_ERROR_INVALID_VALUE;

  // synchronize the stream
  // TODO: Should the stream be synchronized before being destroyed?
  CUresult res = stream->synchronize(stream_lock);
  if (CUDA_SUCCESS != res)
    return res;

  // erase the stream
  streams.erase(stream);

  return CUDA_SUCCESS;
}
