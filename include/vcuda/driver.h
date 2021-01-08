// SPDX-License-Identifier: MIT
#ifndef VCUDA_DRIVER_H
#define VCUDA_DRIVER_H 1
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <list>
#include <optional>
#include <ostream>
#include <shared_mutex>
#include <streambuf>
#include <utility>
#include <vector>

#include <sys/types.h>

#include "vcuda/core.h"
#include "vcuda/device.h"
#include "vcuda/driver/context.h"
#include "vcuda/driver/export.h"
#include "vcuda/driver/stream.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
#define VCUDA_NUM_DEVICE 2

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
#ifndef GOTO
# define GOTO(lbl) do {\
  std::fprintf(stderr, "driver: failure in %s @ %s:%d\n", __FILE__, __func__,\
               __LINE__);\
  if (0 != errno)\
    std::fprintf(stderr, "  errno: %s\n", std::strerror(errno));\
  std::fflush(stderr);\
  goto lbl;\
} while (0)
#endif

namespace vcuda {
  namespace driver {
    class Driver {
      public:
        Driver(std::ostream *log = &std::cerr);
        ~Driver();

        CUresult deviceSynchronize(void);
        CUresult launchKernel(CUfunction   f,
                              unsigned int gridDimX,
                              unsigned int gridDimY,
                              unsigned int gridDimZ,
                              unsigned int blockDimX,
                              unsigned int blockDimY,
                              unsigned int blockDimZ,
                              unsigned int sharedMemBytes,
                              CUstream     hstream,
                              void **      kernelParams,
                              void **      extra);
        CUresult memAlloc(CUdeviceptr *, std::size_t);
        CUresult memCpyDtoH(void *, const CUdeviceptr, std::size_t);
        CUresult memCpyHtoD(CUdeviceptr, const void *, std::size_t);
        CUresult memFree(CUdeviceptr);
        CUresult memSet(CUdeviceptr, const int, std::size_t);
        CUresult streamCreate(CUstream *, unsigned int);
        CUresult streamDestroy(CUstream);
        CUresult streamSynchronize(CUstream);
        CUresult version(int *);

      private:
        pid_t id; /*!< process id of the driver */

        int active_context;       /*!< active context identifier */
        CUstream default_stream;  /*!< default stream identifier */

        std::list<Context> contexts; /*!< list of contexts */

        std::ostream *log;           /*!< ostream for logging */

        inline bool isDev(void)  { return id != getpid(); }
        inline bool isInit(void) { return contexts.size(); }

        inline auto
        find_context(const decltype(contexts.front().getDevnum()) &devnum) {
          const auto it = std::find_if(std::begin(contexts), std::end(contexts),
                            [&devnum] (const auto& e) { return e.getDevnum() == devnum; }
                          );
          return it != std::end(contexts)
                  ? std::pair<const typename std::list<Context>::iterator,
                              std::optional<std::scoped_lock<std::shared_mutex>>>
                    { it, (*it).getMutex() }
                  : std::pair<const typename std::list<Context>::iterator,
                              std::optional<std::scoped_lock<std::shared_mutex>>>
                    { it, std::nullopt };
        }

        template <typename T> inline const char *
        argget(const char *buf, T& arg) {
          std::memcpy(&arg, buf, sizeof(arg));
          return buf + sizeof(arg);
        }
        template <typename T, typename... Args> inline const char *
        argget(const char *buf, T& arg, Args&... args) {
          std::memcpy(&arg, buf, sizeof(arg));
          return argput(buf + sizeof(arg), args...);
        }
    };
  }
}

/*----------------------------------------------------------------------------*/
/*! One instance of driver per program invocation. */
/*----------------------------------------------------------------------------*/
VCUDA_DRIVER_EXPORT extern vcuda::driver::Driver driver;

/*----------------------------------------------------------------------------*/
/*! Driver API. */
/*----------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

//VCUDA_DRIVER_EXPORT CUresult cuInit(unsigned int flags);
VCUDA_DRIVER_EXPORT CUresult cuLaunchKernel(CUfunction f,
                                     unsigned int gridDimX,
                                     unsigned int gridDimY,
                                     unsigned int gridDimZ,
                                     unsigned int blockDimX,
                                     unsigned int blockDimY,
                                     unsigned int blockDimZ,
                                     unsigned int sharedMemBytes,
                                     CUstream     hstream,
                                     void **      kernelParams,
                                     void **      extra);
VCUDA_DRIVER_EXPORT CUresult cuMemAlloc(CUdeviceptr *dptr, std::size_t bytesize);
VCUDA_DRIVER_EXPORT CUresult cuMemFree(CUdeviceptr dptr);
VCUDA_DRIVER_EXPORT CUresult cuMemset(CUdeviceptr dptr, const int value, std::size_t num);
VCUDA_DRIVER_EXPORT CUresult cuMemcpyHtoD(CUdeviceptr dptr, const void *hptr, std::size_t num);
VCUDA_DRIVER_EXPORT CUresult cuMemcpyDtoH(void *hptr, const CUdeviceptr dptr, std::size_t num);
VCUDA_DRIVER_EXPORT CUresult cuStreamCreate(CUstream *phstream, unsigned int flags);
VCUDA_DRIVER_EXPORT CUresult cuStreamDestroy(CUstream hstream);

#ifdef __cplusplus
}
#endif

#endif // VCUDA_DRIVER_H
