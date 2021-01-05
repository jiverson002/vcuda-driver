// SPDX-License-Identifier: MIT
#include <iostream>
#include <mutex>

#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::driver::Stream::run(void) {
  while (on) {
    // acquire in_q lock -- for initial predicate check
    std::unique_lock<std::mutex> in_q_lock(in_q_mtx);

    // wait until there is some work to do
    in_q_filled.wait(in_q_lock, [&]() {
      return !on || !in_q.empty();
    });

    // check to see if the stream has been destroyed
    if (!on)
      break;

    // extract next unit of work
    unit su(in_q.front());
    in_q.pop();

    // notify someone if the in_q is flushed
    in_q_flushed.notify_one();

    // acquire work lock
    std::unique_lock<std::mutex> work_lock(work_mtx);

    // release in_q lock
    in_q_lock.unlock();

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work has been extracted from the queue. !! */
    /*------------------------------------------------------------------------*/

    // dispatch next unit of work
    launchKernel(su);

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work has been dispatched to the device. !! */
    /*------------------------------------------------------------------------*/

    // acquire out_q lock
    std::unique_lock<std::mutex> out_q_lock(out_q_mtx);

    // release work lock
    work_lock.unlock();

    // check to see if the stream has been destroyed
    if (!on)
      break;

    // push next unit of completed work
    out_q.push(su);

    // notify someone that the out_q has been filled
    out_q_filled.notify_one();

    // release out_q lock
    out_q_lock.unlock();

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work is available to be consumed by driver. !! */
    /*------------------------------------------------------------------------*/
  }

  *log << "|  |  `- thread cleanup...done" << std::endl;
}
