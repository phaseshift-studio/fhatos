#ifndef fhat_kernel__structure_hpp
#define fhat_kernel__structure_hpp

#include <fhatos.hpp>
#include <memory>

namespace __private_fhatos {

static int split(const char *text, const char *deliminator, char **&result) {
  char *copy = (char *)malloc(strlen(text) + 1);
  char *token;
  strcpy(copy, text);
  int i = 0;
  while ((token = strsep(&copy, deliminator)) != nullptr) {
    result[i] = (char *)malloc(strlen(token) + 1);
    strcpy(result[i++], token);
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

namespace fhatos {
namespace kernel {

class fURI {

protected:
  std::shared_ptr<const char[]> characters;
  std::shared_ptr<const fURI> parent;

public:
  fURI(const char *furiCharacters) {
    this->parent = std::shared_ptr<fURI>(fURI::root());
    this->characters = std::shared_ptr<const char[]>(furiCharacters);
  }
  fURI(const fURI *parent, const char *extension) {
    this->parent = std::shared_ptr<const fURI>(parent);
    this->characters = std::shared_ptr<const char[]>(extension);
  }
  ~fURI() {
    // delete characters;
    // delete parent;
  }
  static fURI *root() {
    static fURI *root = new fURI("");
    return nullptr;
  }
  const fURI extend(const char *segments) const { return fURI(this, segments); }
  uint8_t length() const;
  bool empty() const { return 0 == this->length(); }
  virtual bool matches(const fURI &pattern) const {
    return __private_fhatos::match(this->c_str(), pattern.c_str());
  }
  bool parentOf(const fURI &furi) const;
  bool childOf(const fURI &furi) const { return furi.parentOf(*this); }
  String segment(const uint8_t index) const {
    return String(__private_fhatos::getIdComponent(this->c_str(), index));
  }
  fURI location() const { return fURI(this->segment(0).c_str()); }
  virtual bool colocated(const fURI &furi) const {
    return furi.location().equals(this->location());
  }

  const char *c_str() const {
    return this->parent->empty() ? this->characters.get()
                                 : (std::string(this->parent->c_str()) +
                                    std::string(this->characters.get()))
                                       .c_str();
  }
  String toString() const { return String(this->c_str()); }
  bool equals(const fURI &other) const {
    return this->toString().equals(other.toString());
  }
};

class ID : public fURI {

public:
  ID(const char *furiCharacters) : fURI(furiCharacters) {
    if (strchr(furiCharacters, '#')) {
      // throw ferror<"IDs can not contain pattern symbols: #">;
    } else if (strchr(furiCharacters, '+')) {
    }
    // throw ferror<"IDs can not contain pattern symbols: +">;
    // fURI::fURI(furiCharacters);
  }
};

class Pattern : public fURI {
  virtual bool colocated(const fURI &furi) const override {
    return furi.location().toString().equals("#") ||
           -1 != furi.location().toString().indexOf("+") ||
           fURI::colocated(furi);
  }
  virtual bool matches(const fURI &pattern) const override {
    return __private_fhatos::match(pattern.c_str(), this->c_str());
  }
};
} // namespace kernel
} // namespace fhatos

#endif