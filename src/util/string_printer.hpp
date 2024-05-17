#ifndef fhatos_stream_string_hpp
#define fhatos_stream_string_hpp

#ifdef NATIVE
#define VIRTUAL
#define EXTENDS
#else
#define VIRTUAL virtual
#define EXTENDS : public Print
#endif

#include <string>

namespace fhatos {
  class StringPrinter final EXTENDS {
  public:
    explicit StringPrinter(std::string *xstring) : xstring(xstring), length(0), position(0) {
    }

    std::string *get() const {
      return this->xstring;
    }

    VIRTUAL int print(const char *c_str) {
      const int length = strlen(c_str);
      for (int i = 0; i < length; i++) {
        this->write(c_str[i]);
      }
      return length;
    }

    VIRTUAL int print(char c) {
      return this->write(c);
    }


    VIRTUAL void flush() {
    };

    VIRTUAL size_t write(const uint8_t c) {
      *xstring += (char) c;
      return 1;
    };

  private:
    std::string *xstring;
    unsigned int length;
    unsigned int position;
  };
} // namespace fhatos

#endif
