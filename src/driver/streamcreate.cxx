// SPDX-License-Identifier: MIT
#include <exception>
#include <iostream>
#include <new>

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

  try {
    // create a new stream
    *phstream = streams.size(); // FIXME: need a better way to find next handle
    streams.emplace_back(*phstream, log);
  } catch (const std::bad_alloc &ba) {
    *log << ba.what() << std::endl;
    return CUDA_ERROR_OUT_OF_MEMORY;
  }

  try {
    // start the thread to manage the new stream
    streams.back().start();
  } catch (const std::exception &e) {
    *log << e.what() << std::endl;
    streams.pop_back();
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
