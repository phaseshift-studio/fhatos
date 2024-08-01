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
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](const char c) { return !std::isspace(c); }));
    }

    static void rtrim(std::string &s) {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](const char c) { return !std::isspace(c); }).base(), s.end());
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

    static int split(const char *text, const char *deliminator, char **&result, const uint8_t offset = 0) {
      char *copy = strdup(text);
      char *token = nullptr;
      int i = offset;
      if (strstr(text, deliminator)) {
        while ((token = strsep(&copy, deliminator)) != nullptr) {
          result[i] = (char *) malloc((strlen(token) + 1) * sizeof(char));
          strcpy(result[i], token);
          result[i][strlen(token)] = '\0';
          i++;
        }
      }
      size_t dl = strlen(deliminator);
      char *substr = new char[dl + 1];
      strncpy(substr, text + (strlen(text) - dl), dl);
      substr[dl] = '\0';
      if (strlen(substr) > 0 && strcmp(substr, deliminator) == 0) {
        result[i] = strdup("");
        i++;
      }
      free(token);
      free(copy);
      return i;
    }

    /*static bool match(const char *id_cstr, const char *pattern_cstr) {
      if (0 == strcmp(pattern_cstr, "#"))
        return true;
      if (!strstr(pattern_cstr, "#") && !strstr(pattern_cstr, "+"))
        return strcmp(id_cstr, pattern_cstr) == 0;
      // if (strlen(id_cstr) == 0 && strcmp(pattern_cstr, "#") == 0)
      // return true;
      char **idParts = new char *[FOS_MAX_FURI_SEGMENTS];
      char **patternParts = new char *[FOS_MAX_FURI_SEGMENTS];
      size_t idLength = split(id_cstr, "/", idParts);
      if (id_cstr[strlen(id_cstr) - 1] == '/')
        idLength++;
      const size_t patternLength = split(pattern_cstr, "/", patternParts);
      // LOG(DEBUG, "Matching: %s <=> %s\n", id, pattern);
      const bool result = [idParts, patternParts, idLength, patternLength]() {
        for (size_t i = 0; i < idLength; i++) {
          if (i >= patternLength)
            return false;
          //   LOG(DEBUG, "\t%s <=%i=> %s\n", idParts[i], i, patternParts[i]);
          if (strcmp(patternParts[i], "#") == 0)
            return true;
          if ((strcmp(patternParts[i], "+") != 0) && (strcmp(patternParts[i], idParts[i]) != 0))
            return false;
        }
        return patternLength == idLength;
      }();
      for (int i = 0; i < idLength; i++) {
        free((void *) idParts[i]);
      }
      for (int i = 0; i < patternLength; i++) {
        free((void *) patternParts[i]);
      }
      delete[] idParts;
      delete[] patternParts;
      return result;
    }*/
  };
} // namespace fhatos

#endif
