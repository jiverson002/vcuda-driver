// SPDX-License-Identifier: MIT
#include <cmath>
#include <cstring>

#include <gtest/gtest.h>

#include "vcuda.h"

#define ASSERT_CUDA_SUCCESS(f) ASSERT_EQ(CUDA_SUCCESS, f)

__global__ (static void kernel)(int n, int *a, int *b, int *c) {
  __global_init__;

  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < n)
    c[idx] = a[idx] + b[idx];
}

TEST(Apptest, VecAdd) {
  int n = 1023;
  int *ha, *hb, *hc;
  int *da, *db, *dc;

  ha = new int[n];
  hb = new int[n];
  hc = new int[n];

  ASSERT_CUDA_SUCCESS(cuMemAlloc((void**)&da, n * sizeof(*da)));
  ASSERT_CUDA_SUCCESS(cuMemAlloc((void**)&db, n * sizeof(*db)));
  ASSERT_CUDA_SUCCESS(cuMemAlloc((void**)&dc, n * sizeof(*dc)));

  for (int i = 0; i < n; i++)
    ha[i] = rand(), hb[i] = rand();

  ASSERT_CUDA_SUCCESS(cuMemcpyHtoD(da, ha, n * sizeof(*ha)));
  ASSERT_CUDA_SUCCESS(cuMemcpyHtoD(db, hb, n * sizeof(*hb)));

  const unsigned int nthrd = 256;
  const unsigned int nblk  = ceil(n / static_cast<double>(nthrd));

  /*--------------------------------------------------------------------------*/
  /* !! The following uses the /explicit/ syntax for launching kernels. !! */
  /*--------------------------------------------------------------------------*/
  void *kernelParams[] = { (void*)&n, (void*)&da, (void*)&db, (void*)&dc };

  ASSERT_CUDA_SUCCESS(cuLaunchKernel(
    CUfunction(VCUDA_kernel_kernel,
               { sizeof(n), sizeof(da), sizeof(db), sizeof(dc) }),
    nblk, 1, 1, nthrd, 1, 1, 0, 0, kernelParams, NULL));

  ASSERT_CUDA_SUCCESS(cuMemcpyDtoH(hc, dc, n * sizeof(*dc)));

  for (int i = 0; i < n; i++)
    ASSERT_EQ(hc[i], ha[i] + hb[i]);

  ASSERT_CUDA_SUCCESS(cuMemFree(da));
  ASSERT_CUDA_SUCCESS(cuMemFree(db));
  ASSERT_CUDA_SUCCESS(cuMemFree(dc));

  delete [] ha;
  delete [] hb;
  delete [] hc;
}
