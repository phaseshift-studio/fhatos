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
#ifndef fhatos_string_printer_hpp
#define fhatos_string_printer_hpp

#ifdef NATIVE
#define VIRTUAL
#define EXTENDS
#else
#define VIRTUAL virtual
#define EXTENDS : public Print
#endif

#include <string>

namespace fhatos {

  // TODO: Move to ansi
  class StringPrinter final EXTENDS {
  public:
    explicit StringPrinter(std::string *xstring) :
      xstring(xstring) {
    }

    string get() const { return *this->xstring; }

    VIRTUAL size_t print(const char *c_str) {
      const size_t length = strlen(c_str);
      for (size_t i = 0; i < length; i++) {
        this->write(c_str[i]);
      }
      return length;
    }

    VIRTUAL uint8_t print(char c) { return this->write(c); }

    VIRTUAL int read() const {
      return this->xstring->at(this->xstring->length() - 1);
    }

    VIRTUAL void flush() {
    };

    VIRTUAL size_t write(const uint8_t c) {
      *xstring += static_cast<char>(c);
      return 1;
    };

  private:
    std::string *xstring;
  };
} // namespace fhatos

#endif