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
    char **_path = nullptr;
    bool sprefix = false;
    bool spostfix = false;
    uint8_t _path_length = 0;
    const char *_query = nullptr;
    const char *_fragment = nullptr;

  public:
    const char *scheme() const { return this->_scheme ? this->_scheme : EMPTY_CHARS; }
    fURI scheme(const char *scheme) {
      fURI newURI = fURI(*this);
      free((void *) newURI._scheme);
      newURI._scheme = 0 == strlen(scheme) ? nullptr : strdup(scheme);
      return newURI;
    }
    /// USER
    const char *user() const { return this->_user ? this->_user : EMPTY_CHARS; }
    fURI user(const char *user) const {
      fURI newURI = fURI(*this);
      free((void *) newURI._user);
      newURI._user = 0 == strlen(user) ? nullptr : strdup(user);
      return newURI;
    }
    /// PASSWORD
    const char *password() const { return this->_password ? this->_password : EMPTY_CHARS; }
    fURI password(const char *password) const {
      fURI newURI = fURI(*this);
      free((void *) newURI._password);
      newURI._password = 0 == strlen(password) ? nullptr : strdup(password);
      return newURI;
    }
    /// HOST
    const char *host() const { return this->_host ? this->_host : EMPTY_CHARS; }
    fURI host(const char *host) const {
      fURI newURI = fURI(*this);
      free((void *) newURI._host);
      newURI._host = (0 == strlen(host) ? nullptr : strdup(host));
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
      fURI newURI = fURI("");
      newURI._scheme = this->_scheme ? strdup(this->_scheme) : nullptr;
      newURI._user = this->_user ? strdup(this->_user) : nullptr;
      newURI._password = this->_password ? strdup(this->_password) : nullptr;
      newURI._host = this->_host ? strdup(this->_host) : nullptr;
      newURI._port = this->_port;
      newURI._query = this->_query ? strdup(this->_query) : nullptr;
      newURI._path_length = 0;
      newURI._path = new char *[10];
      char *dup = strdup(path.c_str());
      std::stringstream ss = std::stringstream(dup);
      string segment;
      uint8_t i = 0;
      char c;
      while (ss.get(c)) {
        if (c == '\0' || isspace(c))
          break;
        if (c == '/') {
          if (segment.empty() && 0 == i) {
            newURI.sprefix = true;
          } else {
            newURI._path[i] = strdup(segment.c_str());
            i++;
          }
          segment.clear();
        } else {
          segment += c;
        }
      }
      StringHelper::trim(segment);
      if (segment.empty()) {
        newURI._path_length = i;
        newURI.spostfix = true;
      } else {
        newURI._path[i] = strdup(segment.c_str());
        newURI._path_length = i + 1;
        newURI.spostfix = false;
#ifndef NATIVE
        int x = strlen(newURI._path[newURI._path_length - 1]);
        newURI._path[newURI._path_length - 1][x - 1] = '\0';
#endif
      }
      free(dup);
      return newURI;
    }

    const char *name() const {
      if (0 == this->_path_length)
        return EMPTY_CHARS;
      for (int i = this->_path_length - 1; i >= 0; i--) {
        if (strlen(this->_path[i]) > 0)
          return this->_path[i];
      }
      return EMPTY_CHARS;
    }

    bool empty() const {
      return this->_path_length == 0 && !this->_host && !this->_scheme && !this->_user && !this->_password &&
             !this->_fragment && !this->_query;
    }
    uint8_t path_length() const { return this->_path_length; }
    /// QUERY
    const char *query() const { return this->_query ? this->_query : EMPTY_CHARS; }
    fURI query(const char *query) const {
      fURI newURI = fURI(*this);
      FOS_SAFE_FREE(newURI._query);
      newURI._query = 0 == strlen(query) ? nullptr : strdup(query);
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
      if (strlen(extension) == 0) {
        fURI newURI = fURI(*this);
        newURI.spostfix = true;
        return newURI;
      }
    // const std::unique_ptr<char, void (*)(void *)> newPathChars =
      //    std::unique_ptr<char, void (*)(void *)>(strdup(this->path().c_str()), free);
      string newPath = string(this->path().c_str());
      if (!this->spostfix)
        newPath += "/";
      if (newPath.empty())
        return this->path(extension);
      newPath += extension;
      return this->path(newPath);
    }

    fURI retract() const {
      if (this->_path_length == 0)
        return *this;
      fURI newURI = fURI(this->toString());
      FOS_SAFE_FREE(newURI._path[newURI._path_length - 1]);
      newURI._path_length = newURI._path_length - 1;
      newURI.spostfix = true;
      return newURI;
    }

    bool is_subfuri_of(const fURI other) const {
      const string this_string = this->toString();
      const string other_string = other.toString();
      return other_string.length() >= this_string.length() &&
             other_string.substr(0, this_string.length()) == this_string;
    }

    virtual fURI resolve(const fURI &other) const {
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
        const std::unique_ptr<char, void (*)(void *)> otherPathChars =
            std::unique_ptr<char, void (*)(void *)>(strdup(other.path().c_str()), free);
        if ((&*otherPathChars)[0] == '/' || this->path().find('/') == string::npos)
          return this->path(&*otherPathChars);
        if (this->spostfix)
          return this->extend(&*otherPathChars);
        return this->retract().extend(&*otherPathChars);
      }
      fURI *newURI = this->spostfix ? new fURI(*this) : new fURI(this->retract());
      for (uint8_t i = 0; i < other._path_length; i++) {
        if (strcmp(other._path[i], "..") == 0)
          newURI = new fURI(newURI->retract());
        else if (strcmp(other._path[i], ".") != 0)
          newURI = new fURI(newURI->extend(other._path[i]));
      }
      newURI->spostfix = other.spostfix;
      fURI x = fURI(*newURI);
      delete newURI;
      return x;
    }


    virtual bool matches(const fURI &pattern) const {
      string patternStr = pattern.toString();
      if (pattern.toString() == "#")
        return true;
      if (patternStr.find('+') == string::npos && patternStr.find('#') == string::npos)
        return this->toString() == patternStr;
      if (strcmp(pattern.scheme(), "#") == 0)
        return true;
      if (strcmp(pattern.scheme(), "+") != 0 && strcmp(this->scheme(), pattern.scheme()) != 0)
        return false;
      if (strcmp(pattern.host(), "#") == 0)
        return true;
      if (strcmp(pattern.host(), "+") != 0 && strcmp(this->host(), pattern.host()) != 0)
        return false;
      if (strcmp(pattern.user(), "#") == 0)
        return true;
      if (strcmp(pattern.user(), "+") != 0 && strcmp(this->user(), pattern.user()) != 0)
        return false;
      if (strcmp(pattern.password(), "#") == 0)
        return true;
      if (strcmp(pattern.password(), "+") != 0 && strcmp(this->password(), pattern.password()) != 0)
        return false;
      for (size_t i = 0; i < pattern.path_length(); i++) {
        if (this->_path_length <= i)
          return false;
        if (strcmp(pattern.path(i), "#") == 0)
          return true;
        if (strcmp(pattern.path(i), "+") != 0 && strcmp(this->path(i), pattern.path(i)) != 0)
          return false;
      }
      return this->_path_length == pattern.path_length();
    }
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    virtual ~fURI() {
      free((void *) this->_scheme);
      free((void *) this->_host);
      free((void *) this->_user);
      free((void *) this->_password);
      free((void *) this->_query);
      free((void *) this->_fragment);
      for (size_t i = 0; i < this->_path_length; i++) {
        free(_path[i]);
      }
      delete[] _path;
    };
    fURI(const fURI &other) : fURI(other.toString().c_str()) {}
    fURI(const string &uriString) : fURI(uriString.c_str()) {}
    fURI(const char *uriChars) {
      if (strlen(uriChars) > 0) {
        const char *dups = strdup(uriChars);
        std::stringstream ss = std::stringstream(dups);
        string token;
        URI_PART part = URI_PART::SCHEME;
        bool hasUserInfo = strchr(dups, '@') != nullptr;
        bool foundAuthority = false;
        while (!ss.eof()) {
          char t = ss.get();
          if (!isascii(t))
            continue;
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
                  if (!this->_path)
                    this->_path = new char *[10];
                  this->_path[this->_path_length] = strdup(token.c_str());
                  this->_path_length = this->_path_length + 1;
                } else {
                  this->sprefix = true;
                }
                part = URI_PART::PATH;
              }
              token.clear();
            } else if (part == URI_PART::PATH) {
              if (!this->_path)
                this->_path = new char *[10];
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
                if (!this->_path)
                  this->_path = new char *[10];
                this->_path[this->_path_length] = strdup(token.c_str());
                this->_path_length = this->_path_length + 1;
              } else
                this->spostfix = true;
              part = URI_PART::QUERY;
              this->_query = strdup("");
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
          else if (!isspace(t) && t != '\xFF' && isascii(t)) {
            this->spostfix = false;
            token += t;
          }
        }
        if (!token.empty()) {
          if ((!foundAuthority && /*part != URI_PART::FRAGMENT &&*/ part != URI_PART::QUERY) ||
              part == URI_PART::PATH || part == URI_PART::SCHEME) {
            if (!this->_path)
              this->_path = new char *[10];
            this->_path[this->_path_length] = strdup(token.c_str());
            this->_path_length = this->_path_length + 1;
          } else if (part == URI_PART::HOST || part == URI_PART::USER) {
            this->_host = strdup(token.c_str());
          } else if (part == URI_PART::PORT) {
            this->_port = stoi(token);
          } else if (part == URI_PART::QUERY) {
            free((void *) this->_query);
            this->_query = strdup(token.c_str());
          } // else if (part == URI_PART::FRAGMENT) {
          // this->_fragment = strdup(token.c_str());
          // }
        }
        FOS_SAFE_FREE(dups);
      }
    }
    bool operator<(const fURI &other) const { return this->toString() < other.toString(); }
    bool operator!=(const fURI &other) const { return !this->equals(other); }
    bool operator==(const fURI &other) const {
      return this->toString() == other.toString();
    } // TODO: do field-wise comparisons
    bool equals(const fURI &other) const { return this->toString() == other.toString(); }

    string toString() const {
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
        if (strlen(this->_path[i]) > 0)
          uri.append(this->_path[i]);
        if (i < (this->_path_length - 1) || this->spostfix)
          uri.append("/");
      }
      if (this->_query)
        uri.append("?").append(this->_query);
      // if (this->_fragment)
      //   uri.append("#").append(this->_fragment);
      StringHelper::trim(uri);
      return uri;
    }
  };

  class ID final : public fURI {
  public:
    ID(const fURI &id) : ID(id.toString()) {}
    ID(const string &furiString) : ID(furiString.c_str()) {}
    ID(const char *furiCharacters) : fURI(furiCharacters) {
      try {
        if (strchr(furiCharacters, '#')) {
          throw fError("%s\n", "IDs can not contain pattern symbols: #");
        } else if (strchr(furiCharacters, '+')) {
          throw fError("%s\n", "IDs can not contain pattern symbols: +");
        }
      } catch (const fError &) {
        delete this;
        throw;
      }
    }
    // const bool isPattern() const override { return false; }
  };


  class Pattern : public fURI {
  public:
    Pattern(const fURI &uri) : Pattern(uri.toString()) {}

    Pattern(const string &uriString) : fURI(uriString){};

    Pattern(const char *uriChars) : fURI(uriChars){};

    /* bool matches(const fURI &pppattern) const override {
       string thisStr = this->toString();
       if (thisStr.find('+') == string::npos && thisStr.find('#') == string::npos)
         return fURI::matches(pppattern);
       ////////////////////////////////////////
       string patternStr = pppattern.toString();
       if (pppattern.toString() == "#")
         return true;
       if (strcmp(pppattern.scheme(), "#") == 0)
         return true;
       if (strcmp(pppattern.scheme(), "+") != 0 && strcmp(this->scheme(), pppattern.scheme()) != 0)
         return false;
       if (strcmp(pppattern.host(), "#") == 0)
         return true;
       if (strcmp(pppattern.host(), "+") != 0 && strcmp(this->host(), pppattern.host()) != 0)
         return false;
       if (strcmp(pppattern.user(), "#") == 0)
         return true;
       if (strcmp(pppattern.user(), "+") != 0 && strcmp(this->user(), pppattern.user()) != 0)
         return false;
       if (strcmp(pppattern.password(), "#") == 0)
         return true;
       if (strcmp(pppattern.password(), "+") != 0 && strcmp(this->password(), pppattern.password()) != 0)
         return false;
       for (size_t i = 0; i < pppattern.path_length(); i++) {
         if (this->_path_length <= i)
           return false;
         if (strcmp(pppattern.path(i), "#") == 0)
           return true;
         if (strcmp(pppattern.path(i), "+") != 0 && strcmp(this->path(i), pppattern.path(i)) != 0)
           return false;
       }
       return this->_path_length == pppattern.path_length();
     }*/

    // bool matches(const fURI &pattern) const override { return pattern.matches(*this); }
  };

  using fURI_p = ptr<fURI>;
  using ID_p = ptr<ID>;
  using Pattern_p = ptr<Pattern>;
  using SourceID = ID;
  using TargetID = ID;

  class BaseIDed {
  public:
    virtual ID_p id() const { return nullptr; }
    virtual bool equals(const BaseIDed &) const { return false; }
  };

  class IDed : public BaseIDed {
  public:
    explicit IDed(const fURI_p &uri) : _id(share(ID(uri->toString()))) {}
    explicit IDed(const ID_p &id) : _id(id) {}

    ID_p id() const override { return this->_id; }

    // const String toString() const { return this->id().toString(); }

    bool equals(const BaseIDed &other) const override { return this->_id->equals(*other.id()); }


  protected:
    ptr<ID> _id;
  };

  //////////////////////////////////////////////
  ///////////////// TYPED FURI /////////////////
  //////////////////////////////////////////////
  class BaseTyped {
  public:
    virtual Pattern_p type() const = 0;
    virtual bool equals(const BaseTyped &) const = 0;
  };

  class Typed : public BaseTyped {
  protected:
    Pattern_p _type;

  public:
    explicit Typed(const fURI_p &uri) : _type(share(Pattern(uri->toString()))) {}
    explicit Typed(const Pattern_p &type) : _type(type) {}
    Pattern_p type() const override { return this->_type; }
    bool equals(const BaseTyped &other) const override { return this->_type->equals(*other.type()); }
  };

  struct furi_comp : public std::less<fURI_p> {
    auto operator()(const fURI_p &a, const fURI_p &b) const { return a->toString() < b->toString(); }
  };

  [[maybe_unused]] static ID_p id_p(const char *idChars) { return share(ID(idChars)); }
  [[maybe_unused]] static ID_p id_p(const ID &id) { return share(id); }
  [[maybe_unused]] static ID_p id_p(const fURI &id) { return share(ID(id)); }
  [[maybe_unused]] static Pattern_p p_p(const char *patternChars) { return share(Pattern(patternChars)); }
  [[maybe_unused]] static Pattern_p p_p(const Pattern &pattern) { return share(pattern); }
  [[maybe_unused]] static Pattern_p p_p(const fURI &pattern) { return share(Pattern(pattern)); }
} // namespace fhatos

#endif
