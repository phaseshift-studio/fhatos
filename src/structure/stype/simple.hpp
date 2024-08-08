//  FhatOS: A Distributed Operating System
//  Copyright (c) 2024 PhaseShift Studio, LLC
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef fhatos_simple_hpp
#define fhatos_simple_hpp
#include <fhatos.hpp>
#include <structure/furi.hpp>
#include <structure/structure.hpp>

namespace fhatos {

  class Simple : public Structure {
  protected:
    explicit Simple(const Pattern &pattern) : Structure(pattern, SType::READ) {}

  public:
    List<Pair<fURI_p, Obj_p>> read(const fURI &furi, const ID &source) override {
      List<Pair<fURI_p, Obj_p>> results = List<Pair<fURI_p, Obj_p>>();
      for (const auto &sub: *this->subscriptions) {
        if (furi.matches(sub->pattern)) {
         // results.push_back(std::make_pair(share(fURI(furi)), Obj::to_uri(sub->pattern)));
        }
      }
      return results;
    }


    void write(const fURI &furi, const Obj_p &obj, const ID &source) override {}
  };
} // namespace fhatos

#endif
