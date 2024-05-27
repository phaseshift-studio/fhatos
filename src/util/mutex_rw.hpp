/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_mutex_rw_hpp
#define fhatos_mutex_rw_hpp

#include <fhatos.hpp>
////
#include FOS_UTIL(mutex.hpp)
//using namespace std;

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
      Pair<RESPONSE_CODE, ptr<A> > result = std::make_pair<RESPONSE_CODE, ptr<A> >(MUTEX_LOCKOUT, nullptr);
      while (result.first == MUTEX_LOCKOUT) {
        result = _READER_LOCK.template lockUnlock<Pair<RESPONSE_CODE, ptr<A> > >(
          [this,supplier]() {
            if (_WRITER_LOCK)
              return std::make_pair<RESPONSE_CODE, ptr<A> >(MUTEX_LOCKOUT, nullptr);
            else {
              return std::make_pair<RESPONSE_CODE, ptr<A> >(OK, supplier());
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
