// SPDX-License-Identifier: MIT
#include <mutex>

#include "vcuda/core.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! Must be called in possession of mtx. */
/*----------------------------------------------------------------------------*/
void vcuda::driver::Stream::add_work(const unit &su) {
  // acquire in_q lock
  std::lock_guard<std::mutex> in_q_lock(in_q_mtx);

  // enqueue next unit of work
  in_q.push(su);

  // notify someone that work is ready
  in_q_filled.notify_one();
}

/*----------------------------------------------------------------------------*/
/*! Must be called in possession of mtx. */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::unit vcuda::driver::Stream::get_work(void) {
  // acquire out_q lock -- for initial predicate check
  std::unique_lock<std::mutex> out_q_lock(out_q_mtx);

  // wait until there is some work to do
  if (out_q.empty()) {
    out_q_filled.wait(out_q_lock, [&]() {
      return !out_q.empty();
    });
  }

  // extract next unit of completed work
  unit su(out_q.front());
  out_q.pop();

  return su;
}
