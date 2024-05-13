#ifndef fhatos_stream_string_hpp
#define fhatos_stream_string_hpp

#include <Stream.h>

namespace fhatos {
  class StringStream final : public Stream {
  public:
    explicit StringStream(String *s) : string(s), position(0) {
    }

    // Stream methods
    virtual int available() override { return string->length() - position; }

    virtual int read() override {
      return position < string->length() ? (*string)[position++] : -1;
    }

    virtual int peek() override {
      return position < string->length() ? (*string)[position] : -1;
    }

    virtual void flush() override {
    };
    // Print methods
    virtual size_t write(const uint8_t c) override {
      (*string) += (char) c;
      return 1;
    };

  private:
    String *string;
    unsigned int length;
    unsigned int position;
  };
} // namespace fhatos

#endif // _STRING_STREAM_H_
