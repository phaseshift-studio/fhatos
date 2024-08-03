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
#ifndef fhatos_message_hpp
#define fhatos_message_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>
#include <structure/furi.hpp>

#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

namespace fhatos {
  //////////////////////////////////////////////
  /////////////// MESSAGE STRUCT ///////////////
  //////////////////////////////////////////////
  struct Message {
  public:
    const ID source;
    const ID target;
    const Obj_p payload;
    const bool retain;

    template<OType otype>
    [[nodiscard]] bool is() const {
      return otype == this->payload->o_type();
    }

    bool isQuery(const char *query = "?") const {
      return !this->retain && payload->isNoObj() && target.query() == query && !target.equals(source);
    }

    [[nodiscard]] bool isReflexive() const { return target.query("").equals(source.query("")); }

    [[nodiscard]] string toString() const {
      char temp[100];
      sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(), payload->toString().c_str(),
              FOS_BOOL_STR(retain), target.toString().c_str());
      return {temp};
    }

    static fError UNKNOWN_PAYLOAD(const ID &id, const Obj_p &payload) {
      return fError("!m%s!! unable to process payload %s\n", id.toString().c_str(), payload->toString().c_str());
    }
  };
} // namespace fhatos

#endif