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

#ifndef fhatos_allocator_hpp
#define fhatos_allocator_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

namespace fhatos {

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
/*
  template<class T>
  struct psram_allocator {
    using value_type = T;

    psram_allocator() noexcept = default;

    template<class U>
    psram_allocator(const psram_allocator<U> &) noexcept {}

    T *allocate(std::size_t n) {
      LOG(INFO, "!gAllocation!! occured for !y%i bytes!!\n", n);
      return (T *) ps_malloc(n);
    }

    void deallocate(T *p, std::size_t n) {
      LOG(INFO, "!rDeallocation!! occured for !y%i bytes!!\n");
      free(p);
    };
    // T *reallocate(void *p, size_t new_size) { return (T *) heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM); }
  };*/

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  /*template<class T, class U>
  constexpr bool operator==(const obj_allocator<T> &a, const obj_allocator<U> &b) noexcept;

  template<class T, class U>
  constexpr bool operator!=(const obj_allocator<T> &a, const obj_allocator<U> &b) noexcept;*/

 /* template<class T, class U>
  constexpr bool operator==(const psram_allocator<T> &a, const psram_allocator<U> &b) noexcept;

  template<class T, class U>
  constexpr bool operator!=(const psram_allocator<T> &a, const psram_allocator<U> &b) noexcept;*/
} // namespace fhatos

#endif
