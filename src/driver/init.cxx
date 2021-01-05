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
   *      constructing new objects and destructing old ones. */
  try {
    devices.reserve(VCUDA_NUM_DEVICE);
  } catch (const std::bad_alloc &ba) {
    *log << ba.what() << std::endl;
    GOTO(ERROR);
  }

  // initialize the predefined number of devices
  for (int i = 0; i < VCUDA_NUM_DEVICE; i++) {
    try {
      devices.emplace_back(i, log, 0 == i ? '`' : '|');
    } catch (const char *e) {
      *log << e << std::endl;
      GOTO(ERROR);
    }

    if (isDev())
      devices.back().poweron();
  }

  /*--------------------------------------------------------------------------*/
  /* !! DEVICES_CLEAR: devices have all been initialized -- any failure after
   *    this should clear the device vector, thus destructing all devices. !! */
  /*--------------------------------------------------------------------------*/

  try {
    // create the default stream
    streams.emplace_back(0, log);
  } catch (const char *e) {
    *log << e << std::endl;
    GOTO(DEVICES_CLEAR);
  }

  /*--------------------------------------------------------------------------*/
  /* !! STREAMS_CLEAR: streams have all been initialized -- any failure after
   *    this should clear the stream vector, thus destructing all streams. !! */
  /*--------------------------------------------------------------------------*/

  try {
    // start the thread to manage the default stream
    streams.back().start();
  } catch (const std::exception &e) {
    *log << e.what() << std::endl;
    GOTO(STREAMS_CLEAR);
  }

  return;

  STREAMS_CLEAR:
  streams.clear();

  DEVICES_CLEAR:
  devices.clear();

  ERROR:
  throw "driver failed to initialize";
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Driver::~Driver(void) {
  *log << "driver: deconstructing..." << std::endl;
}
