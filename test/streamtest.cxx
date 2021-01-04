// SPDX-License-Identifier: MIT
#include <gtest/gtest.h>

#include "vcuda.h"

#define ASSERT_CUDA_SUCCESS(f) ASSERT_EQ(CUDA_SUCCESS, f)

__global__ (static void kernel)(int *a) {
  __global_init__;

  *a = 1;
}

TEST(Streamtest, CreateDestroy) {
  CUstream stream1, stream2, stream3;

  ASSERT_CUDA_SUCCESS(cuStreamCreate(&stream1, 0));
  ASSERT_CUDA_SUCCESS(cuStreamCreate(&stream2, 0));
  ASSERT_CUDA_SUCCESS(cuStreamCreate(&stream3, 0));

  ASSERT_CUDA_SUCCESS(cuStreamDestroy(stream2));
  ASSERT_CUDA_SUCCESS(cuStreamDestroy(stream1));
}

TEST(Streamtest, Launch) {
  int a;
  int *ap;
  CUstream stream1;
  (void)VCUDA_kernel_kernel;

  ASSERT_CUDA_SUCCESS(cuStreamCreate(&stream1, 0));

  ASSERT_CUDA_SUCCESS(cuMemAlloc((void**)&ap, sizeof(*ap)));

  /*--------------------------------------------------------------------------*/
  /* !! The following is the more /explicit/ syntax for launching kernels. !! */
  /*--------------------------------------------------------------------------*/
  void *kernelParams[] = { (void*)&ap };

  ASSERT_CUDA_SUCCESS(cuLaunchKernel({ VCUDA_kernel_kernel, { sizeof(ap) } },
                                     2, 1, 1, 2, 2, 2, 0, stream1,
                                     kernelParams, NULL));

  /*--------------------------------------------------------------------------*/
  /* !! !! */
  /*--------------------------------------------------------------------------*/

  ASSERT_CUDA_SUCCESS(cuMemcpyDtoH(&a, ap, sizeof(*ap)));

  ASSERT_CUDA_SUCCESS(cuMemFree(ap));

  ASSERT_CUDA_SUCCESS(cuStreamDestroy(stream1));

  ASSERT_EQ(a, 1);
}
