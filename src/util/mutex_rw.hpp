#ifndef fhatos_mutex_rw_hpp
#define fhatos_mutex_rw_hpp

#include <fhatos.hpp>
////
#include <util/mutex.hpp>

namespace fhatos {
  template<typename SIZE_TYPE = uint8_t, uint16_t WAIT_TIME_MS = 500>
  class MutexRW {
  protected:
    Mutex _READER_LOCK;
    bool _WRITER_LOCK = false;
    SIZE_TYPE _READER_COUNT = 0;

  public:
    template<typename A>
    A write(const Supplier<A> &supplier) {
      Pair<RESPONSE_CODE, A *> result = std::make_pair<RESPONSE_CODE, A *>(MUTEX_LOCKOUT, nullptr);
      while (result.first == MUTEX_LOCKOUT) {
        result = _READER_LOCK.lockUnlock<Pair<RESPONSE_CODE, A *> >(
          [this,supplier]() {
            if (_WRITER_LOCK)
              return std::make_pair<RESPONSE_CODE, A *>(MUTEX_LOCKOUT, nullptr);
            else {
              A temp = supplier();
              return std::make_pair<RESPONSE_CODE, A *>(OK, &temp);
            }
          });
      }
      return *result.second;
    }

    template<typename A>
    A read(const Supplier<A> &supplier) {
      _READER_LOCK.lockUnlock<void *>([this]() {
        ++_READER_COUNT;
        _WRITER_LOCK = true;
        return nullptr;
      });
      A a = supplier();
      _READER_LOCK.lockUnlock<void *>([this]() {
        if (--_READER_COUNT == 0)
          _WRITER_LOCK = false;
        return nullptr;
      });
      return a;
    }
  };
} // namespace fhatos

#endif
