#ifndef fhatos_kernel__mutex_deque_hpp
#define fhatos_kernel__mutex_deque_hpp

#include <fhatos.hpp>
////
#include <kernel/process/util/mutex/mutex.hpp>

namespace fhatos::kernel {
template <typename T, typename SIZE_TYPE = uint8_t, uint16_t WAIT_TIME_MS = 250>
class MutexDeque {

protected:
  Deque<T> deque;
  Mutex mutex;

private:
  template <typename A = void *>
  A lockSwitch(const bool withMutex, Supplier<A> function) const {
    if (withMutex)
      return mutex.lockUnlock(function);
    else {
      A temp = function();
      return temp;
    }
  }

public:
  const Option<T> pop_front(const bool withMutex = true) {
    return lockSwitch<Option<T>>(withMutex, [this]() {
      if (deque.empty())
        return Option<T>();
      else {
        T t = deque.front();
        deque.pop_front();
        return Option<T>(t);
      }
    });
  }
  const Option<T> pop_back(const bool withMutex = true) {
    return lockSwitch<Option<T>>(withMutex, [this]() {
      if (deque.empty())
        return Option<T>();
      T t = deque.back();
      deque.pop_back();
      return Option<T>(t);
    });
  }
  bool push_front(const T &t, const bool withMutex = true) {
    return lockSwitch<bool>(withMutex, [this, t]() {
      deque.push_front(t);
      return true;
    });
  }
  bool push_back(const T &t, const bool withMutex = true) {
    return lockSwitch<bool>(withMutex, [this, t]() {
      deque.push_back(t);
      return true;
    });
  }
  SIZE_TYPE size(const bool withMutex = true) const {
    return lockSwitch<SIZE_TYPE>(withMutex, [this]() { return deque.size(); });
  }
  bool empty(const bool withMutex = true) const {
    return lockSwitch<bool>(withMutex, [this]() { return deque.empty(); });
  }
};
} // namespace fhatos::kernel

#endif