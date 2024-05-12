#ifndef fhatos_mutex_deque_hpp
#define fhatos_mutex_deque_hpp

#include <fhatos.hpp>
////
#include <util/mutex.hpp>

namespace fhatos {

template <typename T, typename SIZE_TYPE = uint8_t, uint16_t WAIT_TIME_MS = 250>
class MutexDeque {

protected:
  Deque<T> _deque;
  Mutex _mutex;

private:
  template <typename A = void *>
  A lockUnlock(const bool withMutex, Supplier<A> function) const {
    if (withMutex)
      return _mutex.lockUnlock(function);
    else {
      A temp = function();
      return temp;
    }
  }

public:
  // MutexDeque(Mutex *mutex = new Mutex()) : _mutex(mutex) {}

  const Option<T> find(Predicate<T> predicate,
                       const bool withMutex = true) const {
    return lockUnlock<Option<T>>(withMutex, [this, predicate]() {
      T *temp = nullptr;
      for (T t : _deque) {
        if (predicate(t)) {
          temp = &t;
          break;
        }
      }
      return temp ? Option<T>(*temp) : Option<T>();
    });
  }

  const Option<T> pop_front(const bool withMutex = true) {
    return lockUnlock<Option<T>>(withMutex, [this]() {
      if (_deque.empty()) {
        return Option<T>();
      } else {
        T t = _deque.front();
        _deque.pop_front();
        return Option<T>(t);
      }
    });
  }

  void forEach(Consumer<T> consumer, const bool withMutex = true) const {
    lockUnlock<void *>(withMutex, [this, consumer]() {
      for (T t : _deque) {
        consumer(t);
      }
      return nullptr;
    });
  }

  void remove_if(Predicate<T> predicate, const bool withMutex = true) {
    lockUnlock<void *>(withMutex, [this, predicate]() {
      _deque.erase(std::remove_if(_deque.begin(), _deque.end(),
                                  [predicate](T t) { return predicate(t); }),
                   _deque.end());
      return nullptr;
    });
  }

  void remove(const T &toRemove, const bool withMutex = true) {
    this->remove_if([this, toRemove](T t) { return t == toRemove; }, withMutex);
  }

  const Option<T> pop_back(const bool withMutex = true) {
    return lockUnlock<Option<T>>(withMutex, [this]() {
      if (_deque.empty())
        return Option<T>();
      T t = _deque.back();
      _deque.pop_back();
      return Option<T>(t);
    });
  }
  const bool push_front(const T t, const bool withMutex = true) {
    return lockUnlock<bool>(withMutex, [this, t]() {
      _deque.push_front(t);
      return true;
    });
  }
  const bool push_back(const T t, const bool withMutex = true) {
    return lockUnlock<bool>(withMutex, [this, t]() {
      _deque.push_back(t);
      return true;
    });
  }
  const SIZE_TYPE size(const bool withMutex = true) const {
    return lockUnlock<SIZE_TYPE>(withMutex, [this]() { return _deque.size(); });
  }
  const bool empty(const bool withMutex = true) const {
    return lockUnlock<bool>(withMutex, [this]() { return _deque.empty(); });
  }
  const String toString(const bool withMutex = true) const {
    return lockUnlock<String>(withMutex, [this]() {
      String temp = "[";
      for (const auto &t : _deque) {
        if (t)
          temp = temp + t->toString() + ", ";
      }
      temp = temp.substring(0, temp.length() - 3) + "]";
      return temp;
    });
  }

  const void clear(const bool withMutex = true) {
    lockUnlock<void *>(withMutex, [this]() {
      _deque.clear();
      return nullptr;
    });
  }
};
} // namespace fhatos

#endif