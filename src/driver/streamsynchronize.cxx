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

  // record reference to the stream #hstream
  const auto &stream = find(streams, hstream);
  if (stream == streams.end())
    return CUDA_ERROR_INVALID_VALUE;

  /* Synchronize the stream, i.e., wait until stream is ready, i.e., all
   * outstanding commands to be issued by the driver via the stream have
   * completed. */
  return (*stream).synchronize();
}
