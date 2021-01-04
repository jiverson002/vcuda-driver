// SPDX-License-Identifier: MIT
#include <cerrno>
#include <iostream>
#include <mutex>

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

  std::unique_lock<std::mutex> work_lock;

  std::cerr << "driver: stream#" << id << ": flushing pending work queue"
            << std::endl;

  // wait while the stream is flushed by the stream thread.
  while (-1 != (ret = sem_trywait(in_fill))) {
    if (-1 == sem_post(in_fill))
      GOTO(ERROR);

    /* this is so that the driver thread does not repeatedly claim the in_fill
     * semaphore */
    if (-1 == usleep(10000) && EINTR != errno)
      GOTO(ERROR);
  }
  if (-1 == ret && EAGAIN != errno)
    GOTO(ERROR);

  /*--------------------------------------------------------------------------*/
  /* !! At this point, the stream has been flushed, but the stream thread may
   *    still be waiting for work on the device to complete. !! */
  /*--------------------------------------------------------------------------*/

  std::cerr << "driver: stream#" << id << ": waiting for outstanding work"
            << std::endl;

  // acquire work lock -- wait for any work on the device to complete
  work_lock = std::unique_lock<std::mutex>(work_mtx);

  // release work lock
  work_lock.unlock();

  std::cerr << "driver: stream#" << id << ": flushing completed work queue"
            << std::endl;

  // handle any completed work.
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
