// SPDX-License-Identifier: MIT
#include <cstddef>
#include <iostream>
#include <shared_mutex>
#include <vector>

#include "vcuda/core.h"
#include "vcuda/device.h"
#include "vcuda/driver.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Driver::memAlloc(CUdeviceptr *dptr, std::size_t bytesize) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  // find and lock the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context_lock);

  // Synchronize the device
  CUresult res = context->synchronize();
  if (CUDA_SUCCESS != res)
    return res;

  // find and lock the default stream
  const auto &[stream, stream_lock] = context->find_stream(default_stream);
  if (!stream_lock)
    return CUDA_ERROR_INVALID_VALUE;

  // add the stream unit to the work queue of stream #hstream
  try {
    stream->add_work({ &Device::memAlloc
                     , std::vector<size_t>()
                     , NULL
                     , bytesize
                     });
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    GOTO(ERROR);
  }

  // wait until the device has completed the work
  try {
    Stream::unit su(stream->get_work());

    // read results
    argget(su.args, *dptr);

    res = su.res;
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    GOTO(ERROR);
  }

  return res;

  ERROR:
  return CUDA_ERROR;
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
cuMemAlloc(CUdeviceptr *dptr, std::size_t bytesize) {
  return driver.memAlloc(dptr, bytesize);
}
