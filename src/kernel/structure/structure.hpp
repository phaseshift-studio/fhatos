#ifndef fhat_kernel__structure_hpp
#define fhat_kernel__structure_hpp

#include <fhatos.hpp>
#include <memory>

#define FOS_MAX_FURI_SEGMENTS 10

namespace __private_fhatos {

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
} // namespace __private_fhatos

///////////////////////////
///// fURI/ID/Pattern /////
///////////////////////////

// furi://coil@127.0.0.1/devices/device/property

namespace fhatos::kernel {

class fURI {

protected:
  char **__segments;
  uint8_t __length;

public:
  fURI() { __length = 0; }
  fURI(const fURI &furi) {
    this->__segments = new char *[furi.__length];
    for (int i = 0; i < furi.__length; i++) {
      this->__segments[i] = strdup(furi.__segments[i]);
    }
    this->__length = furi.__length;
  };
  fURI(const String &furiString) : fURI(furiString.c_str()) {}
  fURI(const char *furiCharacters) {
    if ((strlen(furiCharacters) == 0) ||
        (strlen(furiCharacters) == 1 && furiCharacters[0] == '/')) {
      this->__length = 0;
    } else {
      uint8_t counter = 0;
      uint8_t length = strlen(furiCharacters);
      for (uint8_t i = 0; i < length; i++) {
        if ((furiCharacters[i]) == '/' &&
            ((i == length - 1) || furiCharacters[i + 1] != '/'))
          counter++;
      }
      this->__segments = new char *[counter + 1];
      if (counter == 0) {
        this->__segments[0] = strdup(furiCharacters);
        this->__length = 1;
      } else {
        this->__length =
            __private_fhatos::split(furiCharacters, "/", this->__segments);
      }
    }
  };
  fURI(const fURI &parent, const char *extension)
      : fURI(parent.toString() + "/" + extension){
            // this->__segments[this->__length++] = strdup(extension);
        };
  fURI(const StringSumHelper &shelper) : fURI(shelper.c_str()){};
  virtual ~fURI() {
    if (this->__length > 0) {
      for (uint8_t i = 0; i < this->__length; i++) {
        delete this->__segments[i];
      }
      delete __segments;
    }
  }
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
    for (uint8_t i = 0; i < this->__length - 1; i++) {
      path = path + "/" + this->__segments[i];
    }
    return fURI(path);
  }
  const uint8_t length() const { return this->__length; }
  const bool empty() const { return 0 == this->__length; }
  virtual bool matches(const fURI &pattern) const {
    return __private_fhatos::match(this->toString().c_str(),
                                   pattern.toString().c_str());
  }
  // bool parentOf(const fURI &furi) const;
  // bool childOf(const fURI &furi) const { return furi.parentOf(*this); }
  const String segment(const uint8_t index) const {
    return String(this->__segments[index]);
  }

  const Option<String> user() const {
    Option<Pair<String, String>> temp = this->user_password();
    return temp.has_value() ? temp->first : Option<String>();
  }

  const Option<Pair<String, String>> user_password() const {
    const int i = this->authority().indexOf("@");
    if (i < 0)
      return Option<Pair<String, String>>();
    String userpass = this->authority().substring(0, i);
    const int j = userpass.indexOf(":");
    if (j < 0)
      return Option<Pair<String, String>>(std::make_pair(userpass, ""));
    return Option<Pair<String, String>>(
        std::make_pair(userpass.substring(0, j), userpass.substring(j + 1)));
  }
  const String host() const {
    String temp = String(this->__segments[0]);
    int i = temp.indexOf("@");
    return i < 0 ? temp : temp.substring(i + 1);
  }
  const String authority() const { return String(this->__segments[0]); }

  virtual bool colocated(const fURI &other) const {
    return strcmp(__segments[0], other.__segments[0]) == 0;
  }
  // const char *c_str() const { return this->toString().c_str(); }
  const String toString() const {
    String temp;
    for (uint8_t i = 0; i < this->__length; i++) {
      temp.concat(this->__segments[i]);
      if (i != this->__length - 1)
        temp.concat('/');
    }
    return temp;
  }
  const bool equals(const fURI &other) const {
    if (this->__length != other.__length)
      return false;
    for (uint8_t i = 0; i < this->__length; i++) {
      if (strcmp(this->__segments[i], other.__segments[i]) != 0)
        return false;
    }
    return true;
  }
  const fURI operator/(const char *cstr) const { return this->extend(cstr); }
  const bool operator<(const fURI furi) const {
    return this->toString() < furi.toString();
  }

  const bool isLocal(const fURI &other) const {
    return this->host().equals(other.host());
  }
};

class ID : public fURI {

public:
  ID() : fURI(){};
  ID(const fURI &id) : ID(id.toString()){};
  ID(const String &furiString) : fURI(furiString){};
  ID(const char *furiCharacters) : fURI(furiCharacters) {
    try {
      if (strchr(furiCharacters, '#')) {
        throw fError("IDs can not contain pattern symbols: #");
      } else if (strchr(furiCharacters, '+')) {
        throw fError("IDs can not contain pattern symbols: +");
      }
    } catch (const fError e) {
      if (this->__length > 0) {
        for (uint8_t i = 0; i < this->__length; i++) {
          delete this->__segments[i];
        }
        delete __segments;
      }
      this->__length = 0;
      throw e;
    }
  }
};

class Pattern : public fURI {
public:
  Pattern() : fURI(""){};
  Pattern(const fURI &fURI) : Pattern(fURI.toString()) {}
  Pattern(const String &furiString) : fURI(furiString){};
  Pattern(const char *furiCharacters) : fURI(furiCharacters){};
  virtual bool colocated(const fURI &furi) const override {
    return furi.authority().equals("#") ||
           -1 != furi.authority().indexOf("+") || fURI::colocated(furi);
  }
  virtual bool matches(const fURI &pattern) const override {
    return __private_fhatos::match(pattern.toString().c_str(),
                                   this->toString().c_str());
  }
};

class IDed {
public:
  IDed(const ID &id) : __id(id) {}
  const ID id() const { return __id; }
  const bool equals(const IDed &other) const {
    return this->__id.equals(other.__id);
  }

protected:
  ID __id;
};

} // namespace fhatos::kernel

#undef FOS_MAX_FURI_SEGMENT_LENGTH

#endif