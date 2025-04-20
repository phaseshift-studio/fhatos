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

#include <algorithm>
#include <sstream>
#include <string>

#define FOS_HEX_STRING_MAX_LENGTH 8

namespace fhatos {
  using std::to_string;
  using std::string;
  using fbyte = uint8_t;

  enum class WILDCARD { NO = 0, PLUS = 1, HASH = 2 };

  class StringHelper final {
  public:
    StringHelper() = delete;

    static constexpr char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    static string clip_0x(const string &hex) {
      return hex.length() > 1 && hex[0] == '0' && hex[1] == 'x' ? hex.substr(2) : hex;
    }

    static string prefix_0x(const string &hex) {
      return hex.length() > 1 && hex[0] == '0' && hex[1] == 'x' ? hex : string("0x").append(hex);
    }

    static string bytes_to_hex(const std::vector<fbyte> &bytes) {
      const size_t size = bytes.size();
      auto hex_string = string(size * 2, ' ');
      for(size_t i = 0; i < size; i++) {
        hex_string[i * 2] = hexmap[(bytes[i] & 0xF0) >> 4];
        hex_string[i * 2 + 1] = hexmap[bytes[i] & 0x0F];
      }
      return hex_string;
    }

    static int hex_to_int(const string &hex) {
      int result = 0;
      sscanf(hex.c_str(), "%X", &result);
      return result;
    }

    static string int_to_hex(const int integer) {
      char buffer[FOS_HEX_STRING_MAX_LENGTH];
      const int s = sprintf(buffer, "%X", integer);
      return s % 2 == 0 ? string(buffer) : string("0").append(buffer);
    }

    static std::vector<fbyte> hex_to_bytes(const std::string &hex) {
      std::vector<fbyte> bytes;
      for(unsigned int i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        const char byte = static_cast<fbyte>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
      }
      return bytes;
    }

    static string cxx_f_metadata(const string &location, const string &name, const uint16_t line_number) {
      return location + "/" + name + "/" + name + "_" + to_string(line_number);
    }

    static string cxx_f_metadata(const string &file, const uint16_t line_number) {
      const size_t slash_index = file.find_last_of('/');
      const string dir = file.substr(slash_index, file.length() - slash_index - 4);
      string temp = dir + "/" + dir + "_" + to_string(line_number);
      // StringHelper::replace(temp, ".", "_");
      return temp;
    }

    static void truncate(std::string &input, const size_t width, const string &ellipsis = "!y.!g.!r.!!") {
      const std::vector<string> lines = StringHelper::tokenize('\n', input);
      string new_string;
      for(int i = 0; i < lines.size(); i++) {
        string line = lines.at(i);
        if(line.length() > width) {
          line.resize(width);
          line.append(ellipsis);
        }
        if(i < lines.size() - 1)
          line.append("\n");
        new_string.append(line);
      }
      input = new_string;
    }


    static void trim(std::string &s) {
      if(s.empty()) return;
      ltrim(s);
      rtrim(s);
    }

    static string repeat(const uint8_t amount, const string &repeater = " ") {
      string temp;
      for(int i = 0; i < amount; i++) {
        temp += repeater;
      }
      return temp;
    }

    static string format(const char *format, ...) {
      va_list arg;
      va_start(arg, format);
      char *message;
      const size_t length = vasprintf(&message, format, arg);
      va_end(arg);
      if(format[strlen(format) - 1] == '\n')
        message[length - 1] = '\n';
      message[length] = '\0';
      va_end(arg);
      const auto ret = string(message);
      free(message);
      return ret;
    }

    static void prefix_each_line(const string &prefix, string *to_prefix) {
      to_prefix->insert(0, prefix);
      for(size_t i = 0; i < to_prefix->length(); i++) {
        i = to_prefix->find("\n", i);
        if(i == string::npos || ++i > to_prefix->length())
          break;
        to_prefix->insert(i, prefix);
        i = i + prefix.length();
      }
    }


    static string replace_groups(std::string *s, const std::string &left_delim, const std::string &right_delim,
                                 const std::function<string(string)> &replace_function) {
      size_t left_pos = 0;
      size_t right_pos = 0;
      size_t temp_pos = 0;
      string new_string;
      while(left_pos != std::string::npos) {
        left_pos = s->find(left_delim, left_pos);
        if(left_pos != std::string::npos) {
          new_string += s->substr(right_pos, left_pos);
          temp_pos = left_pos + left_delim.length();
          if(right_pos = s->find(right_delim, left_pos + left_delim.length());
            right_pos != std::string::npos) {
            temp_pos = right_pos + right_delim.length();
            // Calculate the length of the substring to be replaced
            const size_t length_to_replace = right_pos - left_pos - left_delim.length();
            // Replace the content between delimiters
            const string to_replace = s->substr(left_pos + 1, length_to_replace);
            const string new_replace = replace_function(to_replace);
            new_string += new_replace;
            left_pos += new_replace.length();
          } else {
            new_string += s->substr(temp_pos, s->length() - temp_pos);
            break;
          }
        } else {
          new_string += s->substr(temp_pos, s->length() - temp_pos);
          break;
        }
      }
      return new_string;
    }

    static void ltrim(std::string &s) {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const char c) {
        return !std::isspace(c) && c < 127 && c != '\0';
      }));
    }

    static void rtrim(std::string &s) {
      s.erase(
          std::find_if(s.rbegin(), s.rend(), [](const char c) {
            return !std::isspace(c) && c < 127 && c != '\0';
          }).base(),
          s.end());
    }

    static bool has_wildcards(const std::string &s) {
      return s.find('+') != string::npos || s.find('#') != string::npos;
    }

    static void lower_case(string &s) {
      std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    }

    static uint8_t count_substring(const string &str, const string &sub) {
      if(sub.empty())
        return 0;
      int count = 0;
      for(size_t offset = str.find(sub); offset != std::string::npos; offset = str.find(sub, offset + sub.length())) {
        ++count;
      }
      return count;
    }

    static string substring(const char split, const string &parent) {
      const size_t index = parent.find(split);
      if(index == string::npos)
        return parent;
      return parent.substr(index + 1);
    }

    template<typename T = string>
    static std::vector<T> tokenize(const char split, const string &parent,
                                   const std::function<T(const string &)> transformer = [](const string &s) {
                                     return s;
                                   }) {
      std::vector<T> tokens;
      string sub = parent;
      size_t index;
      while(string::npos != (index = sub.find(split))) {
        const string temp = sub.substr(0, index);
        const T temp2 = transformer(temp);
        tokens.push_back(temp2);
        if(sub.empty())
          break;
        sub = sub.substr(index + 1);
      }
      tokens.push_back(transformer(sub));
      return tokens;
    }

    static string pad(const uint8_t total, const string &text) {
      auto text2 = string(text);
      for(size_t i = 0; i < (total - text.length()); i++) {
        text2 += ' ';
      }
      return text2;
    }

    static int no_ansi_length(const string &s) {
      int count = 0;
      bool last = false;
      for(size_t i = 0; i < s.length(); i++) {
        if(s[i] != '!' && !last)
          count++;
        if(last)
          last = false;
        if(s[i] == '!')
          last = true;
      }
      return count;
    }

    static bool look_ahead(const string &token, std::stringstream *ss, const bool consume = true) {
      const std::stringstream::pos_type start = ss->tellg();
      for(const char i: token) {
        if(i != ss->peek()) {
          ss->seekg(start);
          return false;
        }
        ss->get();
      }
      if(!consume)
        ss->seekg(start);
      return true;
    }

    static string next_token(const char &split, std::stringstream *ss) {
      string token;
      while(!ss->eof()) {
        const char c = ss->get();
        if(c == EOF || c == '\0' || (c == split && !token.empty()))
          break;
        token += c;
      }
      return token;
    }

    static bool is_integer(const string &xstring) {
      if(xstring.empty())
        return false;
      for(uint8_t i = 0; i < xstring.length(); i++) {
        if(!isdigit(xstring[i]))
          return false;
      }
      return true;
    }

    static void replace(string *s, const string &search, const string &replace, const bool forward = true) {
      if(search.empty() || s->length() < search.length())
        return;
      if(!forward) {
        const size_t pos = s->rfind(search, s->length() - 1);
        if(pos == string::npos)
          return;
        s->erase(pos, search.length());
        if(!replace.empty())
          s->insert(pos, replace);
      } else {
        for(size_t pos = 0;; pos += replace.length()) {
          // Locate the substring to replace
          pos = s->find(search, pos);
          if(pos == string::npos)
            break;
          // Replace by erasing and inserting
          s->erase(pos, search.length());
          s->insert(pos, replace);
        }
      }
    }

    static std::stringstream *eat_space(std::stringstream *ss) {
      while(!ss->eof()) {
        if(const char c = static_cast<char>(ss->peek()); !isspace(c))
          return ss;
        ss->get();
      }
      return ss;
    }

    static bool char_ptr_equal(const char *a, const char *b) {
      if(!a)
        return !b;
      if(!b)
        return false;
      return strcmp(a, b) == 0;
    }
  };
} // namespace fhatos

#endif
