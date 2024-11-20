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
#ifndef fhatos_rpi_pinout_hpp
#define fhatos_rpi_pinout_hpp

#include "fhatos.hpp"
#include "util/ansi.hpp"

namespace fhatos {
  class RpiPinout {
    void print() {
      const ptr<Ansi<>> ansi = Options::singleton()->printer<Ansi<>>();
      ansi->print("!S!y1 3 5 7 9!L!d12 4 6 8!!");
    }
  };
}

#endif
