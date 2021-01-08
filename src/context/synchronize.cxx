// SPDX-License-Identifier: MIT
#include "vcuda/core.h"
#include "vcuda/driver/context.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Context::synchronize(void) {
  for (auto &stream : streams) {
    // FIXME: handle return value from stream synchronize
    stream.synchronize();
  }

  return CUDA_SUCCESS;
}
