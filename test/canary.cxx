// SPDX-License-Identifier: MIT
#include <gtest/gtest.h>

#include "vcuda.h"

#define ASSERT_CUDA_SUCCESS(f) ASSERT_EQ(CUDA_SUCCESS, f)

__global__ (static void kernel)(int *a) {
  __global_init__;

  *a = 1;
}

TEST(Canary, Launch) {
  int a, *ap;

  ASSERT_CUDA_SUCCESS(cuMemAlloc((void**)&ap, sizeof(*ap)));

  /*--------------------------------------------------------------------------*/
  /* !! The following is the more /explicit/ syntax for launching kernels. !! */
  /*--------------------------------------------------------------------------*/
  void *kernelParams[] = { (void*)&ap };

  ASSERT_CUDA_SUCCESS(cuLaunchKernel({ VCUDA_kernel_kernel, { sizeof(ap) } },
                                     2, 1, 1, 2, 2, 2, 0, 0,
                                     kernelParams, NULL));

  /*--------------------------------------------------------------------------*/
  /* !! !! */
  /*--------------------------------------------------------------------------*/

  ASSERT_CUDA_SUCCESS(cuMemcpyDtoH(&a, ap, sizeof(*ap)));

  ASSERT_CUDA_SUCCESS(cuMemFree(ap));

  ASSERT_EQ(a, 1);
}
