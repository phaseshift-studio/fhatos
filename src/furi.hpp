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
#ifndef fhatos_furi_hpp
#define fhatos_furi_hpp

#define FOS_MAX_PATH_SEGMENTS 15

#include "fhatos.hpp"
//
#include <sstream>
#include "util/string_helper.hpp"

namespace fhatos {
  enum class URI_PART {
    SCHEME, USER, PASSWORD, HOST, PORT, PATH, FRAGMENT, QUERY
  };

  const static Enums<URI_PART> URI_PARTS = Enums<URI_PART>{
    {URI_PART::SCHEME, "scheme"},
    {URI_PART::USER, "user"},
    {URI_PART::PASSWORD, "password"},
    {URI_PART::HOST, "host"},
    {URI_PART::PORT, "port"},
    {URI_PART::PATH, "path"},
    {URI_PART::FRAGMENT, "fragment"},
    {URI_PART::QUERY, "query"},
  };

  class fURI {
  protected:
    const char *scheme_ = nullptr;
    const char *user_ = nullptr;
    const char *password_ = nullptr;
    const char *host_ = nullptr;
    uint16_t port_ = 0;
    char **path_ = nullptr;
    bool sprefix_ = false;
    bool spostfix_ = false;
    uint8_t path_length_ = 0;
    const char *query_ = nullptr;
    const char *fragment_ = nullptr;

  public:
    [[nodiscard]] const char *scheme() const { return this->scheme_ ? this->scheme_ : ""; }

    fURI scheme(const char *scheme) {
      auto new_uri = fURI(*this);
      free((void *) new_uri.scheme_);
      new_uri.scheme_ = 0 == strlen(scheme) ? nullptr : strdup(scheme);
      return new_uri;
    }

    /// USER
    [[nodiscard]] const char *user() const { return this->user_ ? this->user_ : ""; }

    fURI user(const char *user) const {
      auto new_uri = fURI(*this);
      free((void *) new_uri.user_);
      new_uri.user_ = 0 == strlen(user) ? nullptr : strdup(user);
      return new_uri;
    }

    /// PASSWORD
    [[nodiscard]] const char *password() const { return this->password_ ? this->password_ : ""; }

    fURI password(const char *password) const {
      auto new_uri = fURI(*this);
      free((void *) new_uri.password_);
      new_uri.password_ = 0 == strlen(password) ? nullptr : strdup(password);
      return new_uri;
    }

    /// HOST
    [[nodiscard]] const char *host() const { return this->host_ ? this->host_ : ""; }

    fURI host(const char *host) const {
      auto new_uri = fURI(*this);
      free((void *) new_uri.host_);
      new_uri.host_ = (0 == strlen(host) ? nullptr : strdup(host));
      return new_uri;
    }

    /// PORT
    [[nodiscard]] uint16_t port() const { return this->port_; }

    [[nodiscard]] fURI port(const uint16_t port) const {
      auto new_uri = fURI(*this);
      new_uri.port_ = port;
      return new_uri;
    }

    /// AUTHORITY
    [[nodiscard]] string authority() const {
      string authority;
      if (this->user_) {
        authority += this->user_;
      }
      if (this->password_) {
        authority += ':';
        authority += this->password_;
      }
      if (this->host_) {
        if (this->user_ || this->password_)
          authority += '@';
        authority += this->host_;
      }
      if (this->port_) {
        authority += ':';
        authority += to_string(this->port_);
      }
      return authority;
    }

    [[nodiscard]] fURI authority(const char *authority) const {
      const string authority_string = (strlen(authority) > 1 && authority[0] == '/' && authority[1] == '/')
                                        ? authority
                                        : string("//") + authority;
      const auto furi = fURI(nullptr != this->scheme_
                               ? string(this->scheme_) + ":" + authority_string
                               : authority_string);
      return this->path_length_ > 0 ? furi.path(this->path()) : furi;
    }

    /// PATH
    [[nodiscard]] string path(const uint8_t start, const uint8_t end) const {
      if (start > this->path_length_ || start > end)
        return "";
      string path_str;
      if (this->path_) {
        if (this->sprefix_ && start == 0)
          path_str += '/';
        for (uint8_t i = start; (i < this->path_length_) && (i < end); i++) {
          path_str = path_str.append(this->path_[i]);
          if (i != end - 1)
            path_str += '/';
        }
        if (this->spostfix_ && end >= this->path_length_)
          path_str += '/';
      }
      return path_str;
    }

    [[nodiscard]] string path() const { return this->path(0, this->path_length_); }

    [[nodiscard]] const char *path(const uint8_t segment) const {
      return (this->path_ && this->path_length_ > segment) ? this->path_[segment] : "";
    }

    [[nodiscard]] fURI path(const string &path) const {
      auto new_uri = fURI(*this);
      StringHelper::trim(path);
      for (uint8_t i = 0; i < this->path_length_; i++) {
        free(new_uri.path_[i]);
      }
      delete[] new_uri.path_;
      new_uri.path_length_ = 0;
      new_uri.path_ = new char *[FOS_MAX_PATH_SEGMENTS];
      char *dup = strdup(path.c_str());
      auto ss = std::stringstream(dup);
      string segment;
      uint8_t i = 0;
      char c;
      while (ss.get(c)) {
        if (c == '\0' || isspace(c) || !isascii(c))
          break;
        if (c == '/') {
          if (segment.empty() && 0 == i) {
            new_uri.sprefix_ = true;
          } else {
            new_uri.path_[i] = strdup(segment.c_str());
            i++;
          }
          segment.clear();
        } else {
          segment += c;
        }
      }
      StringHelper::trim(segment);
      if (segment.empty()) {
        new_uri.path_length_ = i;
        new_uri.spostfix_ = true;
      } else {
        new_uri.path_[i] = strdup(segment.c_str());
        new_uri.path_length_ = i + 1;
        new_uri.spostfix_ = false;
      }
      if (path[path.length() - 1] == '/')
        new_uri.spostfix_ = true;
      free(dup);
      if (new_uri.host_ || new_uri.scheme_)
        new_uri.sprefix_ = true;
      return new_uri;
    }

    [[nodiscard]] const char *name() const {
      if (0 == this->path_length_)
        return "";
      for (int i = this->path_length_ - 1; i >= 0; i--) {
        if (strlen(this->path_[i]) > 0)
          return this->path_[i];
      }
      return "";
    }

    [[nodiscard]] bool empty() const {
      return this->path_length_ == 0 && !this->host_ && !this->scheme_ && !this->user_ && !this->password_ &&
             !this->fragment_ && !this->query_;
    }

    [[nodiscard]] uint8_t path_length() const { return this->path_length_; }

    /// QUERY
    [[nodiscard]] const char *query() const { return this->query_ ? this->query_ : ""; }

    [[nodiscard]] bool has_query() const { return this->query_ != nullptr && 0 != strlen(this->query_); }

    fURI query(const char *query) const {
      auto new_uri = fURI(*this);
      FOS_SAFE_FREE(new_uri.query_);
      new_uri.query_ = nullptr == query || 0 == strlen(query) ? nullptr : strdup(query);
      return new_uri;
    }

    /// FRAGMENT
    [[nodiscard]] const char *fragment() const { return this->fragment_ ? this->fragment_ : ""; }

    fURI fragment(const char *fragment) const {
      auto new_uri = fURI(*this);
      new_uri.fragment_ = 0 == strlen(fragment) ? nullptr : fragment;
      return new_uri;
    }

    ////////////////////////////////////////////////////////////////

    fURI extend(const fURI &furi_path) const {
      return this->extend(furi_path.path().c_str());
    }

    fURI extend(const char *extension) const {
      if (strlen(extension) == 0) {
        auto new_uri = fURI(*this);
        new_uri.spostfix_ = true;
        return new_uri;
      }
      auto new_path = string(this->path());
      if (!this->spostfix_ && extension[0] != '/')
        new_path += '/';
      if (new_path.empty())
        return this->path(extension);
      new_path += extension;
      return this->path(new_path);
    }

    [[nodiscard]] fURI retract() const {
      auto new_uri = fURI(*this);
      for (uint8_t i = 0; i < new_uri.path_length_; i++) {
        free(new_uri.path_[i]);
      }
      delete[] new_uri.path_;
      new_uri.path_length_ = this->path_length_ > 1 ? this->path_length_ - 1 : 0;
      new_uri.path_ = new char *[new_uri.path_length_];
      for (uint8_t i = 0; i < new_uri.path_length_; i++) {
        new_uri.path_[i] = strdup(this->path_[i]);
      }
      return new_uri;
    }

    [[nodiscard]] fURI retract_pattern() const {
      const char *end = this->path_[this->path_length_ - 1];
      if (end[0] == '+' || end[0] == '#')
        return this->retract().retract_pattern();
      return *this;
    }

    [[nodiscard]] bool is_subfuri_of(const fURI &other) const {
      const string this_string = this->toString();
      const string other_string = other.toString();
      return other_string.length() >= this_string.length() &&
             other_string.substr(0, this_string.length()) == this_string;
    }

    fURI dissolve() const {
      const string temp = this->path();
      return temp.empty() ? fURI("") : fURI(temp[0] == '/' ? temp.substr(1) : temp);
    }

    bool is_relative() const {
      const char first = this->toString()[0];
      return first == '.' || first == ':';
    }

    bool is_scheme_path() const {
      return this->scheme_ && this->path_length_ > 0 && !this->host_ && !this->user_ && !this->password_;
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
      if (other.is_scheme_path()) {
        if (!this->scheme_)
          return other.sprefix_ ? other : this->extend(other.toString().c_str());
        else if (strcmp(this->scheme_, other.scheme_) != 0) {
          return this->extend(other.toString().c_str());
        }
      }
      ///////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////
      if (other.path_length_ == 0)
        return *this;
      const bool path_end_slash = this->path()[this->path().length() - 1] == '/' || this->spostfix_;
      const bool path_start_slash = this->path()[0] == '/' || this->sprefix_;
      if (other.path().find('.') == string::npos) {
        const auto other_path_chars =
            std::unique_ptr<char, void (*)(void *)>(strdup(other.path().c_str()), free);
        const bool other_start_slash = other_path_chars.get()[0] == '/';
        if (path_end_slash || this->path_length_ == 0)
          return other_start_slash ? this->path(other_path_chars.get()) : this->extend(other_path_chars.get());
        if (other_start_slash)
          return this->path(other_path_chars.get());
        if (this->path_length_ == 1)
          return this->path((path_start_slash) ? (string("/") + other_path_chars.get()) : other_path_chars.get());
        return this->retract().extend(other_path_chars.get());
      }
      fURI *temp = path_end_slash || this->path_length_ == 0 ? new fURI(*this) : new fURI(this->retract());
      for (uint8_t i = 0; i < other.path_length_; i++) {
        if (strcmp(other.path(i), "..") == 0) {
          const fURI *temp2 = new fURI(*temp);
          delete temp;
          temp = temp2->path_length() > 0 ? new fURI(temp2->retract()) : new fURI(*temp2);
          delete temp2;
        } else if (strcmp(other.path(i), ".") != 0) {
          const fURI *temp2 = new fURI(*temp);
          delete temp;
          temp = new fURI(temp2->extend(other.path(i)));
          delete temp2;
        }
        if (i == other.path_length_ - 1)
          temp->spostfix_ = other.spostfix_;
      }
      fURI ret = fURI(*temp);
      delete temp;
      return ret;
    }

    [[nodiscard]] virtual bool is_pattern() const {
      string temp = this->toString();
      return temp.find('#') != string::npos || temp.find('+') != string::npos;
    }

    [[nodiscard]] virtual bool matches(const fURI &pattern) const {
      if (this->equals(pattern))
        return true;
      const string pattern_str = pattern.toString();
      if (pattern.toString() == "#")
        return true;
      if (pattern_str.find('+') == string::npos && pattern_str.find('#') == string::npos)
        return this->toString() == pattern_str;
      if (strcmp(pattern.scheme(), "#") == 0)
        return true;
      if ((strlen(this->scheme()) == 0 && strlen(pattern.scheme()) != 0) ||
          (strcmp(pattern.scheme(), "+") != 0 && strcmp(this->scheme(), pattern.scheme()) != 0))
        return false;
      if (strcmp(pattern.host(), "#") == 0)
        return true;
      if ((strlen(this->host()) == 0 && strlen(pattern.host()) != 0) ||
          (strcmp(pattern.host(), "+") != 0 &&
           strcmp(this->host(), pattern.host()) !=
           0)) // TODO: this should be just to authority as user:pass can't be wildcard matched ??
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
        if (strcmp(pattern.path(i), "#") == 0)
          return true;
        if (0 == i && (this->sprefix_ != pattern.sprefix_))
          return false;
        if (strcmp(pattern.path(i), "+") == 0) {
          if(strcmp(this->path(i), "#") == 0)
            return false;
          if(this->path_length_ <= i && this->spostfix_)
            return true;
        }
        if (this->path_length_ <= i)
          return false;
        if ((strlen(this->path(i)) == 0 && strlen(pattern.path(i)) != 0) ||
            (strcmp(pattern.path(i), "+") != 0 && strcmp(this->path(i), pattern.path(i)) != 0))
          return false;
      }
      return this->path_length_ == pattern.path_length();
    }

    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////
    virtual ~fURI() {
      free((void *) this->scheme_);
      free((void *) this->host_);
      free((void *) this->user_);
      free((void *) this->password_);
      free((void *) this->query_);
      free((void *) this->fragment_);
      for (size_t i = 0; i < this->path_length_; i++) {
        free(path_[i]);
      }
      delete[] path_;
    };

    fURI(const fURI &other) {
      this->scheme_ = other.scheme_ ? strdup(other.scheme_) : nullptr;
      this->user_ = other.user_ ? strdup(other.user_) : nullptr;
      this->password_ = other.password_ ? strdup(other.password_) : nullptr;
      this->host_ = other.host_ ? strdup(other.host_) : nullptr;
      this->port_ = other.port_;
      this->sprefix_ = other.sprefix_;
      this->spostfix_ = other.spostfix_;
      this->path_length_ = other.path_length_;
      this->query_ = other.query_ && strcmp("", other.query_) != 0 ? strdup(other.query_) : nullptr;
      this->fragment_ = other.fragment_ ? strdup(other.fragment_) : nullptr;
      this->path_ = new char *[other.path_length_]();
      for (uint8_t i = 0; i < other.path_length_; i++) {
        this->path_[i] = strdup(other.path_[i]);
      }
    }

    fURI(const string &uriString) : fURI(uriString.c_str()) {
    }

    fURI(const char *uriChars) {
      if (strlen(uriChars) == 0)
        return;
      const char *dups = strdup(uriChars);
      for (size_t i = 0; i < strlen(dups); i++) {
        if (dups[i] == '#' && i != strlen(dups) - 1) {
          const string temp = string(dups);
          free((void *) dups);
          throw fError("Recurssive !b#!! wildcard must be the last character: %s\n", temp.c_str());
        }
      }
      try {
        std::stringstream ss = std::stringstream(dups);
        string token;
        URI_PART part = URI_PART::SCHEME;
        bool hasUserInfo = strchr(dups, '@') != nullptr;
        bool foundAuthority = false;
        while (!ss.eof()) {
          char c = (char) ss.get();
          if (!isascii(c) || isspace(c) || c < 32 || c > 126)
            continue;
          if (!foundAuthority && c == '/' && ss.peek() == '/') {
            foundAuthority = true;
            if (part == URI_PART::SCHEME || part == URI_PART::USER) {
              part = URI_PART::USER;
              ss.get();
            }
          } else if (c == ':') {
            if (part == URI_PART::SCHEME) {
              this->scheme_ = strdup(token.c_str());
              part = URI_PART::USER;
              token.clear();
            } else if (part == URI_PART::USER) {
              if (hasUserInfo) {
                this->user_ = strdup(token.c_str());
                part = URI_PART::PASSWORD;
              } else {
                this->host_ = strdup(token.c_str());
                part = URI_PART::PORT;
              }
              token.clear();
            } else if (part == URI_PART::HOST) {
              this->host_ = strdup(token.c_str());
              part = URI_PART::PORT;
              token.clear();
            } else {
              token += c;
            }
          } else if (c == '@') {
            if (part == URI_PART::USER || part == URI_PART::PASSWORD) {
              if (this->user_) {
                this->password_ = strdup(token.c_str());
              } else {
                this->user_ = strdup(token.c_str());
              }
              part = URI_PART::HOST;
              token.clear();
            } else {
              token += c;
            }
          } else if (c == '/') {
            if (part == URI_PART::PORT) {
              this->port_ = stoi(token);
              part = URI_PART::PATH;
              this->sprefix_ = true;
              token.clear();
            } else if (part == URI_PART::SCHEME || part == URI_PART::HOST || part == URI_PART::USER ||
                       part == URI_PART::PASSWORD) {
              if (foundAuthority) {
                this->host_ = strdup(token.c_str());
                part = URI_PART::PATH;
                this->sprefix_ = true;
              } else {
                if (!token.empty()) {
                  // TODO: what about empty components?
                  if (!this->path_)
                    this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
                  this->path_[this->path_length_] = strdup(token.c_str());
                  this->path_length_ = this->path_length_ + 1;
                  check_path_length(uriChars);
                } else {
                  this->sprefix_ = true;
                }
                part = URI_PART::PATH;
              }
              token.clear();
            } else if (part == URI_PART::PATH) {
              if (!this->path_)
                this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
              this->path_[this->path_length_] = strdup(token.c_str());
              this->path_length_ = this->path_length_ + 1;
              check_path_length(uriChars);
              this->spostfix_ = true;
              token.clear();
            } else {
              token += c;
            }
          } else if (c == '?') {
            if (part == URI_PART::PATH || part == URI_PART::SCHEME) {
              if (!token.empty()) {
                if (!this->path_)
                  this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
                this->path_[this->path_length_] = strdup(token.c_str());
                this->path_length_ = this->path_length_ + 1;
                check_path_length(uriChars);
              } else
                this->spostfix_ = true;
              part = URI_PART::QUERY;
              this->query_ = strdup("");
              token.clear();
            } else if (part == URI_PART::HOST || part == URI_PART::USER) {
              host_ = strdup(token.c_str());
              part = URI_PART::QUERY;
              token.clear();
            } else {
              token += c;
            }
          } /*else if (c == '#') {
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
    token += c;
  }
}*/
          else if (!isspace(c) && isascii(c)) {
            this->spostfix_ = false;
            token += c;
          }
        }
        StringHelper::trim(token);
        if (!token.empty()) {
          if ((!foundAuthority && /*part != URI_PART::FRAGMENT &&*/ part != URI_PART::QUERY) ||
              part == URI_PART::PATH || part == URI_PART::SCHEME) {
            if (!this->path_)
              this->path_ = new char *[FOS_MAX_PATH_SEGMENTS];
            this->path_[this->path_length_] = strdup(token.c_str());
            this->path_length_ = this->path_length_ + 1;
            check_path_length(uriChars);
          } else if (part == URI_PART::HOST || part == URI_PART::USER) {
            this->host_ = strdup(token.c_str());
          } else if (part == URI_PART::PORT) {
            this->port_ = stoi(token);
          } else if (part == URI_PART::QUERY) {
            free((void *) this->query_);
            this->query_ = strdup(token.c_str());
          } // else if (part == URI_PART::FRAGMENT) {
          // this->_fragment = strdup(token.c_str());
          // }
        }
      } catch (const std::exception &) {
        FOS_SAFE_FREE(dups);
        throw;
      }
      FOS_SAFE_FREE(dups);
    }

    bool operator<(const fURI &other) const { return this->toString() < other.toString(); }

    bool operator!=(const fURI &other) const { return !this->equals(other); }

    bool operator==(const fURI &other) const {
      return this->toString() == other.toString();
    } // TODO: do field-wise comparisons
    bool equals(const fURI &other) const { return this->toString() == other.toString(); }

    [[nodiscard]] string toString() const {
      string uri;
      if (this->scheme_)
        uri.append(this->scheme_).append(":");
      if (this->host_ || this->user_) {
        uri.append("//");
        if (this->user_) {
          uri.append(this->user_);
          if (this->password_)
            uri.append(":").append(this->password_);
          uri.append("@");
        }
        if (this->host_) {
          uri.append(this->host_);
          if (this->port_ > 0)
            uri.append(":").append(std::to_string(this->port_));
        }
      }
      if (this->sprefix_)
        uri.append("/");
      for (int i = 0; i < this->path_length_; i++) {
        if (strlen(this->path_[i]) > 0)
          uri.append(this->path_[i]);
        if (i < (this->path_length_ - 1) || this->spostfix_)
          uri.append("/");
      }
      if (this->query_)
        uri.append("?").append(this->query_);
      // if (this->_fragment)
      //   uri.append("#").append(this->_fragment);
      StringHelper::trim(uri);
      return uri;
    }

  private:
    void check_path_length(const char *self) const {
      if (this->path_length_ >= FOS_MAX_PATH_SEGMENTS)
        throw fError("!ymax path length!! of !r%i!! has been reached: !b%s!!\n", FOS_MAX_PATH_SEGMENTS, self);
    }
  };

  class ID final : public fURI {
  public:
    ID(const fURI &id) : fURI(id.toString()) {
    }

    ID(const string &furi_string) : ID(furi_string.c_str()) {
    }

    ID(const char *furi_characters) : fURI(furi_characters) {
      if (strchr(furi_characters, '#')) {
        throw fError("IDs can not contain pattern symbols: !b#!!: %s\n", furi_characters);
      } else if (strchr(furi_characters, '+')) {
        throw fError("IDs can not contain pattern symbols: !b+!!: %s\n", furi_characters);
      }
    }

    // const bool isPattern() const override { return false; }
  };

  class Pattern : public fURI {
  public:
    Pattern(const fURI &uri) : fURI(uri) {
    }

    Pattern(const string &uri_string) : fURI(uri_string) {
    };

    Pattern(const char *uri_chars) : fURI(uri_chars) {
    };
  };

  using fURI_p = ptr<fURI>;
  using ID_p = ptr<ID>;
  using Pattern_p = ptr<Pattern>;
  using SourceID = ID;
  using TargetID = ID;

  class BaseIDed {
  public:
    virtual ~BaseIDed() = default;

    [[nodiscard]] virtual ID_p id() const = 0;

    [[nodiscard]] virtual bool equals(const BaseIDed &) const { return false; }
  };

  class IDed : public BaseIDed {
  protected:
    ID_p id_;

  public:
    explicit IDed(const fURI_p &uri) : id_(share(ID(uri->toString()))) {
    }

    explicit IDed(const ID_p &id) : id_(id) {
    }

    [[nodiscard]] ID_p id() const override { return this->id_; }

    // const String toString() const { return this->id().toString(); }

    [[nodiscard]] bool equals(const BaseIDed &other) const override { return this->id_->equals(*other.id()); }
  };

  //////////////////////////////////////////////
  ///////////////// TYPED FURI /////////////////
  //////////////////////////////////////////////
  class BasePatterned {
  public:
    virtual ~BasePatterned() = default;

    [[nodiscard]] virtual Pattern_p pattern() const = 0;

    [[nodiscard]] virtual bool equals(const BasePatterned &) const = 0;
  };

  class Patterned : public BasePatterned {
  protected:
    Pattern_p pattern_;

  public:
    explicit Patterned(const fURI_p &uri) : pattern_(share(Pattern(uri->toString()))) {
    }

    explicit Patterned(const Pattern_p &type) : pattern_(share(Pattern(*type))) {
    }

    [[nodiscard]] Pattern_p pattern() const override { return this->pattern_; }

    [[nodiscard]] bool equals(const BasePatterned &other) const override {
      return this->pattern_->equals(*other.pattern());
    }
  };

  struct furi_p_less : public std::less<fURI_p> {
    auto operator()(const fURI_p &a, const fURI_p &b) const { return a->toString() < b->toString(); }
  };

  [[maybe_unused]] static fURI_p furi_p(const char *id_chars) { return share(fURI(id_chars)); }

  [[maybe_unused]] static fURI_p furi_p(const fURI &furi) { return share(fURI(furi)); }

  [[maybe_unused]] static ID_p id_p(const char *id_chars) { return share(ID(id_chars)); }

  [[maybe_unused]] static ID_p id_p(const ID &id) { return share(ID(id)); }

  [[maybe_unused]] static ID_p id_p(const fURI &id) { return share(ID(id)); }

  [[maybe_unused]] static Pattern_p p_p(const char *pattern_chars) { return share(Pattern(pattern_chars)); }

  [[maybe_unused]] static Pattern_p p_p(const Pattern &pattern) { return share(Pattern(pattern)); }

  [[maybe_unused]] static Pattern_p p_p(const fURI &pattern) { return share(Pattern(pattern)); }
} // namespace fhatos

#endif
