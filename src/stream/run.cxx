// SPDX-License-Identifier: MIT
#include <iostream>

#include <semaphore.h>

#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::driver::Stream::run(void) {
  while (on) {
    if (-1 == sem_wait(in_fill))
      CUSTREAMPANIC();

    if (!on)
      break;

    // extract next unit of work
    unit su(in_q.front());
    in_q.pop();

    if (-1 == sem_post(in_empty))
      CUSTREAMPANIC();

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work has been extracted from the queue. !! */
    /*------------------------------------------------------------------------*/

    // dispatch next unit of work
    launchKernel(su);

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work has been dispatched to the device. !! */
    /*------------------------------------------------------------------------*/

    if (-1 == sem_wait(out_empty))
      CUSTREAMPANIC();

    if (!on)
      break;

    // enqueue next unit of completed work
    out_q.push(su);

    if (-1 == sem_post(out_fill))
      CUSTREAMPANIC();

    /*------------------------------------------------------------------------*/
    /* !! Next unit of work has completed. !! */
    /*------------------------------------------------------------------------*/
  }

  log << "|  |- thread cleanup...done" << std::endl;
}
