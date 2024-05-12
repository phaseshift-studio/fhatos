#ifndef fhatstructure_hpp
#define fhatstructure_hpp

#include <fhatos.hpp>
#include <util/mutex_deque.hpp>
#include <memory>
#include <utility>

#define FOS_MAX_FURI_SEGMENTS 20

namespace private_fhatos {

static int split(const char *text, const char *deliminator, char **&result,
                 uint8_t offset = 0) {
  char *copy;
  char *freeable_copy;
  freeable_copy = copy = strdup(text);
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
  if (strstr(pattern_cstr, "#") == nullptr &&
      strstr(pattern_cstr, "+") == nullptr)
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
      if ((strcmp(patternParts[i], "+") != 0) &&
          (strcmp(patternParts[i], idParts[i]) != 0))
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

  explicit fURI(const String &furiString) : fURI(furiString.c_str()) {}

  explicit fURI(const char *furiCharacters) {
    if ((strlen(furiCharacters) == 0) ||
        (strlen(furiCharacters) == 1 && furiCharacters[0] == '/')) {
      this->_length = 0;
      this->_segments = new char *[0];
    } else {
      uint8_t counter = 0;
      uint8_t length = strlen(furiCharacters);
      if (furiCharacters[0] == '/')
        counter++;
      for (uint8_t i = 0; i < length; i++) {
        if ((furiCharacters[i]) == '/' &&
            ((i == length - 1) || furiCharacters[i + 1] != '/'))
          counter++;
      }
      this->_segments = new char *[counter + 1];
      if (counter == 0) {
        this->_segments[0] = strdup(furiCharacters);
        this->_length = 1;
      } else {
        if (furiCharacters[0] == '/') {
          this->_segments[0] = strdup("");
          this->_length =
              private_fhatos::split(furiCharacters, "/", this->_segments, 1);
        } else {
          this->_length =
              private_fhatos::split(furiCharacters, "/", this->_segments);
        }
      }
    }
  };

  fURI(const fURI &parent, const char *extension)
      : fURI(parent.toString() + "/" + extension) {
          // this->__segments[this->__length++] = strdup(extension);
        };

  explicit fURI(const StringSumHelper &shelper) : fURI(shelper.c_str()) {};

  virtual ~fURI() {
    if (this->_length > 0) {
      for (uint8_t i = 0; i < this->_length; i++) {
        delete this->_segments[i];
      }
    }
    delete this->_segments;
  }

  //const bool operator==(const fURI other) const { return this->equals(other); }

  const fURI extend(const char *segments) const {
    return ((strlen(segments) == 0) ||
            (strlen(segments) == 1 && segments[0] == '/'))
               ? fURI(*this)
               : fURI(this->toString() + "/" + segments);
  }

  const fURI retract() const {
    if (this->empty())
      return *this;
    String path;
    for (uint8_t i = 0; i < this->_length - 1; i++) {
      if (i > 0)
        path = path + "/";
      path = path + this->_segments[i];
    }
    return fURI(path);
  }

  const uint8_t length() const { return this->_length; }

  const bool empty() const { return 0 == this->_length; }

  virtual const bool matches(const fURI &pattern) const {
    return private_fhatos::match(this->toString().c_str(),
                                 pattern.toString().c_str());
  }

  // bool parentOf(const fURI &furi) const;
  // bool childOf(const fURI &furi) const { return furi.parentOf(*this); }
  const String segment(const uint8_t index) const {
    return String(this->_segments[index]);
  }

  const String lastSegment() const {
    return String(this->_segments[this->_length - 1]);
  }

  const String path() const {
    String temp;
    if (this->_length > 1) {
      for (uint8_t i = 1; i < this->_length; i++) {
        if (i > 1)
          temp = temp + "/";
        temp = temp + this->_segments[i];
      }
    }
    return temp;
  }

  const fURI path(const String path) const {
    return fURI(this->authority()).extend(path.c_str());
  }

  const Option<String> user() const {
    Option<Pair<String, String>> temp = this->user_password();
    return temp.has_value() ? temp->first : Option<String>();
  }

  const Option<Pair<String, String>> user_password() const {
    const int i = this->authority().indexOf("@");
    if (i < 0)
      return {};
    else {
      String userpass = this->authority().substring(0, i);
      const int j = userpass.indexOf(":");
      if (j < 0)
        return {{userpass, String()}};
      else
        return {{userpass.substring(0, j), userpass.substring(j + 1)}};
    }
  }

  const String host() const {
    String temp = this->authority();
    if (temp.isEmpty() || temp.charAt(temp.length() - 1) == '@')
      return String();
    int i = temp.indexOf("@");
    return (i < 0) ? temp : temp.substring(i + 1);
  }

  const fURI host(const String &host) const {
    String temp;
    const Option<Pair<String, String>> x = this->user_password();
    if (x.has_value()) {
      temp = temp + x.value().first;
      if (!x.value().second.isEmpty())
        temp = temp + ":" + x.value().second;
      temp = temp + "@";
    }
    temp = temp + host;
    return this->authority(temp);
  }

  const String authority() const {
    return this->_length > 0 ? String(this->_segments[0]) : String();
  }

  const fURI authority(const String &authority) const {
    fURI temp = fURI(*this);
    if (temp._length == 0)
      return fURI(authority);
    else {
      delete temp._segments[0];
      temp._segments[0] = strdup(authority.c_str());
      return temp;
    }
  }

  const bool hasQuery() const { return this->toString().indexOf("?") != -1; }

  const String query() const {
    for (uint8_t i = 0; i < this->_length; i++) {
      char *ptr = strchr(_segments[i], '?');
      if (ptr) {
        return String(++ptr);
      }
    }
    return String();
  }

  const fURI query(const String &query) const {
    const String temp = this->toString();
    const int index = temp.indexOf("?");
    const fURI furi = -1 == index ? *this : fURI(temp.substring(0, index));
    return query.isEmpty() ? furi : fURI(furi.toString() + "?" + query);
  }

  virtual const bool colocated(const fURI &other) const {
    return this->host().equals(other.host());
  }

  // const char *c_str() const { return this->toString().c_str(); }
  const String toString() const {
    String temp;
    for (uint8_t i = 0; i < this->_length; i++) {
      temp.concat(this->_segments[i]);
      if (i != this->_length - 1)
        temp.concat('/');
    }
    return temp;
  }

  const bool equals(const fURI &other) const {
    if (this->_length != other._length)
      return false;
    for (uint8_t i = 0; i < this->_length; i++) {
      if (strcmp(this->_segments[i], other._segments[i]) != 0)
        return false;
    }
    return true;
  }

  const bool subfuri(const fURI furi) const {
    if (this->_length <= furi._length)
      return false;
    for (uint8_t i = 0; i < furi._length; i++) {
      if (strcmp(this->_segments[i], furi._segments[i]) != 0)
        return false;
    }
    return true;
  }

  const fURI operator/(const char *cstr) const { return this->extend(cstr); }

  const bool operator<(const fURI &furi) const {
    return this->toString() < furi.toString();
  }

  const bool isLocal(const fURI &other) const {
    return this->host().equals(other.host());
  }

  const fURI resolve(const fURI &base) const {
    if (this->authority().isEmpty())
      return base.extend(this->toString().c_str());
    else if (this->host().isEmpty() && !base.host().isEmpty())
      return fURI(this->authority() + base.host()).extend(this->path().c_str());
    else
      return *this;
  }
};

class ID : public fURI {

public:
  ID() : fURI() {};

  ID(const fURI &id) : ID(id.toString()) {};

  ID(const String &furiString) : fURI(furiString) {};

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

class Pattern : public fURI {
public:
  Pattern() : fURI(emptyString) {};

  Pattern(const fURI &fURI) : Pattern(fURI.toString()) {}

  Pattern(const String &furiString) : fURI(furiString) {};

  Pattern(const char *furiCharacters) : fURI(furiCharacters) {};

  const bool colocated(const fURI &furi) const override {
    return furi.authority().equals("#") ||
           -1 != furi.authority().indexOf("+") || fURI::colocated(furi);
  }

  const bool matches(const fURI &pattern) const override {
    return private_fhatos::match(pattern.toString().c_str(),
                                 this->toString().c_str());
  }
};

class IDed {

public:
  explicit IDed(const ID &id) : _id(std::move(id)) {}

  const ID id() const { return _id; }

  const String toString() const { return this->id().toString(); }

  const bool equals(const IDed &other) const {
    return this->_id.equals(other._id);
  }

  virtual Map<String, MutexDeque<IDed *> *> query(const Set<String> &labels) {
    return {};
  }

protected:
  ID _id;
};

} // namespace fhatos

#undef FOS_MAX_FURI_SEGMENT_LENGTH

#endif