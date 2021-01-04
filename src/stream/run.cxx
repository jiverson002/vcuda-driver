// SPDX-License-Identifier: MIT
#include <iostream>
#include <mutex>

#include <semaphore.h>

#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::driver::Stream::run(void) {
  while (on) {
    // acquire queue lock -- for initial predicate check
    std::unique_lock<std::mutex> in_q_lock(in_q_mtx);

    // wait until there is some work to do
    in_q_cv.wait(in_q_lock, [&]() {
      return !in_q.empty();
    });

    // check to see if the stream has been destroyed
    if (!on)
      break;

    // extract next unit of work
    unit su(in_q.front());
    in_q.pop();

    // release in_q_lock
    in_q_lock.unlock();

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work has been extracted from the queue. !! */
    /*------------------------------------------------------------------------*/

    // dispatch next unit of work
    launchKernel(su);

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work has been dispatched to the device. !! */
    /*------------------------------------------------------------------------*/

    // acquire queue lock -- for initial predicate check
    std::unique_lock<std::mutex> out_q_lock(out_q_mtx);

    // wait until there is some work to do
    out_q_cv.wait(out_q_lock, [&]() {
      return VCUDA_MAX_NUM_WORK != out_q.size();
    });

    // check to see if the stream has been destroyed
    if (!on)
      break;

    // push next unit of completed work
    out_q.push(su);

    // release out_q_lock
    out_q_lock.unlock();

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work is available to be consumed by driver. !! */
    /*------------------------------------------------------------------------*/
  }

  *log << "|  |  `- thread cleanup...done" << std::endl;
}
