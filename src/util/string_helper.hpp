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

#ifndef string_helper_hpp
#define string_helper_hpp

#include <string>

namespace fhatos {
  class StringHelper {
  public:
    static inline void trim(const std::string &s) {
      ltrim(const_cast<string &>(s));
      rtrim(const_cast<string &>(s));
    }

    static inline void ltrim(std::string &s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
      }));
    }

    static inline void rtrim(std::string &s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
      }).base(), s.end());
    }

    static bool lookAhead(const string token, std::stringstream *ss) {
      for (int i = 0; i < token.size(); i++) {
        if(token[i] != ss->peek()) {
          for (int j = 0; j <= i; j++) {
            ss += token[j];
          }
          return false;
        } else {
          ss->get();
        }
      }
      return true;
    }

  private:
    StringHelper() = delete;
  };
}

#endif
