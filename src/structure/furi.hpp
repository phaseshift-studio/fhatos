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

#ifndef fhatos_furi_hpp
#define fhatos_furi_hpp

#include <fhatos.hpp>
#include <string.h>
// #include <memory>
// #include <utility>

#define FOS_MAX_FURI_SEGMENTS 20

namespace private_fhatos {
  static int split(const char *text, const char *deliminator, char **&result, const uint8_t offset = 0) {
    char *copy;
    const char *freeable_copy = copy = strdup(text);
    char *token;
    int i = offset;

    while ((token = strsep(&copy, deliminator)) != nullptr) {
      if (strlen(token) > 0) {
        result[i] = strdup(token);
        i++;
      }
    }
    delete token;
    delete freeable_copy;
    return i;
  }

  static bool match(const char *id_cstr, const char *pattern_cstr) {
    if (strstr(pattern_cstr, "#") == nullptr && strstr(pattern_cstr, "+") == nullptr)
      return strcmp(id_cstr, pattern_cstr) == 0;
    if (strlen(id_cstr) == 0 && strcmp(pattern_cstr, "#") == 0)
      return true;
    char **idParts = new char *[FOS_MAX_FURI_SEGMENTS];
    char **patternParts = new char *[FOS_MAX_FURI_SEGMENTS];
    int idLength = split(id_cstr, "/", idParts);
    if (id_cstr[strlen(id_cstr) - 1] == '/')
      idLength++;
    const int patternLength = split(pattern_cstr, "/", patternParts);
    // LOG(DEBUG, "Matching: %s <=> %s\n", id, pattern);
    const bool result = [idParts, patternParts, idLength, patternLength]() {
      for (int i = 0; i < idLength; i++) {
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
    for (uint8_t i = 0; i < idLength; i++) {
      delete[] idParts[i];
    }
    delete[] idParts;
    for (uint8_t i = 0; i < patternLength; i++) {
      delete[] patternParts[i];
    }
    delete[] patternParts;
    return result;
  }
} // namespace private_fhatos

///////////////////////////
///// fURI/ID/Pattern /////
///////////////////////////

// furi://coil@127.0.0.1/devices/device/property

namespace fhatos {
  class fURI {
  protected:
    char **_segments;
    uint8_t _length;

  public:
    fURI() {
      _length = 0;
      _segments = nullptr;
    }

    fURI(const fURI &furi) {
      this->_segments = new char *[furi._length];
      for (int i = 0; i < furi._length; i++) {
        this->_segments[i] = strdup(furi._segments[i]);
      }
      this->_length = furi._length;
    };

    // fURI(const IDed &ided) : fURI(ided._id) {}

    fURI(const string &furiString) : fURI(furiString.c_str()) {}

    fURI(const char *furiCharacters) {
      if ((strlen(furiCharacters) == 0) || (strlen(furiCharacters) == 1 && furiCharacters[0] == '/')) {
        this->_length = 0;
        this->_segments = new char *[0];
      } else {
        uint8_t counter = 0;
        uint8_t length = strlen(furiCharacters);
        if (furiCharacters[0] == '/')
          counter++;
        for (uint8_t i = 0; i < length; i++) {
          if ((furiCharacters[i]) == '/' && ((i == length - 1) || furiCharacters[i + 1] != '/'))
            counter++;
        }
        const char *scheme = strstr(furiCharacters, "://");
        this->_segments = new char *[counter + 1];
        if (counter == 0) {
          this->_segments[0] = strdup(furiCharacters);
          this->_length = 1;
        } else {
          if (furiCharacters[0] == '/') {
            this->_segments[0] = strdup("");
            this->_length = private_fhatos::split(furiCharacters, "/", this->_segments, 1);
          } else if (scheme) {
            this->_length = private_fhatos::split(scheme + 3, "/", this->_segments);
            string schemeString = string(furiCharacters);
            this->_segments[0] = strdup(
                (schemeString.substr(0, schemeString.find_first_of("://")) + "://" + string(_segments[0])).c_str());
          } else {
            this->_length = private_fhatos::split(furiCharacters, "/", this->_segments);
          }
        }
      }
    };

    const bool operator==(const fURI &other) const { return this->equals(other); }

    const bool operator!=(const fURI &other) const { return !this->equals(other); }

    fURI(const fURI &parent, const char *extension) :
        fURI(parent.toString() + "/" + extension){
            // this->__segments[this->__length++] = strdup(extension);
        };

    virtual ~fURI() {
      if (this->_length > 0) {
        for (uint8_t i = 0; i < this->_length; i++) {
          delete this->_segments[i];
        }
      }
      delete this->_segments;
    }

    // const bool operator==(const fURI other) const { return this->equals(other); }

    const fURI extend(const char *segments) const {
      return ((string(segments).length() == 0) || (string(segments).length() == 1 && segments[0] == '/'))
                 ? fURI(*this)
                 : fURI(string(this->toString()).append("/").append(segments));
    }

    const fURI retract(const bool fromRight=true) const {
      if (this->empty())
        return *this;
      string path;
      if(fromRight) {
        for (uint8_t i = 0; i < this->_length - 1; i++) {
          if (i > 0)
            path = path + "/";
          path = path + this->_segments[i];
        }
      } else {
        for (uint8_t i = 1; i < this->_length; i++) {
          if (i > 1)
            path = path + "/";
          path = path + this->_segments[i];
        }
      }
      return fURI(path);
    }

    uint8_t length() const { return this->_length; }

    bool empty() const { return 0 == this->_length; }

    virtual bool matches(const fURI &pattern) const {
      return private_fhatos::match(this->toString().c_str(), pattern.toString().c_str());
    }

    // bool parentOf(const fURI &furi) const;
    // bool childOf(const fURI &furi) const { return furi.parentOf(*this); }
    const string segment(const uint8_t index) const { return string(this->_segments[index]); }

    const string lastSegment() const { return string(this->_segments[this->_length - 1]); }

    const string path() const {
      string temp;
      if (this->_length > 1) {
        for (uint8_t i = 1; i < this->_length; i++) {
          if (i > 1)
            temp = temp + "/";
          temp = temp + this->_segments[i];
        }
      }
      return temp;
    }

    const fURI path(const string &path) const { return fURI(this->authority()).extend(path.c_str()); }

    const fURI user(const char *user) const {
      return this->authority(
          this->host().empty() ? user : (strlen(user) == 0 ? this->host() : string(user) + "@" + this->host()));
    }

    const Option<string> user() const {
      Option<Pair<string, string>> temp = this->user_password();
      return temp.has_value() ? temp->first : Option<string>();
    }

    const Option<Pair<string, string>> user_password() const {
      if (const int i = this->scheme("").authority().find('@'); i < 0)
        return {};
      else {
        string userpass = this->scheme("").authority().substr(0, i);
        const int j = userpass.find(':');
        if (j < 0)
          return {{userpass, string()}};
        else
          return {{userpass.substr(0, j), userpass.substr(j + 1)}};
      }
    }

    const string host() const {
      string temp = this->scheme("").authority();
      if (temp.empty() || temp.at(temp.length() - 1) == '@')
        return string();
      int i = temp.find('@');
      return (i < 0) ? temp : temp.substr(i + 1);
    }

    const fURI host(const char *host) const {
      string temp;
      const Option<Pair<string, string>> x = this->user_password();
      if (x.has_value()) {
        temp = temp + x.value().first;
        if (!x.value().second.empty())
          temp = temp + ":" + x.value().second;
        temp = temp + "@";
      }
      temp = temp + host;
      return this->authority(temp);
    }

    const string scheme() const {
      const int colonIndex = string(this->_segments[0]).find(':');
      if (std::string::npos == colonIndex) {
        return string("");
      } else {
        return  string(this->_segments[0]).substr(0, colonIndex);
      }
    }

    const fURI scheme(const string &scheme) const {
      const int index = string(this->_segments[0]).find_first_of(':');
      const int colonSlashSlashIndex = string(this->_segments[0]).find("://");
      if (std::string::npos == index) {
        if (scheme.empty()) {
          return *this;
        } else {
          return fURI(scheme + (this->host().empty() || this->user_password().has_value() ? "://" : ":") + this->toString());
        }
      } else if (scheme.empty()) {
        fURI temp = fURI(*this);
        temp._segments[0] = strdup(string(temp._segments[0]).substr(index + (std::string::npos == colonSlashSlashIndex ? 1 : 3)).c_str());
        return temp;
      } else {
        fURI temp = fURI(*this);
        temp._segments[0] = strdup((scheme + (this->host().empty() || this->user_password().has_value() ? "://" : ":") + string(temp._segments[0]).substr(index+1)).c_str());
        return temp;
      }
    }

    const string authority() const { return this->_length > 0 ? string(this->_segments[0]) : string(); }

    const fURI authority(const string &authority) const {
      if (fURI temp = fURI(*this); temp._length == 0)
        return fURI(authority);
      else {
        delete temp._segments[0];
        temp._segments[0] = strdup(authority.c_str());
        return temp;
      }
    }

    bool hasQuery(const string &check = "?") const { return strstr(this->_segments[this->_length - 1], check.c_str()); }

    const string query() const {
      for (uint8_t i = 0; i < this->_length; i++) {
        char *ptr = strchr(_segments[i], '?');
        if (ptr) {
          return string(ptr);
        }
      }
      return string();
    }

    const fURI query(const char *query) const {
      const string temp = string(this->toString().c_str());
      const int index = temp.find('?');
      const fURI furi = index < 0 ? *this : fURI(temp.substr(0, index));
      return fURI(string(furi.toString()).append(query));
    }

    virtual bool colocated(const fURI &other) const { return this->host() == other.host(); }

    // const char *c_str() const { return this->toString().c_str(); }
    const string toString() const {
      string temp;
      for (uint8_t i = 0; i < this->_length; i++) {
        temp.append(this->_segments[i]);
        if (i != this->_length - 1)
          temp.append("/");
      }
      return temp;
    }

    bool equals(const fURI &other) const {
      if (this->_length != other._length)
        return false;
      for (uint8_t i = 0; i < this->_length; i++) {
        if (strcmp(this->_segments[i], other._segments[i]) != 0)
          return false;
      }
      return true;
    }

    bool subfuri(const fURI furi) const {
      if (this->_length <= furi._length)
        return false;
      for (uint8_t i = 0; i < furi._length; i++) {
        if (strcmp(this->_segments[i], furi._segments[i]) != 0)
          return false;
      }
      return true;
    }

    const fURI operator/(const char *cstr) const { return this->extend(cstr); }

    bool operator<(const fURI &furi) const { return this->toString() < furi.toString(); }

    bool isLocal(const fURI &other) const { return this->host() == other.host(); }

    const fURI resolve(const fURI &base) const {
      if (this->authority().empty())
        return base.extend(this->toString().c_str());
      else if (this->host().empty() && !base.host().empty())
        return fURI(this->authority() + base.host()).extend(this->path().c_str());
      else
        return *this;
    }
  };

  class ID final : public fURI {
  public:
    ID(const fURI &id) : ID(id.toString()) {}

    ID(const string &furiString) : fURI(furiString) {}

    ID(const char *furiCharacters) : fURI(furiCharacters) {
      try {
        if (strchr(furiCharacters, '#')) {
          throw fError("IDs can not contain pattern symbols: #");
        } else if (strchr(furiCharacters, '+')) {
          throw fError("IDs can not contain pattern symbols: +");
        }
      } catch (const fError &e) {
        if (this->_length > 0) {
          for (uint8_t i = 0; i < this->_length; i++) {
            delete this->_segments[i];
          }
          delete _segments;
        }
        this->_length = 0;
        throw e;
      }
    }
  };

  using SourceID = ID;
  using TargetID = ID;

  class Pattern : public fURI {
  public:
    Pattern(const fURI &fURI) : Pattern(fURI.toString()) {}

    // Pattern(const ID &id) : Pattern(id.toString()) {
    // }

    Pattern(const string &furiString) : fURI(furiString){};

    Pattern(const char *furiCharacters) : fURI(furiCharacters){};

    bool colocated(const fURI &furi) const override {
      return furi.authority() == "#" || furi.authority().find("+") > -1 || fURI::colocated(furi);
    }

    bool matches(const fURI &pattern) const override {
      return private_fhatos::match(pattern.toString().c_str(), this->toString().c_str());
    }
  };

  class BaseIDed {
  public:
    virtual const ID id() const { return nullptr; }
    virtual bool equals(const BaseIDed &other) const { return false; }
  };

  class IDed : public BaseIDed {
  public:
    virtual ~IDed() = default;

    explicit IDed(const ID &id) : _id(id) {}

    virtual const ID id() const override { return this->_id; }

    // const String toString() const { return this->id().toString(); }

    bool equals(const BaseIDed &other) const override { return this->_id.equals(other.id()); }

  protected:
    ID _id;
  };
} // namespace fhatos

#undef FOS_MAX_FURI_SEGMENT_LENGTH

#endif
