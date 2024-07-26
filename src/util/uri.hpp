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
#ifndef fhatos_uri_hpp
#define fhatos_uri_hpp

#include "string_helper.hpp"


#include <fhatos.hpp>
#include <sstream>

namespace fhatos {

  enum class URI_PART { SCHEME, USER, PASSWORD, HOST, PORT, PATH, FRAGMENT, QUERY };
  const static Enums<URI_PART> URI_PARTS = {
      {URI_PART::SCHEME, "scheme"},     {URI_PART::USER, "user"},   {URI_PART::PASSWORD, "password"},
      {URI_PART::HOST, "host"},         {URI_PART::PORT, "port"},   {URI_PART::PATH, "path"},
      {URI_PART::FRAGMENT, "fragment"}, {URI_PART::QUERY, "query"},
  };

  class UriX {

  protected:
    const char *_scheme = nullptr;
    const char *_user = nullptr;
    const char *_password = nullptr;
    const char *_host = nullptr;
    uint16_t _port = 0;
    const char **_path = nullptr;
    bool sprefix = false;
    bool spostfix = false;
    uint8_t _path_length = 0;
    const char *_query = nullptr;
    const char *_fragment = nullptr;

  public:
    const char *scheme() const { return this->_scheme ? this->_scheme : EMPTY_CHARS; }
    UriX scheme(const char *scheme) {
      UriX newURI = UriX(*this);
      newURI._scheme = 0 == strlen(scheme) ? nullptr : scheme;
      return newURI;
    }
    /// USER
    const char *user() const { return this->_user ? this->_user : EMPTY_CHARS; }
    UriX user(const char *user) const {
      UriX newURI = UriX(*this);
      newURI._user = 0 == strlen(user) ? nullptr : user;
      return newURI;
    }
    /// PASSWORD
    const char *password() const { return this->_password ? this->_password : EMPTY_CHARS; }
    UriX password(const char *password) const {
      UriX newURI = UriX(*this);
      newURI._password = 0 == strlen(password) ? nullptr : password;
      return newURI;
    }
    /// HOST
    const char *host() const { return this->_host ? this->_host : EMPTY_CHARS; }
    UriX host(const char *host) const {
      UriX newURI = UriX(*this);
      newURI._host = 0 == strlen(host) ? nullptr : host;
      return newURI;
    }
    /// PORT
    uint16_t port() const { return this->_port; }
    UriX port(const uint16_t port) const {
      UriX newURI = UriX(*this);
      newURI._port = port;
      return newURI;
    }
    /// AUTHORITY
    string authority() const {
      string _authority;
      if (this->_user) {
        _authority += this->_user;
      }
      if (this->_password) {
        _authority += ':';
        _authority += this->_password;
      }
      if (this->_host) {
        if (this->_user || this->_password)
          _authority += '@';
        _authority += this->_host;
      }
      if (this->_port) {
        _authority += ':';
        _authority += to_string(this->_port);
      }
      return _authority;
    }

    /// PATH
    string path(const uint8_t start, const uint8_t end) const {
      string path_str;
      if (this->_path) {
        if (this->sprefix)
          path_str += '/';
        for (uint8_t i = start; i < end && i < this->_path_length; i++) {
          path_str += this->_path[i];
          if (i != end - 1 || i != this->_path_length - 1 || this->spostfix)
            path_str += '/';
        }
      }
      return path_str;
    }
    string path() const { return this->path(0, this->path_length()); }
    const char *path(const uint8_t segment) const {
      return (this->_path && this->_path_length > segment) ? this->_path[segment] : EMPTY_CHARS;
    }
    UriX path(const string &path) const {
      UriX newURI = UriX(this->toString());
      newURI._path = new const char *[10];
      newURI._path_length = 0;
      std::stringstream ss = std::stringstream(path);
      string segment;
      uint8_t i = 0;
      while (!ss.eof()) {
        char c = ss.get();
        if (c == EOF)
          break;
        if (c == '/') {
          newURI._path[i] = strdup(segment.c_str());
          segment.clear();
          i++;
        } else {
          segment += c;
        }
      }
      newURI.sprefix = false;
      if (segment.empty()) {
        newURI._path_length = i;
        newURI.spostfix = true;
      } else {
        newURI._path[i] = strdup(segment.c_str());
        newURI._path_length = i + 1;
        newURI.spostfix = false;
      }
      return newURI;
    }

    const char *name() const { return 0 == this->_path_length ? EMPTY_CHARS : this->_path[this->_path_length - 1]; }

    uint8_t path_length() const { return this->_path_length; }
    /// QUERY
    const char *query() const { return this->_query ? this->_query : EMPTY_CHARS; }
    UriX query(const char *query) const {
      UriX newURI = UriX(*this);
      newURI._query = 0 == strlen(query) ? nullptr : query;
      return newURI;
    }
    /// FRAGMENT
    const char *fragment() const { return this->_fragment ? this->_fragment : EMPTY_CHARS; }
    UriX fragment(const char *fragment) const {
      UriX newURI = UriX(*this);
      newURI._fragment = 0 == strlen(fragment) ? nullptr : fragment;
      return newURI;
    }
    ////////////////////////////////////////////////////////////////

    UriX extend(const char *extension) const {
      return this->path(this->path() + (this->path().ends_with("/") ? "" : "/") + extension);
    }

    UriX retract() const {
      UriX newURI = UriX(*this);
      if (this->_path_length > 0) {
        newURI._path_length = newURI._path_length - 1;
      } else {
        newURI.sprefix = false;
      }
      return newURI;
    }

    UriX resolve(const UriX &other) const {
      ///////////////////////////////////////////////////////////////
      ////////////  mm-ADT specific resolution pattern //////////////
      ///////////////////////////////////////////////////////////////
      ///         /abc ~> :xyz => /abc:xyz  NOT /:xyz             ///
      ///////////////////////////////////////////////////////////////
      if ((!other.toString().empty() && other.toString()[0] == ':') ||
          (!this->path().empty() &&
           this->path()[this->path().length() - 1] == ':'))
        return this->path(this->path() + other.toString());
      ///////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////
      if (other._path_length == 0)
        return *this;
      if (other.path().find('.') == string::npos) {
        string otherPath = other.path();
        if (otherPath[0] == '/')
          return this->path(other.path());
        if (this->_path_length == 0)
          return this->extend(other.path().c_str());
        if (this->spostfix)
          return this->extend(other.path().c_str());
        if (this->path().find('/') == string::npos)
          return this->path(other.path());
        return this->retract().extend(other.path().c_str());
      }
      UriX newURI = UriX(*this);
      if (!newURI.spostfix)
        newURI = newURI.retract();
      for (uint8_t i = 0; i < other._path_length; i++) {
        if (strcmp(other.path(i), "..") == 0) {
          newURI = newURI.retract();

        } else if (strcmp(other.path(i), ".") != 0)
          newURI = newURI.extend(other.path(i));
        if (other.path()[other.path().length() - 1] == '/')
          newURI = newURI.extend("");
      }
      return newURI;
    }


    bool match(const UriX &other) const {
      return StringHelper::match(this->toString().c_str(), other.toString().c_str());
    }
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ~UriX() = default;
    UriX(const char *uriChars) : UriX(string(uriChars)) {}
    UriX(const string &uriString) {
      this->_path = new const char *[10];
      std::stringstream ss = std::stringstream(uriString);
      string token;
      URI_PART part = URI_PART::SCHEME;
      bool hasUserInfo = uriString.find_first_of('@') != string::npos;
      bool foundAuthority = false;
      while (!ss.eof()) {
        char t = ss.get();
        if (!foundAuthority && t == '/' && ss.peek() == '/') {
          foundAuthority = true;
          if (part == URI_PART::SCHEME || part == URI_PART::USER) {
            part = URI_PART::USER;
            ss.get();
          }
        } else if (t == ':') {
          if (part == URI_PART::SCHEME) {
            this->_scheme = strdup(token.c_str());
            part = URI_PART::USER;
            token.clear();
          } else if (part == URI_PART::USER) {
            if (hasUserInfo) {
              this->_user = strdup(token.c_str());
              part = URI_PART::PASSWORD;
            } else {
              this->_host = strdup(token.c_str());
              part = URI_PART::PORT;
            }
            token.clear();
          } else if (part == URI_PART::HOST) {
            this->_host = strdup(token.c_str());
            part = URI_PART::PORT;
            token.clear();
          } else {
            token += t;
          }
        } else if (t == '@') {
          if (part == URI_PART::USER || part == URI_PART::PASSWORD) {
            if (this->_user) {
              this->_password = strdup(token.c_str());
            } else {
              this->_user = strdup(token.c_str());
            }
            part = URI_PART::HOST;
            token.clear();
          } else {
            token += t;
          }
        } else if (t == '/') {
          if (part == URI_PART::PORT) {
            this->_port = stoi(token);
            part = URI_PART::PATH;
            this->sprefix = true;
            token.clear();
          } else if (part == URI_PART::SCHEME || part == URI_PART::HOST || part == URI_PART::USER ||
                     part == URI_PART::PASSWORD) {
            if (foundAuthority) {
              this->_host = strdup(token.c_str());
              part = URI_PART::PATH;
              this->sprefix = true;
            } else {
              if (!token.empty()) { // TODO: what about empty components?
                this->_path[this->_path_length] = strdup(token.c_str());
                this->_path_length = this->_path_length + 1;
              } else {
                this->sprefix = true;
              }
              part = URI_PART::PATH;
            }
            token.clear();
          } else if (part == URI_PART::PATH) {
            this->_path[this->_path_length] = strdup(token.c_str());
            this->_path_length = this->_path_length + 1;
            this->spostfix = true;
            token.clear();
          } else {
            token += t;
          }
        } else if (t == '?') {
          if (part == URI_PART::PATH || part == URI_PART::SCHEME) {
            if (!token.empty()) {
              this->_path[this->_path_length] = strdup(token.c_str());
              this->_path_length = this->_path_length + 1;
            } else
              this->spostfix = true;
            part = URI_PART::QUERY;
            token.clear();
          } else if (part == URI_PART::HOST || part == URI_PART::USER) {
            _host = strdup(token.c_str());
            part = URI_PART::QUERY;
            token.clear();
          } else {
            token += t;
          }
        } else if (t == '#') {
          if (part == URI_PART::PATH || part == URI_PART::SCHEME) {
            if (!token.empty()) {
              this->_path[this->_path_length] = strdup(token.c_str());
              this->_path_length = this->_path_length + 1;
            } else
              this->spostfix = true;
            part = URI_PART::FRAGMENT;
            token.clear();
          } else if (part == URI_PART::HOST || part == URI_PART::USER) {
            _host = strdup(token.c_str());
            part = URI_PART::FRAGMENT;
            token.clear();
          } else if (part == URI_PART::QUERY) {
            this->_query = strdup(token.c_str());
            part = URI_PART::FRAGMENT;
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
        if ((!foundAuthority && part != URI_PART::FRAGMENT && part != URI_PART::QUERY) || part == URI_PART::PATH ||
            part == URI_PART::SCHEME) {
          this->_path[this->_path_length] = strdup(token.c_str());
          this->_path_length = this->_path_length + 1;
        } else if (part == URI_PART::HOST || part == URI_PART::USER) {
          this->_host = strdup(token.c_str());
        } else if (part == URI_PART::PORT) {
          this->_port = stoi(token);
        } else if (part == URI_PART::QUERY) {
          this->_query = strdup(token.c_str());
        } else if (part == URI_PART::FRAGMENT) {
          this->_fragment = strdup(token.c_str());
        }
      }
    }

    bool operator!=(const UriX &other) const { return !this->equals(other); }
    bool operator==(const UriX &other) const {
      return this->toString() == other.toString();
    } // TODO: do field-wise comparisons
    bool equals(const UriX &other) const { return *this == other; }

    const string toString() const {
      LOG(INFO, "%i\n", this->_path_length);
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
      if (this->sprefix)
        uri.append("/");
      for (int i = 0; i < this->_path_length; i++) {
        uri.append(this->_path[i]);
        if (i < (this->_path_length - 1) || this->spostfix)
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
