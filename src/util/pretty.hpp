#ifndef fhatos_pretty_hpp
#define fhatos_pretty_hpp

#include <fhatos.hpp>

namespace fhatos {
  class Pretty {
  public:
    static string prettyBytes(const int bytes, Ansi<StringPrinter> *ansi = nullptr) {
      string x;
      Ansi<StringPrinter>* temp;
      if (ansi)
        temp = ansi;
      else
        temp = new Ansi<StringPrinter>(new StringPrinter(&x));
      if (constexpr float tb = 1099511627776; bytes >= tb)
        temp->printf("%.2f tb", static_cast<float>(bytes) / tb);
      else if (constexpr float gb = 1073741824; bytes >= gb && bytes < tb)
        temp->printf("%.2f gb", static_cast<float>(bytes) / gb);
      else if (constexpr float mb = 1048576; bytes >= mb && bytes < gb)
        temp->printf("%.2f mb", static_cast<float>(bytes) / mb);
      else if (constexpr float kb = 1024; bytes >= kb && bytes < mb)
        temp->printf("%.2f kb", static_cast<float>(bytes) / kb);
      else
        temp->printf("%i bytes", bytes);
      return string(temp->stream()->get()->c_str());
    }
  };
}

#endif
