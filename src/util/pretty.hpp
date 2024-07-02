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

#ifndef fhatos_pretty_hpp
#define fhatos_pretty_hpp

#include <fhatos.hpp>

namespace fhatos {
  class Pretty {
  public:
    static string prettyBytes(const int bytes, Ansi<StringPrinter> *ansi = nullptr) {
      string x;
      Ansi<StringPrinter>* temp;
      if (ansi)
        temp = ansi;
      else
        temp = new Ansi<StringPrinter>(new StringPrinter(&x));
      if (constexpr float tb = 1099511627776; bytes >= tb)
        temp->printf("%.2f tb", static_cast<float>(bytes) / tb);
      else if (constexpr float gb = 1073741824; bytes >= gb && bytes < tb)
        temp->printf("%.2f gb", static_cast<float>(bytes) / gb);
      else if (constexpr float mb = 1048576; bytes >= mb && bytes < gb)
        temp->printf("%.2f mb", static_cast<float>(bytes) / mb);
      else if (constexpr float kb = 1024; bytes >= kb && bytes < mb)
        temp->printf("%.2f kb", static_cast<float>(bytes) / kb);
      else
        temp->printf("%i bytes", bytes);
      return string(temp->stream()->get()->c_str());
    }
  };
}

#endif
