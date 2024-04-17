#ifndef fhat_kernel__structure_hpp
#define fhat_kernel__structure_hpp

#include <fhatos.hpp>
#include <memory>

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
  bool matches(const fURI &pattern) const;
  bool parentOf(const fURI &furi) const;
  bool childOf(const fURI &furi) const { return furi.parentOf(*this); }
  String segment(const uint8_t index) const;
  fURI location() const { return *this; } // fURI(this->segment(0)); }
  bool colocated(const fURI &furi) const {
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

class Pattern : public fURI {};

} // namespace kernel
} // namespace fhatos

#endif