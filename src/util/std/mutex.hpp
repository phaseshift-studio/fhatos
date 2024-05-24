#ifndef fhatos_std_mutex_hpp
#define fhatos_std_mutex_hpp

#include <fhatos.hpp>
#include <mutex>
#include <sys/time.h>
#include <chrono>
#include <ctime>

namespace fhatos {
  class Mutex {
  private:
    std::mutex *xmutex = new std::mutex();

  public:
    ~Mutex() { delete this->xmutex; }

    template<typename T = void *>
    T lockUnlock(const Supplier<T> criticalFunction,
                 const uint16_t millisecondsWait = 250) {
      if (this->lock(millisecondsWait)) {
        T t = criticalFunction();
        this->unlock();
        return t;
      } else {
        fError error("Unable to lock mutex: %i", __LINE__);
        LOG_EXCEPTION(error);
        throw error;
      }
    }

    const bool lock(const uint16_t millisecondsWait = 250) {
      const long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
      while (true) {
        if (this->xmutex->try_lock()) {
          return true;
        } else if ((std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch()).count() - timestamp) > millisecondsWait) {
          return false;
        }
      }
    }

    const bool unlock(const uint16_t millisecondsWait = 250) {
      this->xmutex->unlock();
      return true;
    }
  };
} // namespace fhatos

#endif
