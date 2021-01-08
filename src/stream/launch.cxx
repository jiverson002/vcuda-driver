// SPDX-License-Identifier: MIT
#include <cstring>

#include "vcuda/core.h"
#include "vcuda/device.h"
#include "vcuda/driver.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
CUresult
vcuda::driver::Stream::launchKernel(unit& su) {
  // copy regs from queue to device
  device.regs->cmd = su.cmd;
  std::memcpy(device.regs->args, su.args, su.argsbytes);

  // signal that work is ready for the device.
  if (-1 == sem_post(device.regs->work))
    GOTO(LAUNCH_FAILED);

  // for for device to finish computation.
  if (-1 == sem_wait(device.regs->done))
    GOTO(LAUNCH_FAILED);

  // copy result
  std::memcpy(su.args, device.regs->args, device.regs->argsbytes);

  // res
  return su.res = device.regs->res;

  LAUNCH_FAILED:
  return CUDA_ERROR_LAUNCH_FAILED;
}
