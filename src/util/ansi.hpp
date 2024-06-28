/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_ansi_hpp
#define fhatos_ansi_hpp


#include <cstdarg>
#include <stdio.h>
#include <string.h>
#include <util/string_printer.hpp>

namespace fhatos {

  class CPrinter {
  public:
    static CPrinter *singleton() {
      static auto printer = CPrinter();
      return &printer;
    }

#ifdef NATIVE
    static int print(const char *c_str) { return printf("%s", c_str); }
    static void flush() { fflush(stdout); }
#else
    static int print(const char *c_str) { return Serial.printf("%s", c_str); }
    static void flush() { Serial.flush(); }
#endif
  };

  template<typename PRINTER = CPrinter>
  class Ansi {
  public:
    static Ansi *singleton() {
      static Ansi *ansi = new Ansi<PRINTER>();
      return ansi;
    }

  protected:
    PRINTER *printer;
    std::string *_buffer = new std::string();
    StringPrinter *_printer = new StringPrinter(_buffer);
    bool _on = true;


    enum { fg_normal = 30, bg_normal = 40, bright_color = 52 };

    void color(const uint8_t fgcolor, const uint8_t bgcolor) {
      if (this->_on)
        this->printf("\033[0;%dm", fg_normal + fgcolor);
    }

    //  COLOR
    enum {
      BLACK = 0, // !b
      RED = 1, // !r
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
            if (isupper(j))
              this->bold();
            const char jj = tolower(j);
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
            else {
              this->_printer->print(buffer[i]);
              this->_printer->print(buffer[i + 1]);
            }
          }
          i++;
        } else {
          this->_printer->print(buffer[i]);
        }
      }
      this->printer->print(this->_buffer->c_str());
      this->flush();
    }

  public:
    Ansi() : printer(nullptr) {}

    explicit Ansi(PRINTER *printer) : printer(printer) {}

    PRINTER *stream() { return this->_printer; }

    void on(bool turnOn = true) { this->_on = turnOn; }

    void print(const char c) { this->parse(&c, 1); }

    void print(const char *c) { this->parse(c, strlen(c)); }

    void println(const char *c = "") {
      this->print(c);
      this->print('\n');
    }

    PRINTER *getPrinter() { return this->printer; }

    void flush() {
      this->_buffer->clear();
      this->printer->flush();
    }

    const char *strip(const char *s) {
      std::string temp;
      auto *ansi = new Ansi<StringPrinter>(new StringPrinter(new std::string()));
      ansi->on(false);
      ansi->print(s);
      ansi->flush();
      const char *c = (new std::string(*ansi->getPrinter()->get()))->c_str();
      delete ansi;
      return c;
    }

    void printf(const char *format, ...) {
      va_list arg;
      va_start(arg, format);
      char temp[255];
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

    void normal() {
      if (this->_on)
        this->print("\033[0m");
    }

    void underline() {
      if (this->_on)
        this->print("\033[4m");
    }

    void bold() {
      if (this->_on)
        this->print("\033[1m");
    }

    void red(const bool bright = false) { color(RED + (bright ? BRIGHT : 0), 0); }

    void green(const bool bright = false) { color(GREEN + (bright ? BRIGHT : 0), 0); }

    void blue(const bool bright = false) { color(BLUE + (bright ? BRIGHT : 0), 0); }

    void magenta(const bool bright = false) { color(MAGENTA + (bright ? BRIGHT : 0), 0); }

    void cyan(const bool bright = false) { color(CYAN + (bright ? BRIGHT : 0), 0); }

    void white(const bool bright = false) { color(WHITE + (bright ? BRIGHT : 0), 0); }

    void yellow(const bool bright = false) { color(YELLOW + (bright ? BRIGHT : 0), 0); }

    void black(const bool bright = false) { color(BLACK + (bright ? BRIGHT : 0), 0); }
  };
} // namespace fhatos

#endif
