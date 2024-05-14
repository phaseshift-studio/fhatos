#ifndef fhatos_stream_string_hpp
#define fhatos_stream_string_hpp

#include <fhatos.hpp>
#include <Stream.h>


namespace fhatos {
  class StringStream final : public Stream {
  public:
    explicit StringStream(String *s) : a(true), position(0) {
      xstring.a = s;
    }

    explicit StringStream(std::string *s) : a(false), position(0) {
      xstring.b = s;
    }

    std::string* get() {
      return this->xstring.b;
    }

    // Stream methods
    virtual int available() override {
      return a ? xstring.a->length() - position : xstring.b->length() - position;
    }

    virtual int read() override {
      return a
               ? position < xstring.a->length()
                   ? (*xstring.a)[position++]
                   : -1
               : position < xstring.b->length()
                   ? (*xstring.b)[position++]
                   : -1;
    }

    virtual int peek() override {
      return a
               ? position < xstring.a->length()
                   ? (*xstring.a)[position]
                   : -1
               : position < xstring.b->length()
                   ? (*xstring.b)[position]
                   : -1;
    }

    virtual void flush() override {
    };
    // Print methods
    virtual size_t write(const uint8_t c) override {
      if (a) {
        *xstring.a += (char) c;
        return 1;
      } else {
        *xstring.b += (char) c;
        return 1;
      }
    };

  private:
    bool a;

    union {
      String *a;
      std::string *b;
    } xstring;

    unsigned int length;
    unsigned int position;
  };
} // namespace fhatos

#endif // _STRING_STREAM_H_
