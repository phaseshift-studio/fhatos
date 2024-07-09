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
#ifndef fhatos_exts_hpp
#define fhatos_exts_hpp


#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/furi.hpp>

namespace fhatos {
  class Exts {

  public:
    Exts() = delete;
    static List<Pair<ID, Type_p>> exts(const ID &extId) {
      static Map_p<ID, List<Pair<ID, Type_p>>> _exts = share(
          Map<ID, List<Pair<ID, Type_p>>>{{ID("/ext/process"),
                                           {{ID("/rec/thread"), TYPE_PARSER("[id=>as(/uri/),setup=>_,loop=>_]")},
                                            {ID("/rec/fiber"), TYPE_PARSER("[id=>as(/uri/),setup=>_,loop=>_]")},
                                            {ID("/rec/coroutine"), TYPE_PARSER("[id=>as(/uri/),setup=>_,loop=>_]")}}},
                                          {ID("/ext/collection"),
                                           {{ID("/lst/pair"), TYPE_PARSER("[_,_]")},
                                            {ID("/lst/trip"), TYPE_PARSER("[_,_,_]")},
                                            {ID("/lst/quad"), TYPE_PARSER("[_,_,_,_]")}}}});
      return _exts->at(extId);
    }
  };
} // namespace fhatos

#endif
