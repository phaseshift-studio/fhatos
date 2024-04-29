#ifndef fhatos_kernel__ansi2_hpp
#define fhatos_kernel__ansi2_hpp

#include <Stream.h>
#include <kernel/util/string_stream.hpp>

namespace fhatos::kernel {

template <typename PRINTER> class Ansi {

protected:
  PRINTER *printer;
  String *__buffer = new String();
  StringStream *__stream = new StringStream(__buffer);
  bool __on = true;

  enum { fg_normal = 30, bg_normal = 40, bright_color = 52 };

  void color(const uint8_t fgcolor, const uint8_t bgcolor) {
    this->printf("\033[0;%dm", fg_normal + fgcolor);
  }

  //  COLOR
  enum {
    BLACK = 0, // !b
    RED = 1,   // !r
    GREEN = 2, // !g
    YELLOW = 3,
    BLUE = 4,
    MAGENTA = 5,
    CYAN = 6,
    WHITE = 7,
    BRIGHT = 8, //  Add this to any of the previous 8 to get a bright color
  };

  // STYLE
  enum {
    UNDERLINE, // !_
  };

  //  foreground, background, and color accept one of the following colors:
  //  * color name from above:          ANSI::red
  //  * bright color name from above:   ANSI::red + ANSI::bright
  //  * gray color:                     ANSI::gray2color(gray)
  //  * RGB color:                      ANSI::rgb2color(r, g, b)

  void parse(const char *buffer, const int bufferLength) {
    for (int i = 0; i < bufferLength; i++) {
      if (buffer[i] == '!') {
        const char j = buffer[i + 1];
        if ('_' == j)
          this->underline();
        else if ('!' == j)
          this->normal();
        else {
          if (isUpperCase(j))
            this->bold();
          const char jj = toLowerCase(j);
          if ('r' == jj)
            this->red();
          else if ('g' == jj)
            this->green();
          else if ('b' == jj)
            this->blue();
          else if ('m' == jj)
            this->magenta();
          else if ('c' == jj)
            this->cyan();
          else if ('w' == jj)
            this->white();
          else if ('y' == jj)
            this->yellow();
          else if ('d' == jj)
            this->black();
          else
            this->__stream->print(buffer[i]);
        }
        i++;
      } else {
        this->__stream->print(buffer[i]);
      }
    }
    this->printer->print(this->__buffer->c_str());
    this->__buffer->clear();
  }

public:
  Ansi() {}

  Ansi(PRINTER *printer) : printer(printer) {}

  void on(bool turnOn = true) { this->__on = turnOn; }

  void print(const char c) { this->parse(&c, 1); }

  void print(const char *c) { this->parse(c, strlen(c)); }

  void println(const char *c) {
    this->print(c);
    this->print('\n');
  }

  void printf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[127];
    char *buffer = temp;
    size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
      buffer = new (std::nothrow) char[len + 1];
      if (!buffer) {
        return;
      }
      va_start(arg, format);
      vsnprintf(buffer, len + 1, format, arg);
      va_end(arg);
    }
    this->parse(temp, len);
    if (buffer != temp) {
      delete[] buffer;
    }
  }

  //////////////////////////

  void normal() { this->print("\033[0m"); }

  void underline() { this->print("\033[4m"); }

  void bold() { this->print("\033[1m"); }

  void red(const bool bright = false) { color(RED + (bright ? BRIGHT : 0), 0); }

  void green(const bool bright = false) {
    color(GREEN + (bright ? BRIGHT : 0), 0);
  }

  void blue(const bool bright = false) {
    color(BLUE + (bright ? BRIGHT : 0), 0);
  }

  void magenta(const bool bright = false) {
    color(MAGENTA + (bright ? BRIGHT : 0), 0);
  }

  void cyan(const bool bright = false) {
    color(CYAN + (bright ? BRIGHT : 0), 0);
  }

  void white(const bool bright = false) {
    color(WHITE + (bright ? BRIGHT : 0), 0);
  }

  void yellow(const bool bright = false) {
    color(YELLOW + (bright ? BRIGHT : 0), 0);
  }

  void black(const bool bright = false) {
    color(BLACK + (bright ? BRIGHT : 0), 0);
  }
};

} // namespace fhatos::kernel

#endif