#ifndef fhatos_kernel__ansi_hpp
#define fhatos_kernel__ansi_hpp

#include <Stream.h>
#include <ansi.h>
#include <kernel/util/string_stream.hpp>

namespace fhatos::kernel {

class Ansi : public ANSI {
public:
  Ansi(Stream *baseStream = &::Serial) : ANSI(baseStream) { this->__on = true; }

  Ansi *on() {
    this->__on = true;
    return this;
  }

  Ansi *off() {
    this->__on = false;
    return this;
  }

  Ansi *black() { return this->color(ANSI::black); }

  template <typename... Args> Ansi *black(const char *format, Args... args) {
    return this->color(ANSI::black, format, args...);
  }

  Ansi *white() { return this->color(ANSI::white); }

  template <typename... Args> Ansi *white(const char *format, Args... args) {
    return this->color(ANSI::white, format, args...);
  }

  Ansi *cyan() { return this->color(ANSI::cyan); }

  template <typename... Args> Ansi *cyan(const char *format, Args... args) {
    return this->color(ANSI::cyan, format, args...);
  }

  Ansi *magenta() { return this->color(ANSI::magenta); }

  template <typename... Args> Ansi *magenta(const char *format, Args... args) {
    return this->color(ANSI::magenta, format, args...);
  }

  template <typename... Args> Ansi *blue(const char *format, Args... args) {
    return this->color(ANSI::blue, format, args...);
  }

  Ansi *blue() { return this->color(ANSI::blue); }
  template <typename... Args> Ansi *red(const char *format, Args... args) {
    return this->color(ANSI::red, format, args...);
  }

  Ansi *red() { return this->color(ANSI::red); }

  template <typename... Args> Ansi *yellow(const char *format, Args... args) {
    return this->color(ANSI::yellow, format, args...);
  }

  Ansi *yellow() { return this->color(ANSI::yellow); }
  template <typename... Args> Ansi *green(const char *format, Args... args) {
    return this->color(ANSI::green, format, args...);
  }

  Ansi *green() { return this->color(ANSI::green); }

  Ansi *normal() {
    if (this->__on)
      ANSI::normal();
    return this;
  }

  Ansi *underline() {
    if (this->__on)
      ANSI::underline();
    return this;
  }

  Ansi *bold() {
    if (this->__on)
      ANSI::bold();
    return this;
  }

  String inlineColor(const uint8_t color, const String &text) {
    return this->__on ? String("\033[") + (char)color + "m" + text + "\033[0m"
                      : text;
  }

  template <typename... Args>
  Ansi *color(const uint8_t color, const char *format, Args... args) {
    if (this->__on)
      this->foreground(color);
    this->printf(format, args...);
    if (this->__on)
      ANSI::normal();
    return this;
  }

  Ansi *color(const uint8_t color) {
    if (this->__on)
      this->foreground(color);
    return this;
  }

  Ansi *parse(const String &text) {
    return this->parse(text.c_str(), text.length());
  }

  Ansi *parse(const char *buffer, const int bufferLength) {
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
            this->print(buffer[i]);
        }
        i++;
      } else {
        this->print(buffer[i]);
      }
    }
    return this;
  }

protected:
  bool __on = true;
};
} // namespace fhatos::kernel
#endif