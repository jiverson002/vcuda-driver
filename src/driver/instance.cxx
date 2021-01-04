// SPDX-License-Identifier: MIT
#include "vcuda/core/nullstream.h"
#include "vcuda/driver.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
#ifdef VCUDA_WITH_LOGGING
VCUDA_DRIVER_EXPORT vcuda::driver::Driver driver;
#else
static vcuda::core::NullStream ns;

VCUDA_DRIVER_EXPORT vcuda::driver::Driver driver(ns);
#endif
