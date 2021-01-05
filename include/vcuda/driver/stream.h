// SPDX-License-Identifier: MIT
#ifndef VCUDA_DRIVER_STREAM_H
#define VCUDA_DRIVER_STREAM_H 1
#include <atomic>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ostream>
#include <mutex>
#include <condition_variable>
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

        Stream(std::size_t, std::ostream *);
        ~Stream(void);

        CUresult launchKernel(unit& su);
        CUresult synchronize(void);
        CUresult destroy(void);

        void add_work(const unit &su);
        unit get_work(void);

        inline void start(void) { thread = std::thread(&Stream::run, this); }
        inline std::size_t get_id(void) const { return id; }
        inline std::lock_guard<std::mutex> lock(void) {
          return std::lock_guard<std::mutex>(mtx);
        }

      private:
        std::thread thread;      /*!< TODO */

        std::queue<unit> in_q;   /*!< TODO */
        std::queue<unit> out_q;  /*!< TODO */

        std::size_t id;       /*!< the stream id */
        std::atomic<bool> on; /*!< indicator variable that device has been
                                   powered on (true) or off (false) */

        std::mutex mtx;
        std::mutex copy_mtx;
        std::mutex work_mtx;
        std::mutex in_q_mtx;
        std::condition_variable in_q_filled;
        std::condition_variable in_q_flushed;
        std::mutex out_q_mtx;
        std::condition_variable out_q_filled;

        std::ostream *log;

        void run(void);

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
