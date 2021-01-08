// SPDX-License-Identifier: MIT
#include <exception>
#include <iostream>
#include <new>
#include <string>

#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/driver/context.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Context::Context(int devnum, std::ostream *log,
                                const std::string &pfx)
  : id(getpid()), devnum(devnum), mtx(),
    device(devnum, log, pfx == "`" ? "    " : "|   "), streams(), pfx(pfx),
    log(log)
{
  if (isDev())
    device.poweron();

  /*--------------------------------------------------------------------------*/
  /* !! Only the driver process will make it to this point. !! */
  /*--------------------------------------------------------------------------*/

  try {
    // create the default stream
    streams.emplace_back(0, device, log, pfx == "`" ? "   |" : "|  |");
  } catch (const char *e) {
    *log << e << std::endl;
    GOTO(ERROR);
  }

  /*--------------------------------------------------------------------------*/
  /* !! STREAMS_CLEAR: streams have all been initialized -- any failure after
   *    this should clear the stream vector, thus destructing all streams. !! */
  /*--------------------------------------------------------------------------*/

  try {
    // start the thread to manage the default stream
    streams.back().start();
  } catch (const std::exception &e) {
    *log << e.what() << std::endl;
    GOTO(STREAMS_CLEAR);
  }

  return;

  STREAMS_CLEAR:
  streams.clear();

  ERROR:
  throw "context failed to initialize";
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Context::~Context(void) {
  *log << pfx << "- deconstructing context#" << devnum << "..." << std::endl;
}
