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
vcuda::driver::Driver::memFree(CUdeviceptr dptr) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  CUresult res;

  // Synchronize the device (XXX: not sure if this should be here?)
  if (CUDA_SUCCESS != (res = deviceSynchronize()))
    return res;

  // record reference to the stream #0
  Stream &stream = streams[0];

  std::cerr << "stream#0: adding work (memFree)" << std::endl;

  // add the stream unit to the work queue of stream #hstream
  try {
    stream.add_work(Stream::unit( devices[adev]
                                , &Device::memFree
                                , std::vector<size_t>()
                                , NULL
                                , dptr
                                ));
  } catch (const char *e) {
    *log << "driver: " << e << std::endl;
    GOTO(ERROR);
  }

  // wait until the device has complete the work
  try {
    res = stream.get_work().res;
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
cuMemFree(CUdeviceptr dptr) {
  return driver.memFree(dptr);
}
