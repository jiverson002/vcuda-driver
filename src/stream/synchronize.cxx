// SPDX-License-Identifier: MIT
#include <mutex>

#include "vcuda/core.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Stream::synchronize(void) {
  CUresult res = CUDA_SUCCESS;

  // acquire stream lock
  std::lock_guard<std::mutex> lock(mtx);

  /*--------------------------------------------------------------------------*/
  /* !! From this point forward, the size of in_q can only decrease, since no
   *    other thread will be able to add to it as long as this thread holds
   *    lock. !! */
  /*--------------------------------------------------------------------------*/

  // acquire in_q lock -- for initial predicate check
  std::unique_lock<std::mutex> in_q_lock(in_q_mtx);

  // wait until in_q has been flushed
  if (!in_q.empty()) {
    in_q_flushed.wait(in_q_lock, [&]() {
      return in_q.empty();
    });
  }

  /*--------------------------------------------------------------------------*/
  /* !! At this point, the pending work queue has been flushed, but the stream
   *    thread may still be waiting for work on the device to complete. At most
   *    there can be one outstanding job for the stream thread to wait on. !! */
  /*--------------------------------------------------------------------------*/

  // acquire work lock
  std::unique_lock<std::mutex> work_lock(work_mtx);

  // acquire out_q lock -- for initial predicate check
  std::unique_lock<std::mutex> out_q_lock(out_q_mtx);

  // handle any completed work.
  while (!out_q.empty()) {
    // extract next unit of completed work
    res = out_q.front().res;
    out_q.pop();

    if (CUDA_SUCCESS != res) {
      // FIXME: handle error code from kernel launch
    }
  }

  /*--------------------------------------------------------------------------*/
  /* !! There is no work in stream's work queue. !! */
  /*--------------------------------------------------------------------------*/

  return res;
}
