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
#ifndef fhatos_uri_hpp
#define fhatos_uri_hpp

#include <fhatos.hpp>
#include <sstream>

#include "string_helper.hpp"
namespace fhatos {
  class UriX {

  public:
    const char *_scheme = nullptr;
    const char *_user = nullptr;
    const char *_password = nullptr;
    const char *_host = nullptr;
    uint16_t _port = 0;
    const char **_path = nullptr;
    bool sprefix = false;
    bool spostfix = false;
    uint8_t _pathLength = 0;
    const char *_query = nullptr;
    const char *_fragment = nullptr;

    UriX(const string &furiString) {
      this->_path = new const char *[10];
      std::stringstream ss = std::stringstream(furiString);
      string token;
      string part = "scheme";
      bool hasUserInfo = furiString.find_first_of('@') != string::npos;
      bool foundAuthority = false;
      while (!ss.eof()) {
        char t = ss.get();
        if (!foundAuthority && t == '/' && ss.peek() == '/') {
          foundAuthority = true;
          if (part == "scheme" || part == "user") {
            part = "user";
            ss.get();
          }
        } else if (t == ':') {
          if (part == "scheme") {
            this->_scheme = strdup(token.c_str());
            part = "user";
            token.clear();
          } else if (part == "user") {
            if (hasUserInfo) {
              this->_user = strdup(token.c_str());
              part = "password";
            } else {
              this->_host = strdup(token.c_str());
              part = "port";
            }
            token.clear();
          } else if (part == "host") {
            this->_host = strdup(token.c_str());
            part = "port";
            token.clear();
          } else {
            token += t;
          }
        } else if (t == '@') {
          if (part == "user" || part == "password") {
            if (this->_user) {
              this->_password = strdup(token.c_str());
            } else {
              this->_user = strdup(token.c_str());
            }
            part = "host";
            token.clear();
          } else {
            token += t;
          }
        } else if (t == '/') {
          if (part == "port") {
            this->_port = stoi(token);
            part = "path";
            this->sprefix = true;
            token.clear();
          } else if (part == "scheme" || part == "host" || part == "user" || part == "password") {
            if (foundAuthority) {
              this->_host = strdup(token.c_str());
              part = "path";
              this->sprefix = true;
            } else {
              if (!token.empty()) {
                this->_path[this->_pathLength] = strdup(token.c_str());
                this->_pathLength = this->_pathLength + 1;
              } else {
                this->sprefix = true;
              }
              part = "path";
            }
            token.clear();
          } else if (part == "path") {
            this->_path[this->_pathLength] = strdup(token.c_str());
            this->_pathLength = this->_pathLength + 1;
            this->spostfix = true;
            token.clear();
          } else {
            token += t;
          }
        } else if (t == '?') {
          if (part == "path" || part == "scheme") {
            if (!token.empty()) {
              this->_path[this->_pathLength] = strdup(token.c_str());
              this->_pathLength = this->_pathLength + 1;
            } else
              this->spostfix = true;
            part = "query";
            token.clear();
          } else if (part == "host" || part == "user") {
            _host = strdup(token.c_str());
            part = "query";
            token.clear();
          } else {
            token += t;
          }
        } else if (t == '#') {
          if (part == "path" || part == "scheme") {
            if (!token.empty()) {
              this->_path[this->_pathLength] = strdup(token.c_str());
              this->_pathLength = this->_pathLength + 1;
            } else
              this->spostfix = true;
            part = "fragment";
            token.clear();
          } else if (part == "host" || part == "user") {
            _host = strdup(token.c_str());
            part = "fragment";
            token.clear();
          } else if (part == "query") {
            this->_query = strdup(token.c_str());
            part = "fragment";
            token.clear();
          } else {
            token += t;
          }
        } else if (t != '\xFF') {
          this->spostfix = false;
          token += t;
        }
      }
      if (!token.empty()) {
        if ((!foundAuthority && part != "fragment" && part != "query") || part == "path" || part == "scheme") {
          this->_path[this->_pathLength] = strdup(token.c_str());
          this->_pathLength = this->_pathLength + 1;
        } else if (part == "host" || part == "user") {
          this->_host = strdup(token.c_str());
        } else if (part == "port") {
          this->_port = stoi(token);
        } else if (part == "query") {
          this->_query = strdup(token.c_str());
        } else if (part == "fragment") {
          this->_fragment = strdup(token.c_str());
        }
      }
    }

    const bool operator==(const UriX &other) const { return this->toString() == other.toString(); }
    const bool equals(const UriX &other) const { return *this == other; }

    const string toString() const {
      LOG(INFO, "%i\n", this->_pathLength);
      string uri;
      if (this->_scheme)
        uri.append(this->_scheme).append(":");
      if (this->_host || this->_user) {
        uri.append("//");
        if (this->_user) {
          uri.append(this->_user);
          if (this->_password)
            uri.append(":").append(this->_password);
          uri.append("@");
        }
        if (this->_host) {
          uri.append(this->_host);
          if (this->_port > 0)
            uri.append(":").append(std::to_string(this->_port));
        }
      }
      for (int i = 0; i < this->_pathLength; i++) {
        if (i != 0 || this->sprefix)
          uri.append("/");
        uri.append(this->_path[i]);
        if (this->spostfix)
          uri.append("/");
      }
      if (this->_query)
        uri.append("?").append(this->_query);
      if (this->_fragment)
        uri.append("#").append(this->_fragment);
      return uri;
    }
  };
} // namespace fhatos

#endif
