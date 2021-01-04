// SPDX-License-Identifier: MIT
#ifndef VCUDA_DRIVER_STREAM_H
#define VCUDA_DRIVER_STREAM_H 1
#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <thread>
#include <queue>

#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/device.h"

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
#ifndef GOTO
# define GOTO(lbl) do {\
  std::fprintf(stderr, "stream: failure in %s @ %s:%d\n", __FILE__, __func__,\
               __LINE__);\
  if (0 != errno)\
    std::fprintf(stderr, "  errno: %s\n", std::strerror(errno));\
  std::fflush(stderr);\
  goto lbl;\
} while (0)
#endif

namespace vcuda {
  namespace driver {
    /*------------------------------------------------------------------------*/
    /*! */
    /*------------------------------------------------------------------------*/
    class Stream {
      public:
        class unit {
          private:
            using device_fn = CUresult(Device::* volatile)(void);

          public:
            CUresult res;
            char args[4096];

            template <typename... Args>
            unit(const Device &dev, device_fn cmd,
                 const std::vector<std::size_t>& size,
                 const void **vargs,
                 const Args&... args)
              : dev(dev), cmd(cmd)
            {
              argsbytes = static_cast<decltype(argsbytes)>(
                argputv(argput(this->args, args...), size, vargs) - this->args
              );
            }

            template <typename T> inline
            char * argput(char *buf, const T& arg) {
              std::memcpy(buf, &arg, sizeof(arg));
              return buf + sizeof(arg);
            }
            template <typename T, typename... Args> inline
            char * argput(char *buf, const T& arg, const Args&... args) {
              std::memcpy(buf, &arg, sizeof(arg));
              return argput(buf + sizeof(arg), args...);
            }
            char * argputv(char *buf, const std::vector<std::size_t>& size,
                           const void **args)
            {
              for (decltype(size.size()) i = 0; i < size.size(); i++) {
                buf = argput(buf, size[i]);

                std::memcpy(buf, args[i], size[i]);
                buf += size[i];
              }
              return buf;
            }

          private:
            const Device &dev;
            device_fn cmd;
            size_t argsbytes;

            friend class Stream;
        };

        Stream(int, std::ostream &log);
        Stream(Stream &&other);

        ~Stream(void);

        void run(void);
        void start() { thread = std::thread(&Stream::run, this); }

        CUresult launchKernel(unit& su);

        CUresult synchronize(void);

        void add_work(const unit &su);
        unit get_work(void);

        sem_t *in_empty;    /*!< TODO */
        sem_t *in_fill;     /*!< TODO */
        sem_t *out_empty;   /*!< TODO */
        sem_t *out_fill;    /*!< TODO */
        std::thread thread; /*!< TODO */

        std::queue<unit> in_q;   /*!< TODO */
        std::queue<unit> out_q;  /*!< TODO */

        int id;               /*!< the stream id */
      private:
        std::atomic<bool> on; /*!< indicator variable that device has been powered
                                   on (true) or off (false) */
        char in_empty_fname[64];  /*!< file name of in_empty semaphore */
        char in_fill_fname[64];   /*!< file name of in_fill semaphore */
        char out_empty_fname[64]; /*!< file name of out_empty semaphore */
        char out_fill_fname[64];  /*!< file name of out_fill semaphore */

        std::ostream &log;

        void panic(const char * const filename, const char * const funcname,
                   const int line);
    };
  }
}

/*----------------------------------------------------------------------------*/
/*! */
/*----------------------------------------------------------------------------*/
#define CUSTREAMPANIC() panic(__FILE__, __func__, __LINE__)

#endif // VCUDA_DRIVER_STREAM_H
