// SPDX-License-Identifier: MIT
#include <semaphore.h>

#include "vcuda/core.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
void vcuda::driver::Stream::add_work(const unit &su) {
  if (-1 == sem_wait(in_empty))
    throw "could not wait for stream.in_empty";

  // enqueue next unit of work
  in_q.push(su);

  if (-1 == sem_post(in_fill))
    throw "could not post to stream.in_fill";
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::unit vcuda::driver::Stream::get_work(void) {
  if (-1 == sem_wait(out_fill))
    throw "could not wait for stream.out_fill";

  // extract next unit of completed work
  unit su(out_q.front());
  out_q.pop();

  if (-1 == sem_post(out_empty))
    throw "could not post to stream.out_empty";

  return su;
}
