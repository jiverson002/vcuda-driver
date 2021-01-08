// SPDX-License-Identifier: MIT
#ifndef VCUDA_DRIVER_CONTEXT_H
#define VCUDA_DRIVER_CONTEXT_H 1
#include <algorithm>
#include <list>
#include <optional>
#include <ostream>
#include <shared_mutex>
#include <string>
#include <utility>

#include <sys/types.h>
#include <unistd.h>

#include "vcuda/core.h"
#include "vcuda/device.h"
#include "vcuda/driver/stream.h"

namespace vcuda {
  namespace driver {
    class Context {
      public:
        Context(int, std::ostream *, const std::string &);
        ~Context();

        CUresult synchronize(void);
        CUstream streamCreate(unsigned int);
        CUresult streamDestroy(CUstream);

        inline auto getDevnum(void) const { return devnum; }
        inline std::shared_mutex& getMutex(void) { return mtx; }
        inline const auto& get_device(void) const { return device; }

      private:
        pid_t id;   /*!< process id */
        int devnum; /*!< device number */

        std::shared_mutex mtx; /*!< context r/w mutex */

        Device device;             /*!< device corresponding to this context */
        std::list<Stream> streams; /*!< list of streams */

        std::string pfx;    /*!< */
        std::ostream *log;  /*!< ostream for logging */

        inline auto isDev(void) const { return id != getpid(); }

      public:
        inline auto find_stream(const decltype(streams.front().get_id()) &id) {
          const auto it = std::find_if(std::begin(streams), std::end(streams),
                            [&id] (const auto& e) { return e.get_id() == id; }
                          );
          return it != std::end(streams)
                  ? std::pair<const typename std::list<Stream>::iterator&,
                              std::optional<std::scoped_lock<std::mutex>>>
                    { it, (*it).getMutex() }
                  : std::pair<const typename std::list<Stream>::iterator&,
                              std::optional<std::scoped_lock<std::mutex>>>
                    { it, std::nullopt };
        }
    };
  }
}

#endif // VCUDA_DRIVER_CONTEXT_H
