#ifndef fhatos_mutex_rw_hpp
#define fhatos_mutex_rw_hpp

#include <fhatos.hpp>
////
#include FOS_UTIL(mutex.hpp)
using namespace std;

namespace fhatos {
  template<typename SIZE_TYPE = uint8_t, uint16_t WAIT_TIME_MS = 500>
  class MutexRW {
  protected:
    Mutex<WAIT_TIME_MS> _READER_LOCK;
    bool _WRITER_LOCK = false;
    SIZE_TYPE _READER_COUNT = 0;

  public:
    template<typename A>
    ptr<A> write(const Supplier<ptr<A> > &supplier) {
      Pair<RESPONSE_CODE, ptr<A> > result = make_pair<RESPONSE_CODE, ptr<A> >(MUTEX_LOCKOUT, nullptr);
      while (result.first == MUTEX_LOCKOUT) {
        result = _READER_LOCK.template lockUnlock<Pair<RESPONSE_CODE, ptr<A> > >(
          [this,supplier]() {
            if (_WRITER_LOCK)
              return make_pair<RESPONSE_CODE, ptr<A> >(MUTEX_LOCKOUT, nullptr);
            else {
              return make_pair<RESPONSE_CODE, ptr<A> >(OK, supplier());
            }
          });
      }
      return result.second;
    }

    template<typename A>
    A read(const Supplier<A> &supplier) {
      _READER_LOCK.template lockUnlock<void *>([this]() {
        ++_READER_COUNT;
        _WRITER_LOCK = true;
        return nullptr;
      });
      A a = supplier();
      _READER_LOCK.template lockUnlock<void *>([this]() {
        if (--_READER_COUNT == 0)
          _WRITER_LOCK = false;
        return nullptr;
      });
      return a;
    }
  };
} // namespace fhatos

#endif
