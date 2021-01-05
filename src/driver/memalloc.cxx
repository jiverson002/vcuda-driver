// SPDX-License-Identifier: MIT
#include <cstddef>
#include <iostream>
#include <mutex>
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

  CUresult res;

  // Synchronize the device
  if (CUDA_SUCCESS != (res = deviceSynchronize()))
    return res;

  // record reference to the stream #0
  const auto &stream = find(streams, static_cast<std::size_t>(0));
  if (stream == streams.end())
    return CUDA_ERROR_INVALID_VALUE;

  // lock stream here
  std::lock_guard<std::mutex> lock((*stream).lock());

  // add the stream unit to the work queue of stream #hstream
  try {
    (*stream).add_work(Stream::unit( devices[adev]
                                   , &Device::memAlloc
                                   , std::vector<size_t>()
                                   , NULL
                                   , bytesize
                                   ));
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    GOTO(ERROR);
  }

  // wait until the device has complete the work
  try {
    Stream::unit su((*stream).get_work());

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
