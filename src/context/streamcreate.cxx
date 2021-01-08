// SPDX-License-Identifier: MIT
#include <exception>

#include "vcuda/core.h"
#include "vcuda/driver/context.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUstream
vcuda::driver::Context::streamCreate(unsigned int flags) {
  // FIXME: need a better way to find next handle
  CUstream stream = streams.size();

  streams.emplace_back(stream, device, log, pfx);

  try {
    // start the thread to manage the new stream
    streams.back().start();
  } catch (const std::exception &e) {
    /* remove stream from list of streams if it failed to start, then propagate
     * exception */
    streams.pop_back();
    throw e;
  }

  return stream;

  (void)flags;
}
