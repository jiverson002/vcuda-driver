// SPDX-License-Identifier: MIT
#include <cassert>

#include "vcuda/core.h"
#include "vcuda/driver.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT CUresult
vcuda::driver::Driver::deviceSynchronize(void) {
  if (!isInit())
    return CUDA_ERROR_NOT_INITIALIZED;

  // record reference to the active context
  const auto &[context, context_lock] = find_context(active_context);
  assert(context != contexts.end());

  /* Synchronize the device, i.e., wait until device is ready, i.e., all
   * outstanding commands to be issued by the driver via any stream have
   * completed. */
  context->synchronize();

  return CUDA_SUCCESS;
}
