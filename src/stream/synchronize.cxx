// SPDX-License-Identifier: MIT
#include <cerrno>

#include <semaphore.h>
#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Stream::synchronize(void) {
  int ret;
  CUresult res = CUDA_SUCCESS;

  // wait while the stream is flushed by the stream thread.
  while (-1 != (ret = sem_trywait(in_fill))) {
    if (-1 == sem_post(in_fill))
      GOTO(ERROR);

    if (-1 == usleep(100000) && EINTR != errno)
      GOTO(ERROR);
  }
  if (-1 == ret && EAGAIN != errno)
    GOTO(ERROR);

  // handle any any completed work.
  while (-1 != sem_trywait(out_fill)) {
     // extract next unit of completed work
    res = out_q.front().res;
    out_q.pop();

    if (CUDA_SUCCESS != res) {
      // TODO: handle error code from kernel launch
    }

    if (-1 == sem_post(out_empty))
      GOTO(ERROR);
  }
  if (-1 == ret && EAGAIN != errno)
    GOTO(ERROR);

  /*--------------------------------------------------------------------------*/
  /* !! There is no work in stream's work queue. !! */
  /*--------------------------------------------------------------------------*/

  return res;

  ERROR:
  return CUDA_ERROR;
}
