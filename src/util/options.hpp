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
#ifndef fhatos_options_hpp
#define fhatos_options_hpp
#include <memory>

using namespace std;

namespace fhatos {
  class Options final : public enable_shared_from_this<Options> {
  public:
    uint8_t LOGGING;
    void *ROUTING;
    Ansi<> *PRINTING;
    pair<string, string> URI_PATTERN;

    explicit Options() {}

    template<typename ROUTER>
    ROUTER *router() {
      if (nullptr == ROUTING)
        throw fError("No router secified in global options\n");
      return (ROUTER *) this->ROUTING;
    }

    template<typename LOGGER>
    LOGGER logger() {
      return (LOGGER) this->LOGGING;
    }

    template<typename PRINTER = Ansi<>>
    PRINTER *printer() {
      if (nullptr == PRINTING)
        throw fError("No printer secified in global options\n");
      return (PRINTER *) this->PRINTING;
    }
  };

  static std::shared_ptr<Options> GLOBAL_OPTIONS = std::shared_ptr<Options>(new Options());

} // namespace fhatos
#endif
