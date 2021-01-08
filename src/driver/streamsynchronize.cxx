// SPDX-License-Identifier: MIT
#include <shared_mutex>

#include "vcuda/core.h"
#include "vcuda/driver.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
vcuda::driver::Driver::streamSynchronize(CUstream hstream) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  // find and lock the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context_lock);

  // record reference to the stream #hstream
  const auto &[stream, stream_lock] = context->find_stream(hstream);
  if (!stream_lock)
    return CUDA_ERROR_INVALID_VALUE;

  /* Synchronize the stream, i.e., wait until stream is ready, i.e., all
   * outstanding commands to be issued by the driver via the stream have
   * completed. */
  return stream->synchronize(stream_lock);
}
