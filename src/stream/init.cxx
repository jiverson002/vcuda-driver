// SPDX-License-Identifier: MIT
#include <iostream>
#include <ostream>
#include <system_error>

#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::Stream(std::size_t streamnum, std::ostream *log)
  : thread(), id(streamnum), on(true), log(log)
{
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::~Stream(void) {
  *log << "|- deconstructing stream#" << id << "..." << std::endl;

  // TODO: Should the stream be synchronized before being destroyed?
  //synchronize();
  //*log << "|  |- stream synchronize...done" << std::endl;

  if (thread.joinable()) {
    *log << "|  `- joining with thread#" << thread.get_id() << "..."
         << std::endl;
    try {
      on = false;
      in_q_filled.notify_all();
      thread.join();
    } catch (const std::system_error& e) {
      *log << e.what() << std::endl;
    }
  }
}
