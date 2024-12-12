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
#include <iostream>
#include <memory>
#include <random>
#include <stdio.h>
#include <string.h>
#include <thread>
#include <time.h>
#include "options.hpp"
#include "string_printer.hpp"

namespace fhatos {
  class CPrinter {
  public:
    static CPrinter *singleton() {
      static auto printer = CPrinter();
      return &printer;
    }
#ifdef NATIVE
    static int print(const char *c_str) {
      const size_t length = strlen(c_str);
      std::cout.write(c_str, length);
      return length;
    }

    static int read() {
      return getchar();
    }

    static void flush() {
      fflush(stdout);
    }
#else
    static int print(const char *c_str) {
      return Serial.print(c_str);
    }
    static void flush() {
      Serial.flush();
    }

    static int read() {
      return Serial.available() <= 0 ? -1 : Serial.read();
    }
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
    uint16_t *slots[10] = {};
    bool on_ = true;

    enum { fg_normal = 30, bg_normal = 40, bright_color = 52 };

    void color(const uint8_t fgcolor, const uint8_t) {
      if(this->on_)
        this->printf("\033[0;%dm", fg_normal + fgcolor);
    }

    //  foreground, background, and color accept one of the following colors:
    //  * color name from above:          ANSI::red
    //  * bright color name from above:   ANSI::red + ANSI::bright
    //  * gray color:                     ANSI::gray2color(gray)
    //  * RGB color:                      ANSI::rgb2color(r, g, b)

    void parse(const char *buffer, const uint16_t buffer_length) {
      for(uint16_t i = 0; i < buffer_length; i++) {
        if(buffer[i] > 126)
          continue;
        if(buffer[i] == '!') {
          const char j = buffer[i + 1];
          if('!' == j)
            this->normal();
            ////////////////////////////////// POSITION
          else if('^' == j) {
            const char dir = buffer[i + 2];
            string s;
            for(int m = i + 3; m < buffer_length; m++) {
              if(buffer[m] == '^')
                break;
              s += buffer[m];
              i = m;
            }
            char *end;
            uint8_t steps = strtol(s.c_str(), &end, 10);
            if(dir == 'S')
              this->save_cursor(steps);
            else if(dir == 'L')
              this->load_cursor(steps);
            else if(dir == 'u')
              this->up(steps);
            else if(dir == 'l')
              this->left(steps);
            else if(dir == 'd')
              this->down(steps);
            else if(dir == 'r')
              this->right(steps);
            else if(dir == 't') {
              string t;
              for(int m = i + 3; m < buffer_length; m++) {
                if(buffer[m] == '^')
                  break;
                t += buffer[m];
                i = m;
              }
              this->teleport(steps, strtol(t.c_str(), &end, 10));
            }
          }
          ////////////////////////////// FONT
          // else if('*' == j)
          //   this->background();
          else if('_' == j)
            this->underline();
          else if('-' == j)
            this->strike_through();
          else if('~' == j)
            this->italic();
          else if('*' == j)
            this->blink();
          else if('X' == j)
            this->clear();
          else if('Q' == j)
            this->top_left();
          else if('Z' == j)
            this->bottom_left();
          else if('H' == j)
            this->home();
          else if(!isalpha(j)) {
            this->printer_.print(buffer[i]);
            this->printer_.print(j);
          } else {
            ////////////////////////////// COLOR
            if(isupper(j))
              this->bold();
            const char jj = tolower(j);
            if('r' == jj)
              this->red();
            else if('g' == jj)
              this->green();
            else if('b' == jj)
              this->blue();
            else if('m' == jj)
              this->magenta();
            else if('c' == jj)
              this->cyan();
            else if('w' == jj)
              this->white();
            else if('y' == jj)
              this->yellow();
            else if('d' == jj)
              this->black();
            else {
              this->printer_.print(buffer[i]);
              this->printer_.print(j);
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
    Ansi() : Ansi(*CPrinter::singleton()) {
    }

    explicit Ansi(string *str) : Ansi(StringPrinter(str)) {
    }

    explicit Ansi(const PRINTER printer) : printer(printer) {
      for(int i = 0; i < 10; i++) {
        uint16_t t[2] = {0, 0};
        this->slots[i] = t;
      }
    }

    void on(bool turn_on = true) { this->on_ = turn_on; }

    bool is_on() const { return this->on_; }

    void print(const char c) { this->parse(&c, 1); }

    void print(const char *c) { this->parse(c, strlen(c)); }

    void println(const char *c = "") {
      if(strlen(c) > 0)
        this->print(c);
      this->print('\n');
    }

    PRINTER get_printer() { return this->printer; }

    int read() const {
      return this->printer.read();
    }

    void flush() {
      this->printer.flush();
      this->buffer_.clear();
    }

    static string strip(const string &s) {
      auto a = std::string();
      const auto b = StringPrinter(&a);
      auto ansi = Ansi<StringPrinter>(b);
      ansi.on(false);
      ansi.print(s.c_str());
      ansi.flush();
      auto ret = string(ansi.get_printer().get());
      return ret;
    }

    void printf(const char *format, ...) {
      va_list arg;
      va_start(arg, format);
      char *message;
      const size_t length = vasprintf(&message, format, arg);
      va_end(arg);
      if(format[strlen(format) - 1] == '\n')
        message[length - 1] = '\n';
      message[length] = '\0';
      this->parse(message, length);
      free(message);
    }

    //////////////////////////

    void normal() {
      if(this->on_)
        this->print("\033[0m");
    }

    void clear() {
      if(this->on_)
        this->print("\033[2J");
    }

    void italic() {
      if(this->on_)
        this->print("\033[3m");
    }

    void underline() {
      if(this->on_)
        this->print("\033[4m");
    }

    void strike_through() {
      if(this->on_)
        this->print("\033[9m");
    }

    void reverse() {
      if(this->on_)
        this->print("\033[7m");
    }

    void bold() {
      if(this->on_)
        this->print("\033[1m");
    }

    void blink() {
      if(this->on_)
        this->print("\033[5m");
    }

    void clear_line() {
      if(this->on_)
        this->print("\033[2K");
    }

    // void background() {
    //   if (this->_on)
    //     this->print("\033[40m");
    // }

    ////////// POSITIONING

    void top_left() {
      if(this->on_)
        this->print("\033[H");
    }

    void bottom_left() {
      if(this->on_)
        this->print("\033[F");
    }


    ////////// COLORING
    /*
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
        */
    void black(const bool bright = false) { color(0 + (bright ? 8 : 0), 0); }
    void red(const bool bright = false) { color(1 + (bright ? 8 : 0), 0); }
    void green(const bool bright = false) { color(2 + (bright ? 8 : 0), 0); }
    void yellow(const bool bright = false) { color(3 + (bright ? 8 : 0), 0); }
    void blue(const bool bright = false) { color(4 + (bright ? 8 : 0), 0); }
    void magenta(const bool bright = false) { color(5 + (bright ? 8 : 0), 0); }
    void cyan(const bool bright = false) { color(6 + (bright ? 8 : 0), 0); }
    void white(const bool bright = false) { color(7 + (bright ? 8 : 0), 0); }

    /////////////// CURSOR MOVEMENT ///////////////

    void left(const uint16_t columns = 1) {
      this->move('D', columns);
    }

    void right(const uint16_t columns = 1) {
      this->move('C', columns);
    }

    void down(const uint16_t rows = 1) {
      /*for (int i = 0; i < rows; i++) {
        this->print('\n');
      }*/
      this->move('B', rows);
    }

    void up(const uint16_t rows = 1) {
      this->move('A', rows);
    }

    void teleport(const uint16_t row, const uint16_t column) {
      if(this->on_) {
        this->print("\033[");
        this->print(std::to_string(row).c_str());
        this->print(';');
        this->print(std::to_string(column).c_str());
        this->print('H');
      }
    }

    void home() {
      if(this->on_) {
        this->print("\033[H");
      }
    }

    void to_column(const uint16_t column) {
      this->move('G', column);
    }

    void move(const char direction, const uint16_t columns_or_rows) {
      if(this->on_) {
        this->print("\033[");
        this->print(std::to_string(columns_or_rows).c_str());
        this->print(direction);
      }
    }

    void cursor(const bool visible) {
      if(this->on_) {
        this->print(visible ? "\x1b[?25h" : "\x1b[?25l");
      }
    }

    void save_cursor(const uint8_t slot = 0) {
      if(this->on_) {
        if(0 == slot) {
          this->print("\033[s");
        } else {
          this->location(this->slots[slot]);
        }
      }
    }

    void load_cursor(const uint8_t slot = 0) {
      if(this->on_) {
        if(0 == slot) {
          this->print("\033[u");
        } else {
          this->teleport(this->slots[slot][0], this->slots[slot][1]);
        }
      }
    }

    void location(uint16_t *pos) {
      if(this->on_) {
        this->print("\033[6n");
        this->read(); // esc
        this->read(); // [
        const char a[] = {(char) this->read()};
        pos[0] = atoi(a);
        this->read(); // ;
        const char b[] = {(char) this->read()};
        pos[1] = atoi(b);
        this->read(); // R
      }
    }

    /*
    ESC[H	moves cursor to home position (0, 0)
    ESC[{line};{column}H
    ESC[{line};{column}f	moves cursor to line #, column #
    ESC[#A	moves cursor up # lines
    ESC[#B	moves cursor down # lines
    ESC[#C	moves cursor right # columns
    ESC[#D	moves cursor left # columns
    ESC[#E	moves cursor to beginning of next line, # lines down
    ESC[#F	moves cursor to beginning of previous line, # lines up
    ESC[#G	moves cursor to column #
    ESC[6n	request cursor position (reports as ESC[#;#R)
    ESC M	moves cursor one line up, scrolling if needed
    ESC 7	save cursor position (DEC)
    ESC 8	restores the cursor to the last saved position (DEC)
    ESC[s	save cursor position (SCO)
    ESC[u	restores the cursor to the last saved position (SCO)
    */

    static string silly_print(const char *text, const bool rainbow = true, const bool rollercoaster = true) {
      srand(time(nullptr));
      const string colors = "rgbmcy";
      string ret;
      for(size_t i = 0; i < strlen(text); i++) {
        if(rainbow)
          ret = ret.append("!").append(string("") + colors[rand() % colors.length()]);
        ret =
            ret.append(string("") +
                       static_cast<char>(rollercoaster ? (rand() % 2 ? tolower(text[i]) : toupper(text[i])) : text[i]));
      }
      if(rainbow)
        ret = ret.append("!!");
      return ret;
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////// PROGRESS BAR /////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////

  struct ProgressBar {
  protected:
    Ansi<> *ansi_;
    const uint8_t total_counts_;
    uint8_t current_counts_;
    const char *meter_icon_;

    ProgressBar(Ansi<> *ansi, const uint8_t total_counts, const char *meter_icon = "#") : ansi_(ansi),
      total_counts_(total_counts), current_counts_(0), meter_icon_(meter_icon) {
    }

  public:
    static shared_ptr<ProgressBar> start(Ansi<> *ansi, const uint8_t total_counts, const char *meter_icon = "#") {
      ansi->cursor(false);
      return shared_ptr<ProgressBar>(new ProgressBar(ansi, total_counts, meter_icon));
    }

    bool done() const { return this->current_counts_ >= this->total_counts_; }

    void end(const string &end_message = "done") {
      this->current_counts_ = this->total_counts_;
      this->incr_count(end_message);
      this->ansi_->cursor(true);
    }

    void incr_count(const string &message = "") {
      uint8_t percentage =
          0 == this->current_counts_
            ? 0
            : ((static_cast<float>(this->current_counts_) / static_cast<float>(this->total_counts_)) * 100.f);
      ++this->current_counts_;
      if(percentage >= 100) {
        percentage = 100;
        this->ansi_->clear_line();
      }
      if(this->ansi_->is_on()) {
        const size_t meter_icon_size = Ansi<>::strip(this->meter_icon_).length();
        this->ansi_->print("!g[INFO]  [!b");
        for(int j = 0; j < percentage; j = j + 2 + (meter_icon_size - 1)) {
          // + 2 to make bar half as long
          this->ansi_->print(this->meter_icon_);
        }
        this->ansi_->print("!!");
        for(int j = percentage; j < 99; j = j + 2) {
          this->ansi_->print(' ');
        }

        this->ansi_->printf("!g] !y%i%%!! %-25s\r", percentage, message.c_str());
      }
    }
  };

  ////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////// TREE BAR /////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////

  struct ObjTree {
  };


  template<typename PRINTER = Ansi<>>
  shared_ptr<PRINTER> printer() {
    return Options::singleton()->printer<PRINTER>();
  }
} // namespace fhatos

#endif
