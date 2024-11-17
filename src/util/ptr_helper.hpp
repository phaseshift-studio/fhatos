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

#include <memory>

using namespace std;
namespace fhatos {

  class Obj;

  template<typename T = Obj>
  using ptr = shared_ptr<T>;

  class PtrHelper final {
  private:
    template<typename T = Obj>
    struct NonDeleter {
      void operator()(const T *) { /*LOG(INFO, "Deleting...not!\n");*/ }
    };

  public:
    PtrHelper() = delete;

    template<typename T = Obj>
    static NonDeleter<T> NON_DELETER_SINGLETON() {
      static auto *singleton = new NonDeleter<T>();
      return *singleton;
    }

    template<typename T = Obj>
    static shared_ptr<T> no_delete(shared_ptr<T> &ptr_t) {
      auto temp = shared_ptr<T>(static_cast<T *>(nullptr), NON_DELETER_SINGLETON<T>());
      temp.swap(ptr_t);
      return temp;
    }

    template<typename T = Obj>
    static std::shared_ptr<T> no_delete(const T &t) {
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
      List<ptr<T>> new_list = List<ptr<T>>();
      for (const auto &t: list) {
        new_list.push_back(make_shared<T>(t));
      }
      return new_list;
    }
  };

} // namespace fhatos
#endif
