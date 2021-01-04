// SPDX-License-Identifier: MIT
#include <iostream>

#include <semaphore.h>

#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Stream::destroy(void) {
  if (thread.joinable()) {
    auto tid = thread.get_id();
    *log << "|  |- joining with thread#" << tid << "..." << std::endl;
    try {
      on = false;
      (void)sem_post(in_fill);
      (void)sem_post(out_empty);
      thread.join();
    } catch (const std::system_error& e) {
      return CUDA_ERROR;
    }
    *log << "|  `- unlinking semaphores..." << std::endl;

    if (0 != sem_unlink(in_fill_fname))
      return CUDA_ERROR;
    *log << "|     |- " << in_fill_fname << "...done" << std::endl;
    if (0 != sem_unlink(in_empty_fname))
      return CUDA_ERROR;
    *log << "|     |- " << in_empty_fname << "...done" << std::endl;
    if (0 != sem_unlink(out_fill_fname))
      return CUDA_ERROR;
    *log << "|     |- " << out_fill_fname << "...done" << std::endl;
    if (0 != sem_unlink(out_empty_fname))
      return CUDA_ERROR;
    *log << "|     `- " << out_empty_fname << "...done" << std::endl;
  }

  return CUDA_SUCCESS;
}
