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
#ifndef string_helper_hpp
#define string_helper_hpp

#include <sstream>
#include <string>

namespace fhatos {
  class StringHelper final {
  public:
    StringHelper() = delete;
    static constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    static string hexStr(const unsigned char *data, const int len) {
      string s(len * 2, ' ');
      for (uint8_t i = 0; i < len; ++i) {
        s[2 * i] = hexmap[(data[i] & 0xF0) >> 4];
        s[2 * i + 1] = hexmap[data[i] & 0x0F];
      }
      return s;
    }

    static void trim(const std::string &s) {
      ltrim(const_cast<string &>(s));
      rtrim(const_cast<string &>(s));
    }

    static void ltrim(std::string &s) {
      s.erase(s.begin(),
              std::find_if(s.begin(), s.end(), [](const char c) { return !std::isspace(c) && c >= 0 && c < 128; }));
    }

    static void rtrim(std::string &s) {
      s.erase(
          std::find_if(s.rbegin(), s.rend(), [](const char c) { return !std::isspace(c) && c >= 0 && c < 128; }).base(),
          s.end());
    }

    static uint8_t countSubstring(const string &str, const string &sub) {
      if (sub.length() == 0)
        return 0;
      int count = 0;
      for (size_t offset = str.find(sub); offset != std::string::npos; offset = str.find(sub, offset + sub.length())) {
        ++count;
      }
      return count;
    }

    static bool lookAhead(const string &token, std::stringstream *ss, bool consume = true) {
      std::stringstream::pos_type start = ss->tellg();
      for (size_t i = 0; i < token.size(); i++) {
        if (token[i] != ss->peek()) {
          ss->seekg(start);
          return false;
        } else {
          ss->get();
        }
      }
      if (!consume)
        ss->seekg(start);
      return true;
    }
  };
} // namespace fhatos

#endif
