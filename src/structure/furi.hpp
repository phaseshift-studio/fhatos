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

#include <fhatos.hpp>
//
#include <sstream>
#include <util/string_helper.hpp>

namespace fhatos {

  enum class URI_PART { SCHEME, USER, PASSWORD, HOST, PORT, PATH, FRAGMENT, QUERY };
  const static Enums<URI_PART> URI_PARTS = {
      {URI_PART::SCHEME, "scheme"},     {URI_PART::USER, "user"},   {URI_PART::PASSWORD, "password"},
      {URI_PART::HOST, "host"},         {URI_PART::PORT, "port"},   {URI_PART::PATH, "path"},
      {URI_PART::FRAGMENT, "fragment"}, {URI_PART::QUERY, "query"},
  };

  class fURI {

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
    fURI scheme(const char *scheme) {
      fURI newURI = fURI(*this);
      newURI._scheme = 0 == strlen(scheme) ? nullptr : scheme;
      return newURI;
    }
    /// USER
    const char *user() const { return this->_user ? this->_user : EMPTY_CHARS; }
    fURI user(const char *user) const {
      fURI newURI = fURI(*this);
      newURI._user = 0 == strlen(user) ? nullptr : user;
      return newURI;
    }
    /// PASSWORD
    const char *password() const { return this->_password ? this->_password : EMPTY_CHARS; }
    fURI password(const char *password) const {
      fURI newURI = fURI(*this);
      newURI._password = 0 == strlen(password) ? nullptr : password;
      return newURI;
    }
    /// HOST
    const char *host() const { return this->_host ? this->_host : EMPTY_CHARS; }
    fURI host(const char *host) const {
      fURI newURI = fURI(*this);
      newURI._host = (0 == strlen(host) ? nullptr : host);
      return newURI;
    }
    /// PORT
    uint16_t port() const { return this->_port; }
    fURI port(const uint16_t port) const {
      fURI newURI = fURI(*this);
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
        if (this->sprefix && start == 0)
          path_str += '/';
        for (uint8_t i = start; i < end && i < this->_path_length; i++) {
          path_str += this->_path[i];
          if (i != end - 1)
            path_str += '/';
        }
        if (this->spostfix && end >= this->_path_length)
          path_str += '/';
      }
      return path_str;
    }
    string path() const { return this->path(0, this->path_length()); }
    const char *path(const uint8_t segment) const {
      return (this->_path && this->_path_length > segment) ? this->_path[segment] : EMPTY_CHARS;
    }
    fURI path(const string &path) const {
      fURI newURI = fURI(this->toString());
      newURI._path = new const char *[20]; // TODO: make dynamic/intelligent
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

    bool empty() const {
      return this->_path_length == 0 && !this->_host && !this->_scheme && !this->_user && !this->_password &&
             !this->_fragment && !this->_query;
    }
    uint8_t path_length() const { return this->_path_length; }
    /// QUERY
    const char *query() const { return this->_query ? this->_query : EMPTY_CHARS; }
    fURI query(const char *query) const {
      fURI newURI = fURI(*this);
      newURI._query = 0 == strlen(query) ? nullptr : query;
      return newURI;
    }
    /// FRAGMENT
    const char *fragment() const { return this->_fragment ? this->_fragment : EMPTY_CHARS; }
    fURI fragment(const char *fragment) const {
      fURI newURI = fURI(*this);
      newURI._fragment = 0 == strlen(fragment) ? nullptr : fragment;
      return newURI;
    }
    ////////////////////////////////////////////////////////////////

    fURI extend(const char *extension) const {
      return this->path(this->path() + (this->path().ends_with("/") ? "" : "/") + extension);
    }

    fURI retract() const {
      fURI newURI = fURI(*this);
      if (this->_path_length > 0) {
        newURI._path_length = newURI._path_length - 1;
      } else {
        newURI.sprefix = false;
      }
      return newURI;
    }

    fURI resolve(const fURI &other) const {
      ///////////////////////////////////////////////////////////////
      ////////////  mm-ADT specific resolution pattern //////////////
      ///////////////////////////////////////////////////////////////
      ///         /abc ~> :xyz => /abc:xyz  NOT /:xyz             ///
      ///////////////////////////////////////////////////////////////
      if ((!other.toString().empty() && other.toString()[0] == ':') ||
          (!this->toString().empty() && this->toString()[this->toString().length() - 1] == ':'))
        return fURI(this->toString() + other.toString());
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
      fURI newURI = fURI(*this);
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


    virtual bool matches(const fURI &other) const {
      return StringHelper::match(this->toString().c_str(), other.toString().c_str());
    }
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    virtual ~fURI() = default;
    fURI(const char *uriChars) : fURI(string(uriChars)) {}
    fURI(const string &uriString) {
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
            this->_query = "";
            token.clear();
          } else if (part == URI_PART::HOST || part == URI_PART::USER) {
            _host = strdup(token.c_str());
            part = URI_PART::QUERY;
            token.clear();
          } else {
            token += t;
          }
        } /*else if (t == '#') {
          if (part == URI_PART::PATH || part == URI_PART::SCHEME) {
            if (!token.empty()) {
              this->_path[this->_path_length] = strdup(token.c_str());
              this->_path_length = this->_path_length + 1;
            } else
              this->spostfix = true;
            part = URI_PART::FRAGMENT;
            this->_fragment = "";
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
        }*/
        else if (t != '\xFF') {
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

    bool operator<(const fURI &other) const { return this->toString() < other.toString(); }
    bool operator!=(const fURI &other) const { return !this->equals(other); }
    bool operator==(const fURI &other) const {
      return this->toString() == other.toString();
    } // TODO: do field-wise comparisons
    bool equals(const fURI &other) const { return *this == other; }

    const string toString() const {
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
      // if (this->_fragment)
      //   uri.append("#").append(this->_fragment);
      return uri;
    }
  };

  class ID final : public fURI {
  public:
    ID(const fURI &id) : ID(id.toString()) {}

    ID(const string &furiString) : fURI(furiString) {}

    ID(const char *furiCharacters) : fURI(furiCharacters) {
      try {
        if (strchr(furiCharacters, '#')) {
          throw fError("%s\n", "IDs can not contain pattern symbols: #");
        } else if (strchr(furiCharacters, '+')) {
          throw fError("%s\n", "IDs can not contain pattern symbols: +");
        }
      } catch (const fError &e) {
        if (this->_path_length > 0) {
          for (uint8_t i = 0; i < this->_path_length; i++) {
            delete this->_path[i];
          }
          delete _path;
        }
        this->_path_length = 0;
        throw;
      }
    }

    // const bool isPattern() const override { return false; }
  };

  using ID_p = ptr<ID>;

  using SourceID = ID;
  using TargetID = ID;

  class Pattern : public fURI {
  public:
    Pattern(const fURI &uri) : Pattern(uri.toString()) {}

    // Pattern(const ID &id) : Pattern(id.toString()) {
    // }

    Pattern(const string &uriString) : fURI(uriString){};

    Pattern(const char *uriChars) : fURI(uriChars){};

    /* bool colocated(const fURI &furi) const override {
       return furi.authority() == "#" || furi.authority().find('+') > -1 || fURI::colocated(furi);
     }*/

    bool matches(const fURI &pattern) const override {
      return StringHelper::match(pattern.toString().c_str(), this->toString().c_str());
    }
  };

  using fURI_p = ptr<fURI>;
  class BaseIDed {
  public:
    virtual ~BaseIDed() = default;
    virtual ID_p id() const { return nullptr; }
    virtual bool equals(const BaseIDed &) const { return false; }
  };
  using Patter_p = ptr<Pattern>;

  class IDed : public BaseIDed {
  public:
    ~IDed() override = default;

    explicit IDed(const fURI_p &uri) : _id(share(ID(*uri))) {}
    explicit IDed(const ID_p &id) : _id(id) {}

    ID_p id() const override { return this->_id; }

    // const String toString() const { return this->id().toString(); }

    bool equals(const BaseIDed &other) const override { return this->_id->equals(*other.id()); }


  protected:
    ptr<ID> _id;
  };

  struct furi_comp : public std::less<fURI_p> {
    auto operator()(const fURI_p &a, const fURI_p &b) const { return a->toString() < b->toString(); }
  };
} // namespace fhatos

#endif
