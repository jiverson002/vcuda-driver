// SPDX-License-Identifier: MIT
#include <cstddef>
#include <iostream>
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
  if (!devices.size())
    return CUDA_ERROR_NOT_INITIALIZED;

  CUresult res;

  // Synchronize the device
  if (CUDA_SUCCESS != (res = deviceSynchronize()))
    return res;

  // record reference to the device #adev
  Device &dev = devices[adev];

  // record reference to the stream #0
  const auto &stream = find(streams, static_cast<std::size_t>(0));
  if (stream == streams.end())
    return CUDA_ERROR_INVALID_VALUE;

  // lock stream here
  std::lock_guard<std::mutex> lock((*stream).lock());

  // add the stream unit to the work queue of stream #0
  try {
    (*stream).add_work(Stream::unit( dev
                                   , &Device::memSet
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
    res = (*stream).get_work().res;
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
