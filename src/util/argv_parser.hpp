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
#ifndef fhatos_argv_parser_hpp
#define fhatos_argv_parser_hpp

#include <fhatos.hpp>

namespace fhatos {
  class ArgvParser {
    Map<const string, string> map_ = Map<const string, string>();

  public:
    void init(const int &argc, char **argv) {
      for (int i = 1; i < argc; ++i) {
        const auto temp = string(argv[i]);
        const size_t j = temp.find_first_of('=');
        if (j != string::npos) {
          string key = temp.substr(0, j);
          string value = temp.substr(j + 1);
          this->map_.insert({key, value});
        } else {
          string key = temp;
          string value = "";
          this->map_.insert({key, value});
        }
      }
    }

    string option(const string &option, const char *or_else) const {
      return this->map_.count(option) ? this->map_.at(option) : or_else;
    }
  };
} // namespace fhatos
#endif