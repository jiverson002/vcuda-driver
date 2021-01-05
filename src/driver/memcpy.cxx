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
vcuda::driver::Driver::memCpyDtoH(void *hptr, CUdeviceptr dptr, std::size_t num) {
  if (!isInit())
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
                                   , &Device::memCpyDtoH
                                   , std::vector<size_t>()
                                   , NULL
                                   , dptr
                                   , num
                                   ));
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    GOTO(ERROR);
  }

  // copy data from shared buffer
  if (-1 == dev.read(hptr, num))
    GOTO(ERROR);

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
CUresult
vcuda::driver::Driver::memCpyHtoD(CUdeviceptr dptr, const void *hptr, std::size_t num) {
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
                                   , &Device::memCpyHtoD
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
  if (-1 == dev.write(hptr, num))
    GOTO(ERROR);

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
