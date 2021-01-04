// SPDX-License-Identifier: MIT
#ifndef VCUDA_DRIVER_H
#define VCUDA_DRIVER_H 1
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <ostream>
#include <streambuf>
#include <vector>

#include <sys/types.h>

#include "vcuda/core.h"
#include "vcuda/device.h"
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
        Driver(std::ostream &log = std::cerr);
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
        CUresult streamSynchronize(CUstream);

      private:
        pid_t id;                     /*!< process id of the driver */
        int adev;                     /*!< active device identifier */
        std::vector<Device> devices;  /*!< list of devices */
        std::vector<Stream> streams;  /*!< list of streams */

        std::ostream &log;            /*!< ostream for logging */

        inline bool isDev(void)  { return id != getpid(); }
        inline bool isInit(void) { return devices.size(); }

        template <typename T> inline
        const char * argget(const char *buf, T& arg) {
          std::memcpy(&arg, buf, sizeof(arg));
          return buf + sizeof(arg);
        }
        template <typename T, typename... Args> inline
        const char * argget(const char *buf, T& arg, Args&... args) {
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
VCUDA_DRIVER_EXPORT CUresult cuMemcpyDtoH(void *hptr, const CUdeviceptr dptr, std::size_t num);
VCUDA_DRIVER_EXPORT CUresult cuMemcpyHtoD(CUdeviceptr dptr, const void *hptr, std::size_t num);

#ifdef __cplusplus
}
#endif

#endif // VCUDA_DRIVER_H
