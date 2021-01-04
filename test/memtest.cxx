// SPDX-License-Identifier: MIT
#include <gtest/gtest.h>

#include "vcuda.h"

#define ASSERT_CUDA_SUCCESS(f) ASSERT_EQ(CUDA_SUCCESS, f)

TEST(Memtest, MallocFree) {
  char *ptr = NULL;

  ASSERT_CUDA_SUCCESS(cuMemAlloc((void**)&ptr, 10));

  ASSERT_CUDA_SUCCESS(cuMemFree(ptr));
}

TEST(Memtest, Memset) {
  int n = 1023, v = rand();
  int *h, *d;

  ASSERT_TRUE(NULL != (h = new int[n]));

  ASSERT_CUDA_SUCCESS(cuMemAlloc((void**)&d, n * sizeof(*d)));

  ASSERT_CUDA_SUCCESS(cuMemset(d, v, n * sizeof(*d)));

  ASSERT_CUDA_SUCCESS(cuMemcpyDtoH(h, d, n * sizeof(*d)));

  // construct correct value
  memset(&v, v, sizeof(v));

  for (int i = 0; i < n; i++)
    ASSERT_EQ(h[i], v);

  delete [] h;

  ASSERT_CUDA_SUCCESS(cuMemFree(d));
}

TEST(Memtest, Memcpy) {
  int n = 1023;
  int *h1, *h2, *d;

  ASSERT_TRUE(NULL != (h1 = new int[n]));
  ASSERT_TRUE(NULL != (h2 = new int[n]));

  ASSERT_CUDA_SUCCESS(cuMemAlloc((void**)&d, n * sizeof(*d)));

  for (int i = 0; i < n; i++)
    h1[i] = rand();

  ASSERT_CUDA_SUCCESS(cuMemcpyHtoD(d, h1, n * sizeof(*h1)));

  ASSERT_CUDA_SUCCESS(cuMemcpyDtoH(h2, d, n * sizeof(*d)));

  for (int i = 0; i < n; i++)
    ASSERT_EQ(h1[i], h2[i]);

  delete [] h1;
  delete [] h2;

  ASSERT_CUDA_SUCCESS(cuMemFree(d));
}
