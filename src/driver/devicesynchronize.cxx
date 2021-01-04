// SPDX-License-Identifier: MIT
#include "vcuda/core.h"
#include "vcuda/driver.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
vcuda::driver::Driver::deviceSynchronize(void) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  /* Synchronize the device, i.e., wait until device is ready, i.e., all
   * outstanding commands to be issued by the driver via any stream have
   * completed. */
  for (Stream &stream : streams) {
    // FIXME: handle return value from stream synchronize
    stream.synchronize();
  }

  return CUDA_SUCCESS;
}
