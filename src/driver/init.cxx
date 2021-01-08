// SPDX-License-Identifier: MIT
#include <iostream>

#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/driver.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Driver::Driver(std::ostream *log)
  : id(getpid()), active_context(0), default_stream(0), contexts(), log(log)
{
  // initialize the predefined number of contexts (one for each device)
  try {
    for (int i = 0; i < VCUDA_NUM_DEVICE; i++) {
      contexts.emplace_back(i, log, 0 != i ? "`" : "|");
    }
  } catch (const char *e) {
    *log << e << std::endl;
    GOTO(ERROR);
  }

  return;

  ERROR:
  throw "driver failed to initialize";
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Driver::~Driver(void) {
  *log << "driver: deconstructing..." << std::endl;
}
