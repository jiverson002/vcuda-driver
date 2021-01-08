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
vcuda::driver::Driver::memSet(CUdeviceptr dptr, const int value, std::size_t num) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  // find and lock the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context_lock);

  // Synchronize the device (XXX: not sure if this should be here?)
  CUresult res = context->synchronize();
  if (CUDA_SUCCESS != res)
    return res;

  // find and lock the default stream
  const auto &[stream, stream_lock] = context->find_stream(default_stream);
  if (!stream_lock)
    return CUDA_ERROR_INVALID_VALUE;

  // add the stream unit to the work queue of stream #0
  try {
    stream->add_work(Stream::unit( &Device::memSet
                                 , std::vector<size_t>()
                                 , NULL
                                 , dptr
                                 , value
                                 , num
                                 ));
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    GOTO(ERROR);
  }

  // wait until the device has complete the work
  try {
    res = stream->get_work().res;
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
cuMemset(CUdeviceptr dptr, const int value, std::size_t num) {
  return driver.memSet(dptr, value, num);
}
