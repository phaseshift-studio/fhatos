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
  enum class WILDCARD { NO = 0, PLUS = 1, HASH = 2 };

  class StringHelper final {
  public:
    StringHelper() = delete;

    static constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    static string hex_string(const unsigned char *data, const int len) {
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

    static string repeat(const uint8_t amount, const string &repeater = " ") {
      string temp;
      for (int i = 0; i < amount; i++) {
        temp += repeater;
      }
      return temp;
    }

    static string format(const char *format, ...) {
      char message[FOS_DEFAULT_BUFFER_SIZE];
      va_list arg;
      va_start(arg, format);
      vsnprintf(message, FOS_DEFAULT_BUFFER_SIZE, format, arg);
      va_end(arg);
      return string(message);
    }

    static void ltrim(std::string &s) {
      s.erase(s.begin(),
              std::find_if(s.begin(), s.end(), [](const char c) { return !std::isspace(c) && c < 127; }));
    }

    static void rtrim(std::string &s) {
      s.erase(
        std::find_if(s.rbegin(), s.rend(),
                     [](const char c) { return !std::isspace(c) && c < 127; }).base(),
        s.end());
    }

    static bool has_wildcards(const std::string &s) {
      return s.find('+') != string::npos || s.find('#') != string::npos;
    }


    static WILDCARD has_wildcard(const char *s) {
      for (size_t i = 0; i < strlen(s); i++) {
        if (s[i] == '+')
          return WILDCARD::PLUS;
        if (s[i] == '#')
          return WILDCARD::HASH;
      }
      return WILDCARD::NO;
    }

    static uint8_t count_substring(const string &str, const string &sub) {
      if (sub.empty())
        return 0;
      int count = 0;
      for (size_t offset = str.find(sub); offset != std::string::npos; offset = str.find(sub, offset + sub.length())) {
        ++count;
      }
      return count;
    }

    static string pad(const uint8_t total, const string &text, [[maybe_unused]] const bool ignoreAnsi = true) {
      auto text2 = string(text);
      for (size_t i = 0; i < (total - text.length()); i++) {
        text2 += ' ';
      }
      return text2;
    }

    static int no_ansi_length(const string &s) {
      int count = 0;
      bool last = false;
      for (size_t i = 0; i < s.length(); i++) {
        if (s[i] != '!' && !last)
          count++;
        if (last)
          last = false;
        if (s[i] == '!')
          last = true;
      }
      return count;
    }

    static bool look_ahead(const string &token, std::stringstream *ss, const bool consume = true) {
      const std::stringstream::pos_type start = ss->tellg();
      for (const char i: token) {
        if (i != ss->peek()) {
          ss->seekg(start);
          return false;
        }
        ss->get();
      }
      if (!consume)
        ss->seekg(start);
      return true;
    }

    static bool is_integer(const string &xstring) {
      for (uint8_t i = 0; i < xstring.length(); i++) {
        if (!isdigit(xstring[i]))
          return false;
      }
      return true;
    }

    static std::stringstream *eat_space(std::stringstream *ss) {
      while (!ss->eof()) {
        char c = (char) ss->peek();
        if (!isspace(c))
          return ss;
        else
          ss->get();
      }
      return ss;
    }
  };
} // namespace fhatos

#endif
