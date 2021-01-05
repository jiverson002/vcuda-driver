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
    *log << "|  `- joining with thread#" << tid << "..." << std::endl;
    try {
      on = false;
      in_q_filled.notify_all();
      thread.join();
    } catch (const std::system_error& e) {
      return CUDA_ERROR;
    }
  }

  return CUDA_SUCCESS;
}
