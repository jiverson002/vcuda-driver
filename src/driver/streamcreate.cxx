// SPDX-License-Identifier: MIT
#include <cassert>
#include <iostream>
#include <new>
#include <system_error>

#include "vcuda/core.h"
#include "vcuda/driver.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Driver::streamCreate(CUstream *phstream, unsigned int flags) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;
  if (0 != flags)
    return CUDA_ERROR_INVALID_VALUE;

  // find and lock the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context_lock);

  try {
    // create a new stream
    *phstream = context->streamCreate(flags);
  } catch (const std::bad_alloc &ba) {
    *log << ba.what() << std::endl;
    return CUDA_ERROR_OUT_OF_MEMORY;
  } catch (const std::system_error &se) {
    *log << se.what() << std::endl;
    return CUDA_ERROR;
  }

  return CUDA_SUCCESS;
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
cuStreamCreate(CUstream *phstream, unsigned int flags) {
  return driver.streamCreate(phstream, flags);
}
