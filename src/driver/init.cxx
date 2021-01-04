// SPDX-License-Identifier: MIT
#include <exception>
#include <iostream>
#include <new>

#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/driver.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Driver::Driver(std::ostream *log) : adev(0), log(log) {
  id = getpid();

  /* XXX: Ensure that the device vector is not reallocated, thus copy
   * constructing new devices. */
  try {
    devices.reserve(VCUDA_NUM_DEVICE);
  } catch (const std::bad_alloc &ba) {
    std::cerr << ba.what() << std::endl;
    return;
  }

  /* XXX: Ensure that the stream vector is not reallocated, thus copy
   * constructing new devices. */
  try {
    streams.reserve(VCUDA_MAX_NUM_STREAM);
  } catch (const std::bad_alloc &ba) {
    std::cerr << ba.what() << std::endl;
    return;
  }

  // initialize the predefined number of devices
  for (int i = 0; i < VCUDA_NUM_DEVICE; i++) {
    try {
      devices.emplace_back(i, log, 0 == i ? '`' : '|');
    } catch (const char *e) {
      *log << e << std::endl;
    }

    if (isDev())
      devices.back().poweron();
  }

  try {
    // create the default stream
    streams.emplace_back(0, log);
  } catch (const char *e) {
    *log << e << std::endl;
    devices.clear();
    return;
  }

  try {
    // start the thread to manage the default stream
    streams.back().start();
  } catch (const std::exception &e) {
    *log << e.what() << std::endl;
    devices.clear();
    streams.clear();
    return;
  }
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Driver::~Driver(void) {
  *log << "driver: deconstructing..." << std::endl;
}
