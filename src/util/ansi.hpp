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
#pragma once
#ifndef fhatos_ansi_hpp
#define fhatos_ansi_hpp


#include <cstdarg>
#include <memory>
#include <random>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <util/options.hpp>
#include <util/string_printer.hpp>

using namespace std;

namespace fhatos {
  class CPrinter {
  public:
    static CPrinter *singleton() {
      static CPrinter printer = CPrinter();
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
    static shared_ptr<Ansi<PRINTER>> singleton() {
      static Ansi<PRINTER> ansi = Ansi<PRINTER>();
      static shared_ptr<Ansi<PRINTER>> ansi_p = shared_ptr<Ansi<PRINTER>>(&ansi, [](const auto *) {
      });
#ifndef NATIVE
      static bool _setup = false;
      if (!_setup) {
        _setup = true;
        Serial.begin(FOS_SERIAL_BAUDRATE);
        Serial.setTimeout(FOS_SERIAL_TIMEOUT);
      }
#endif
      return ansi_p;
    }

  protected:
    PRINTER printer;
    std::string buffer_ = std::string();
    StringPrinter printer_ = StringPrinter(&buffer_);
    bool on_ = true;


    enum {
      fg_normal = 30, bg_normal = 40, bright_color = 52
    };

    void color(const uint8_t fgcolor, const uint8_t) {
      if (this->on_)
        this->printf("\033[0;%dm", fg_normal + fgcolor);
    }

    //  COLOR
    enum {
      BLACK = 0, // !b
      RED = 1, // !r
      GREEN = 2, // !g
      YELLOW = 3, // !y
      BLUE = 4, // !b
      MAGENTA = 5, // !m
      CYAN = 6, // !c
      WHITE = 7, // !w
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

    void parse(const char *buffer, const int buffer_length) {
      for (int i = 0; i < buffer_length; i++) {
        if (buffer[i] < 0 || buffer[i] > 126)
          continue;
        if (buffer[i] == '!') {
          const char j = buffer[i + 1];
          if ('!' == j)
            this->normal();
            //else if('*' == j)
            //  this->background();
          else if ('_' == j)
            this->underline();
          else if ('~' == j)
            this->italic();
          else if ('*' == j)
            this->blink();
          else if ('X' == j)
            this->clear();
          else if ('Q' == j)
            this->top_left();
          else if ('Z' == j)
            this->bottom_left();
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
              this->printer_.print(buffer[i]);
              this->printer_.print(buffer[i + 1]);
            }
          }
          i++;
        } else {
          this->printer_.print(buffer[i]);
        }
      }
      this->printer.print(this->buffer_.c_str());
      this->flush();
    }

  public:
    Ansi() : printer(*CPrinter::singleton()) {
    }

    explicit Ansi(string *str) : printer(StringPrinter(str)) {
    }

    explicit Ansi(PRINTER printer) : printer(printer) {
    }

    void on(bool turn_on = true) { this->on_ = turn_on; }

    void print(const char c) { this->parse(&c, 1); }

    void print(const char *c) { this->parse(c, strlen(c)); }

    void println(const char *c = "") {
      if (strlen(c) > 0)
        this->print(c);
      this->print('\n');
    }

    PRINTER get_printer() { return this->printer; }

    void flush() {
      this->buffer_.clear();
      this->printer.flush();
    }

    static string strip(const string &s) {
      auto a = std::string();
      auto b = StringPrinter(&a);
      auto ansi = Ansi<StringPrinter>(b);
      ansi.on(false);
      ansi.print(s.c_str());
      ansi.flush();
      string ret = string(ansi.get_printer().get());
      return ret;
    }

    void printf(const char *format, ...) {
      char message[1024];
      va_list arg;
      va_start(arg, format);
      int length = vsnprintf(message, 1023, format, arg);
      va_end(arg);
      message[length] = '\0';
      this->parse(message, length);
    }

    //////////////////////////

    void normal() {
      if (this->on_)
        this->print("\033[0m");
    }

    void clear() {
      if (this->on_)
        this->print("\033[2J");
    }

    void italic() {
      if (this->on_)
        this->print("\033[3m");
    }

    void underline() {
      if (this->on_)
        this->print("\033[4m");
    }

    void bold() {
      if (this->on_)
        this->print("\033[1m");
    }

    void blink() {
      if (this->on_)
        this->print("\033[5m");
    }

    //void background() {
    //  if (this->_on)
    //    this->print("\033[40m");
    //}

    ////////// POSITIONING

    void top_left() {
      if (this->on_)
        this->print("\033[H");
    }

    void bottom_left() {
      if (this->on_)
        this->print("\033[F");
    }


    ////////// COLORING

    void red(const bool bright = false) { color(RED + (bright ? BRIGHT : 0), 0); }

    void green(const bool bright = false) { color(GREEN + (bright ? BRIGHT : 0), 0); }

    void blue(const bool bright = false) { color(BLUE + (bright ? BRIGHT : 0), 0); }

    void magenta(const bool bright = false) { color(MAGENTA + (bright ? BRIGHT : 0), 0); }

    void cyan(const bool bright = false) { color(CYAN + (bright ? BRIGHT : 0), 0); }

    void white(const bool bright = false) { color(WHITE + (bright ? BRIGHT : 0), 0); }

    void yellow(const bool bright = false) { color(YELLOW + (bright ? BRIGHT : 0), 0); }

    void black(const bool bright = false) { color(BLACK + (bright ? BRIGHT : 0), 0); }

    void up() {
      if (this->on_)
        this->print("\033[1A");
    }

    void down() {
      if (this->on_)
        this->print("\033[1B");
    }

    void left() {
      if (this->on_)
        this->print("\033[1D");
    }

    void right() {
      if (this->on_)
        this->print("\033[1C");
    }

    static string silly_print(const char *text, const bool rainbow = true, const bool rollercoaster = true) {
      srand(time(nullptr));
      const string colors = "rgbmcy";
      string ret;
      for (size_t i = 0; i < strlen(text); i++) {
        if (rainbow)
          ret = ret.append("!").append(string("") + colors[rand() % colors.length()]);
        ret = ret.append(string("") +
                         (char) (rollercoaster ? (rand() % 2 ? tolower(text[i]) : toupper(text[i])) : text[i]));
      }
      if (rainbow)
        ret = ret.append("!!");
      return ret;
    }
  };

  template<typename PRINTER = Ansi<>>
  shared_ptr<PRINTER> printer() {
    return Options::singleton()->printer<PRINTER>();
  }
} // namespace fhatos

#endif
