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
vcuda::driver::Driver::memCpyDtoH(void *hptr, CUdeviceptr dptr, std::size_t num) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  // find and lock the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context_lock);

  // get the active device
  const auto &device = context->get_device();

  // Synchronize the device
  CUresult res = context->synchronize();
  if (CUDA_SUCCESS != res)
    return res;

  // find and lock the default stream
  const auto &[stream, stream_lock] = context->find_stream(default_stream);
  if (!stream_lock)
    return CUDA_ERROR_INVALID_VALUE;

  // add the stream unit to the work queue of stream #0
  try {
    stream->add_work({ &Device::memCpyDtoH
                     , std::vector<size_t>()
                     , NULL
                     , dptr
                     , num
                     });
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    GOTO(ERROR);
  }

  // copy data from shared buffer
  if (-1 == device.read(hptr, num))
    GOTO(ERROR);

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
CUresult
vcuda::driver::Driver::memCpyHtoD(CUdeviceptr dptr, const void *hptr, std::size_t num) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  // find and lock the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context_lock);

  // get the active device
  const auto &device = context->get_device();

  // Synchronize the device
  CUresult res = context->synchronize();
  if (CUDA_SUCCESS != res)
    return res;

  // find and lock the default stream
  const auto &[stream, stream_lock] = context->find_stream(default_stream);
  if (!stream_lock)
    return CUDA_ERROR_INVALID_VALUE;

  // add the stream unit to the work queue of stream #0
  try {
    stream->add_work(Stream::unit( &Device::memCpyHtoD
                                 , std::vector<size_t>()
                                 , NULL
                                 , dptr
                                 , num
                                 ));
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    GOTO(ERROR);
  }

  // copy data to shared buffer
  if (-1 == device.write(hptr, num))
    GOTO(ERROR);

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
cuMemcpyDtoH(void *hptr, CUdeviceptr dptr, std::size_t num) {
  return driver.memCpyDtoH(hptr, dptr, num);
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
cuMemcpyHtoD(CUdeviceptr dptr, const void *hptr, std::size_t num) {
  return driver.memCpyHtoD(dptr, hptr, num);
}
