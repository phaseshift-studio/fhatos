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
#pragma once
#ifndef fhatos_ptr_helper
#define fhatos_ptr_helper

// #include <fhatos.hpp>

namespace fhatos {
  class Obj;
  class PtrHelper {
  private:
    template<typename T = Obj>
    struct NonDeleter {
      void operator()(const T *) { /*LOG(INFO, "Deleting...not!\n");*/
      }
    };

  public:
    PtrHelper() = delete;
    template<typename T = Obj>
    static NonDeleter<T> NON_DELETER_SINGLETON() {
      static auto *singleton = new NonDeleter<T>();
      return *singleton;
    }

    template<typename T = Obj>
    static ptr<T> no_delete(ptr<T> ptr_t) {
      ptr<T> temp = ptr<T>((T *) nullptr, NON_DELETER_SINGLETON<T>());
      temp.swap(ptr_t);
      return temp;
    }

    template<typename T = Obj>
    static ptr<T> no_delete(const T& t) {
      return share(t, NON_DELETER_SINGLETON<T>());
    }

    template<typename T = Obj>
    static ptr<T> no_delete(T *t) {
      return ptr<T>(t, NON_DELETER_SINGLETON<T>());
    }

    template<typename T = Obj>
    static ptr<T> clone(const T &t) {
      return share(T(t));
    }
    template<typename T = Obj>
    static ptr<T> clone(const ptr<T> &t) {
      return share(T(*t));
    }
    template<typename T = Obj>
    static List<ptr<T>> clone(const List<T> &list) {
      List<ptr<T>> newList = List<ptr<T>>();
      for (const auto &t: list) {
        newList.push_back(share(T(t)));
      }
      return newList;
    }
  };

} // namespace fhatos
#endif
