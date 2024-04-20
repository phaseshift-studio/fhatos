#ifndef fhat_kernel__structure_hpp
#define fhat_kernel__structure_hpp

#include <fhatos.hpp>
#include <memory>

namespace __private_fhatos {

static int split(const char *text, const char *deliminator, char **&result,
                 uint8_t offset = 0) {
  char *copy = (char *)malloc(strlen(text) + 1);
  char *token;
  strcpy(copy, text);
  int i = offset;
  while ((token = strsep(&copy, deliminator)) != nullptr) {
    if (strlen(token) > 0) {
      result[i] = (char *)malloc(strlen(token) + 1);
      strcpy(result[i], token);
      result[i][strlen(token)] = '\0';
      i++;
    }
  }
  delete copy;
  copy = nullptr;
  return i;
}

static bool match(const char *id_cstr, const char *pattern_cstr) {
  if (strstr(pattern_cstr, "#") == nullptr &&
      strstr(pattern_cstr, "+") == nullptr)
    return strcmp(id_cstr, pattern_cstr) == 0;
  if (strlen(id_cstr) == 0 && strcmp(pattern_cstr, "#") == 0)
    return true;
  char **idParts = new char *[20];
  char **patternParts = new char *[20];
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
  delete idParts;
  delete patternParts;
  idParts = nullptr;
  patternParts = nullptr;
  return result;
}

static char *getIdComponent(const char *id, const uint8_t component) {
  char **idParts = new char *[20];
  const uint8_t count = split(id, "/", idParts);
  char *result = component < count ? idParts[component] : NULL;
  delete idParts;
  return result;
}

static const String getLocationComponent(const String &furi) {
  return furi.substring(0, furi.indexOf("/"));
}
} // namespace __private_fhatos

///////////////////////////
///// fURI/ID/Pattern /////
///////////////////////////

namespace fhatos::kernel {

class fURI {

protected:
  char **__segments;
  uint8_t __length;

public:
  fURI(){};
  fURI(const char *furiCharacters) {
    this->__segments = new char *[20];
    this->__length =
        __private_fhatos::split(furiCharacters, "/", this->__segments);
  };
  fURI(const fURI *parent, const char *extension) {
    String temp = parent->toString();
    temp = temp + "/" + extension;
    if (temp.endsWith("/"))
      temp = temp.substring(0, temp.length() - 1);
    this->__segments = new char *[20];
    this->__length =
        __private_fhatos::split(temp.c_str(), "/", this->__segments);
  };
  fURI(const StringSumHelper &shelper) : fURI(shelper.c_str()){};
  ~fURI() { delete[] this->__segments; }
  const fURI extend(const char *segments) const { return fURI(this, segments); }
  uint8_t length() const { return this->__length; }
  bool empty() const { return 0 == this->__length; }
  virtual bool matches(const fURI &pattern) const {
    return __private_fhatos::match(this->toString().c_str(),
                                   pattern.toString().c_str());
  }
  // bool parentOf(const fURI &furi) const;
  // bool childOf(const fURI &furi) const { return furi.parentOf(*this); }
  String segment(const uint8_t index) const {
    return (index < this->__length) ? String(this->__segments[index])
                                    : String();
  }
  String location() const { return String(this->__segments[0]); }
  virtual bool colocated(const fURI &other) const {
    return strcmp(__segments[0], other.__segments[0]) == 0;
  }
  String toString() const {
    String temp;
    for (uint8_t i = 0; i < this->__length; i++) {
      temp = temp + this->segment(i) + "/";
    }
    return temp.endsWith("/") ? temp.substring(0, temp.length() - 1) : temp;
  }
  bool equals(const fURI &other) const {
    if (this->__length != other.__length)
      return false;
    for (uint8_t i = 0; i < this->__length; i++) {
      if (strcmp(this->__segments[i], other.__segments[i]) != 0)
        return false;
    }
    return true;
  }
  const fURI operator/(const char *cstr) const { return this->extend(cstr); }
  bool operator<(const fURI furi) const {
    return this->toString() < furi.toString();
  }
};

class ID : public fURI {

public:
  ID() : fURI(){};
  ID(const char *furiCharacters) : fURI(furiCharacters) {
    if (strchr(furiCharacters, '#')) {
      throw fError("IDs can not contain pattern symbols: #");
    } else if (strchr(furiCharacters, '+')) {
      throw fError("IDs can not contain pattern symbols: +");
    }
  }
};

class Pattern : public fURI {
public:
  Pattern() : fURI(){};
  Pattern(const char *furiCharacters) : fURI(furiCharacters){};
  virtual bool colocated(const fURI &furi) const override {
    return furi.location().equals("#") || -1 != furi.location().indexOf("+") ||
           fURI::colocated(furi);
  }
  virtual bool matches(const fURI &pattern) const override {
    return __private_fhatos::match(pattern.toString().c_str(),
                                   this->toString().c_str());
  }
};

class IDed {
public:
  IDed(const ID &id) { this->__id = id; }
  const ID id() const { return this->__id; }
  const bool equals(const IDed &other) const {
    return this->id().equals(other.id());
  }

protected:
  ID __id;
};

} // namespace fhatos::kernel

#endif