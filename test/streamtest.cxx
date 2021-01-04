// SPDX-License-Identifier: MIT
#include <gtest/gtest.h>

#include "vcuda.h"

#define ASSERT_CUDA_SUCCESS(f) ASSERT_EQ(CUDA_SUCCESS, f)

TEST(Streamtest, CreateDestroy) {
  CUstream stream1, stream2, stream3;

  ASSERT_CUDA_SUCCESS(cuStreamCreate(&stream1, 0));
  ASSERT_CUDA_SUCCESS(cuStreamCreate(&stream2, 0));
  ASSERT_CUDA_SUCCESS(cuStreamCreate(&stream3, 0));

  ASSERT_CUDA_SUCCESS(cuStreamDestroy(stream2));
  ASSERT_CUDA_SUCCESS(cuStreamDestroy(stream1));
}
