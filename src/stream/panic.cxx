// SPDX-License-Identifier: MIT
#include <cstdio>
#include <cstdlib>

#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::driver::Stream::panic(const char * const filename,
                                  const char * const funcname,
                                  const int line)
{
  if (on) {
    std::fprintf(stderr, "PANIC: stream#%d: %s:%s@%d\n", id, filename, funcname,
                 line);
    std::fflush(stderr);
    std::abort();
  }
}
