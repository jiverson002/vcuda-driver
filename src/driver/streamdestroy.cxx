// SPDX-License-Identifier: MIT
#include <cassert>

#include "vcuda/core.h"
#include "vcuda/driver.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Driver::streamDestroy(CUstream hstream) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;
  if (default_stream == hstream) // XXX: Cannot destroy default stream
    return CUDA_ERROR_INVALID_VALUE;

  // find and lock the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context_lock);

  context->streamDestroy(hstream);

  return CUDA_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
cuStreamDestroy(CUstream hstream) {
  return driver.streamDestroy(hstream);
}
