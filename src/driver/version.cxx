// SPDX-License-Identifier: MIT
#include "vcuda/core.h"
#include "vcuda/driver.h"
#include "vcuda/driver/config.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Driver::version(int *driverVersion) {
  if (NULL == driverVersion)
    return CUDA_ERROR_INVALID_VALUE;

  *driverVersion = 1000 * VCUDA_DRIVER_VERSION_MAJOR +\
                   10   * VCUDA_DRIVER_VERSION_MINOR;

  return CUDA_SUCCESS;
}
