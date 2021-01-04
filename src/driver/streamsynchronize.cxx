// SPDX-License-Identifier: MIT
#include "vcuda/core.h"
#include "vcuda/driver.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
vcuda::driver::Driver::streamSynchronize(CUstream hstream) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;
  if (static_cast<decltype(streams.size())>(hstream) >= streams.size())
    return CUDA_ERROR_INVALID_VALUE;

  // record reference to the stream #hstream
  Stream &stream = streams[hstream];

  /* Synchronize the stream, i.e., wait until stream is ready, i.e., all
   * outstanding commands to be issued by the driver via the stream have
   * completed. */
  return stream.synchronize();
}
