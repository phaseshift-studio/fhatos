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

#ifndef fhatos_psram_allocator_hpp
#define fhatos_psram_allocator_hpp

#include "../../fhatos.hpp"

namespace fhatos {

  template<class T>
  struct PSRAMAllocator {
    typedef T value_type;

    PSRAMAllocator() = default;

    template<class U>
    constexpr PSRAMAllocator(const PSRAMAllocator<U> &) noexcept {}

    [[nodiscard]] T *allocate(const std::size_t n) {
      if (n > static_cast<std::size_t>(-1) / sizeof(T))
        throw std::bad_alloc();
      if (auto p = static_cast<T *>(ps_malloc(n * sizeof(T))))
        return p;
      throw std::bad_alloc();
    }
    void deallocate(T *p, std::size_t) noexcept { std::free(p); }
  };
} // namespace fhatos
#endif