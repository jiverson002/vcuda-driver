// SPDX-License-Identifier: MIT
#include <iostream>
#include <ostream>
#include <string>
#include <system_error>

#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::Stream(std::size_t streamnum, const Device &device,
                              std::ostream *log, const std::string &pfx)
  : thread(), device(device), id(streamnum), on(true), mtx(), copy_mtx(),
    work_mtx(), in_q_mtx(), in_q_filled(), in_q_flushed(), out_q_mtx(),
    out_q_filled(), pfx(pfx), log(log)
{
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::~Stream(void) {
  *log << pfx << "- deconstructing stream#" << id << "..." << std::endl;

  const char back = pfx.back();
  pfx.back() = back == '`' ? ' ' : '|';
  pfx += "  ";
  //std::string npfx = back == '`' ? "   " : "|  ";

  // TODO: Should the stream be synchronized before being destroyed?
  //synchronize();
  //*log << "|  |  |- stream synchronize...done" << std::endl;

  if (thread.joinable()) {
    *log << pfx << "`- joining with thread#" << thread.get_id() << "..."
         << std::endl;
    pfx += "   ";
    try {
      on = false;
      in_q_filled.notify_all();
      thread.join();
    } catch (const std::system_error& e) {
      *log << e.what() << std::endl;
    }
  }
}
