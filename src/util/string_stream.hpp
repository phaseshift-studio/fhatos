#ifndef fhatos_stream_string_hpp
#define fhatos_stream_string_hpp

#include <Stream.h>

namespace fhatos {

class StringStream : public Stream {
public:
  StringStream(String *s) : string(s), position(0) {}

  // Stream methods
  virtual int available() { return string->length() - position; }
  virtual int read() {
    return position < string->length() ? (*string)[position++] : -1;
  }
  virtual int peek() {
    return position < string->length() ? (*string)[position] : -1;
  }
  virtual void flush(){};
  // Print methods
  virtual size_t write(uint8_t c) {
    (*string) += (char)c;
    return 1;
  };

private:
  String *string;
  unsigned int length;
  unsigned int position;
};

} // namespace fhatos

#endif // _STRING_STREAM_H_