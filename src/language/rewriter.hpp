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
#ifndef fhatos_rewrite_hpp
#define fhatos_rewrite_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

namespace fhatos {
  using PriorPost = Pair<List<ID>, List<ID>>;
  using Rewrite = Trip<ID, Function<BCode_p, BCode_p>, PriorPost>;
  struct Rewriter {
    List<Rewrite> _rewrites;
    explicit Rewriter(const List<Rewrite> &rewrites) : _rewrites(rewrites) {}
    BCode_p apply(const BCode_p &bcode) {
      BCode_p running = bcode;
      for (const Rewrite &rw: this->_rewrites) {
        LOG(DEBUG, "Apply rewrite %s\n", std::get<0>(rw).toString().c_str());
        running = std::get<1>(rw)(running);
      }
      return running;
    }
  };
} // namespace fhatos
#endif
