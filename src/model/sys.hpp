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
#ifndef fhatos_sys_hpp
#define fhatos_sys_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <structure/stype/computed.hpp>
#include <util/string_helper.hpp>

namespace fhatos {
  class Sys : public Computed {
    explicit Sys(const Pattern &pattern = "/sys/#") : Computed(pattern) {
#ifdef NATIVE
      this->read_functions_->insert(
        {furi_p(pattern.resolve("./+")), [this](const fURI_p &furi) {
          std::ifstream cpuInfo("/proc/cpuinfo");
          std::string line;
          IdObjPairs result;
          while (std::getline(cpuInfo, line)) {
            if (line.find("processor") != std::string::npos) {
              result.push_back({id_p(this->pattern_->resolve("./processor")), str(line)});
            } else if (line.find("cpu architecture") != std::string::npos) {
              result.push_back({id_p(this->pattern_->resolve("./cpu")), str(line)});
            } else if (line.find("physical id") != std::string::npos) {
              result.push_back({id_p(this->pattern_->resolve("./id")), str(line)});
            }
          }
          return make_shared<IdObjPairs>(result);
        }});
#endif
    }

  public:
    static ptr<Sys> singleton(const Pattern &pattern) {
      static auto sysp = ptr<Sys>(new Sys(pattern));
      return sysp;
    }
  };
}

#endif
