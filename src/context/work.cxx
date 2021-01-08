// SPDX-License-Identifier: MIT
#include <mutex>

#include "vcuda/core.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! Must be called in possession of mtx. */
/*----------------------------------------------------------------------------*/
void vcuda::driver::Context::add_work(CUstream hstream, const unit &su) {
  // find and lock the selected stream
  const auto &[stream, lock] = find_stream(hstream);
  if (stream == streams.end())
    return CUDA_ERROR_INVALID_VALUE;

  // add a unit to the work queue of stream #hstream
  stream->add_work(su);
}

/*----------------------------------------------------------------------------*/
/*! Must be called in possession of mtx. */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::unit vcuda::driver::Context::get_work(void) {
  stream->get_work()
}
