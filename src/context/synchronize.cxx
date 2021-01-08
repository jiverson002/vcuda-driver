// SPDX-License-Identifier: MIT
#include "vcuda/core.h"
#include "vcuda/driver/context.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Context::synchronize(void) {
  CUresult res = CUDA_SUCCESS;

  for (auto &stream : streams) {
    CUresult nres = stream.synchronize();
    if (CUDA_SUCCESS != nres && CUDA_SUCCESS == res)
      res = nres;
  }

  return res;
}
