// SPDX-License-Identifier: MIT
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <ostream>
#include <system_error>

#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::Stream(int streamnum, std::ostream &log)
  : thread(), id(streamnum), on(true), log(log)
{
  // in_empty: create semaphore object name
  if (0 >= std::snprintf(in_empty_fname, sizeof(in_empty_fname),
                         "/cu%d-s%d-ie", getpid(), streamnum))
    GOTO(ERROR);

  // in_fill: create semaphore object name
  if (0 >= std::snprintf(in_fill_fname, sizeof(in_fill_fname),
                         "/cu%d-s%d-if", getpid(), streamnum))
    GOTO(ERROR);

  // out_empty: create semaphore object name
  if (0 >= std::snprintf(out_empty_fname, sizeof(out_empty_fname),
                         "/cu%d-s%d-oe", getpid(), streamnum))
    GOTO(ERROR);

  // out_fill: create semaphore object name
  if (0 >= std::snprintf(out_fill_fname, sizeof(out_fill_fname),
                         "/cu%d-s%d-of", getpid(), streamnum))
    GOTO(ERROR);

  // in_empty: open semaphore
  in_empty = sem_open(in_empty_fname, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR, 64);
  if (SEM_FAILED == in_empty)
    GOTO(ERROR);

  /*--------------------------------------------------------------------------*/
  /* !! IN_EMPTY_UNLINK: in_empty semaphore has been initialized -- any failure
   *    after this should unlink the semaphore. !! */
  /*--------------------------------------------------------------------------*/

  // in_fill: open semaphore
  in_fill = sem_open(in_fill_fname, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR, 0);
  if (SEM_FAILED == in_fill)
    GOTO(IN_EMPTY_UNLINK);

  /*--------------------------------------------------------------------------*/
  /* !! IN_FILL_UNLINK: in_empty semaphore has been initialized -- any failure
   *    after this should unlink the semaphore. !! */
  /*--------------------------------------------------------------------------*/

  // out_empty: open semaphore
  out_empty = sem_open(out_empty_fname, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR, 64);
  if (SEM_FAILED == out_empty)
    GOTO(IN_FILL_UNLINK);

  /*--------------------------------------------------------------------------*/
  /* !! OUT_EMPTY_UNLINK: in_empty semaphore has been initialized -- any failure
   *    after this should unlink the semaphore. !! */
  /*--------------------------------------------------------------------------*/

  // out_fill: open semaphore
  out_fill = sem_open(out_fill_fname, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR, 0);
  if (SEM_FAILED == out_fill)
    GOTO(OUT_EMPTY_UNLINK);

  /*--------------------------------------------------------------------------*/
  /* !! Stream initialization has completed successfully. !! */
  /*--------------------------------------------------------------------------*/
  return;

  OUT_EMPTY_UNLINK:
  (void)sem_unlink(out_empty_fname);

  IN_FILL_UNLINK:
  (void)sem_unlink(in_fill_fname);

  IN_EMPTY_UNLINK:
  (void)sem_unlink(in_empty_fname);

  ERROR:
  throw "stream initialization failed";
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::Stream(Stream &&other)
  : in_empty(other.in_empty), in_fill(other.in_fill),
    out_empty(other.out_empty), out_fill(other.out_fill),
    thread(std::move(other.thread)), id(other.id), on(other.on.load()),
    log(other.log)
{
  std::memcpy(in_empty_fname, other.in_empty_fname, sizeof(other.in_empty_fname));
  std::memcpy(in_fill_fname, other.in_fill_fname, sizeof(other.in_fill_fname));
  std::memcpy(out_empty_fname, other.out_empty_fname, sizeof(other.out_empty_fname));
  std::memcpy(out_fill_fname, other.out_fill_fname, sizeof(other.out_fill_fname));
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
vcuda::driver::Stream::~Stream(void) {
  log << "|- deconstructing stream#" << id << "..." << std::endl;

  // TODO: Should the stream be synchronized before being destroyed?
  //synchronize();
  //log << "|  |- stream synchronize...done" << std::endl;

  try {
    on = false;
    (void)sem_post(in_fill);
    (void)sem_post(out_empty);
    thread.join();
  } catch (const std::system_error& e) {
    CUSTREAMPANIC();
  }

  (void)sem_unlink(in_fill_fname);
  (void)sem_unlink(in_empty_fname);
  (void)sem_unlink(out_fill_fname);
  (void)sem_unlink(out_empty_fname);
  log << "|  `- semaphore cleanup...done" << std::endl;
}
