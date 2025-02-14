#pragma once
#ifndef fhatos_oled_hpp
#define fhatos_oled_hpp

#ifdef ARDUINO
#define I2C_ADDRESS 0x3C

#include "../../../../fhatos.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../structure/router.hpp"
#include "ext/SSD1306Ascii.h"
#include "ext/SSD1306AsciiWire.h"
#include <Wire.h>
#include <Arduino.h>
#include "../../../../lang/type.hpp"
#include "../../../model.hpp"

namespace fhatos {
  static ID_p OLED_FURI = id_p(FOS_URI "/oled");

  class OLED final : public Model<OLED>, public PPrinter {

  public:
    SSD1306AsciiWire ssd1306;
    Obj *oled_obj;

    [[nodiscard]] std::pair<uint16_t, uint16_t> pos() override {
      const uint16_t row = this->ssd1306.row();
      const uint16_t col = this->ssd1306.col();
      this->oled_obj->rec_set("pos", lst({jnt(col), jnt(row)}));
      return {row, col};
    }

    void clear() override {
      this->ssd1306.clear();
    }

    void normal() override {
      this->ssd1306.setFont(Verdana12);
    }

    void bold() override {
      this->ssd1306.setFont(Verdana12_bold);
    }

    void italic() override {
      this->ssd1306.setFont(Verdana12_italic);
    }

    void up(const uint16_t rows) override {
      const ptr<std::vector<Obj_p>> pos = this->oled_obj->rec_get("pos")->lst_value();
      const uint16_t new_col = pos->at(0)->int_value();
      const uint16_t new_row = pos->at(1)->int_value() - (ssd1306.fontRows() * rows);
      this->oled_obj->rec_set("pos", lst({pos->at(0), jnt(new_row)}));
      this->ssd1306.setCursor(new_col, new_row);
    }

    void down(const uint16_t rows) override {
      const ptr<std::vector<Obj_p>> pos = this->oled_obj->rec_get("pos")->lst_value();
      const uint16_t new_col = pos->at(0)->int_value();
      const uint16_t new_row = pos->at(1)->int_value() + (ssd1306.fontRows() * rows);
      this->oled_obj->rec_set("pos", lst({pos->at(0), jnt(new_row)}));
      this->ssd1306.setCursor(new_col, new_row);
    }

    void left(const uint16_t cols) override {
      const ptr<std::vector<Obj_p>> pos = this->oled_obj->rec_get("pos")->lst_value();
      const uint16_t new_col = pos->at(0)->int_value() - (ssd1306.fontWidth() * cols);
      const uint16_t new_row = pos->at(1)->int_value();
      this->oled_obj->rec_set("pos", lst({jnt(new_col), pos->at(1)}));
      this->ssd1306.setCursor(new_col, new_row);
    }

    void right(const uint16_t cols) override {
      const ptr<std::vector<Obj_p>> pos = this->oled_obj->rec_get("pos")->lst_value();
      const uint16_t new_col = pos->at(0)->int_value() + (ssd1306.fontWidth() * cols);
      const uint16_t new_row = pos->at(1)->int_value();
      this->oled_obj->rec_set("pos", lst({jnt(new_col), pos->at(1)}));
      this->ssd1306.setCursor(new_col, new_row);
    }

    void teleport(const uint16_t col, const uint16_t row) override {
      this->oled_obj->rec_set("pos", lst({jnt(col), jnt(row)}));
      this->ssd1306.setCursor(col, row);
    }

    void print(char character) override {
      if(!this->ssd1306.print(character))
        throw fError("unable to write to oled at %i", this->oled_obj->rec_get("addr")->int_value());
    }

    void print(const char *chars) override {
      LOG(INFO, "oled: %s\n", chars);
      if(!this->ssd1306.print(chars))
        throw fError("unable to write to oled at %i", this->oled_obj->rec_get("addr")->int_value());
    }

    static ptr<OLED> create_state(const Obj_p &oled) {
      I2C::get_state(Router::singleton()->read(id_p(oled->rec_get("config/i2c")->uri_value())));
      auto oled_state = make_shared<OLED>();
      oled_state->oled_obj = const_cast<Obj *>(oled.get());
      oled_state->ssd1306.begin(&Adafruit128x64, oled_state->oled_obj->rec_get("config/addr")->int_value());
      oled_state->ssd1306.setCursor(
          oled_state->oled_obj->rec_get("pos/0")->int_value(),
          oled_state->oled_obj->rec_get("pos/1")->int_value());
      oled_state->ssd1306.setFont(Verdana12);
      oled_state->ssd1306.clear();
      return oled_state;
    }

    static Obj_p print_inst(const Obj_p &oled, const InstArgs &args) {
      const ptr<OLED> oled_state = OLED::get_state(oled);
      oled_state->oled_obj = const_cast<Obj *>(oled.get());
      oled_state->ssd1306.setCursor(
          oled_state->oled_obj->rec_get("pos/0")->int_value(),
          oled_state->oled_obj->rec_get("pos/1")->int_value());
      const string text = args->arg(0)->str_value();
      parse(text.c_str(), text.length(), oled_state.get());
      oled_state->oled_obj->rec_set("pos",
                                    lst({
                                        jnt(oled_state->ssd1306.col()),
                                        jnt(oled_state->ssd1306.row())}));
      return oled;
    }

    /*static Obj_p clear_inst(const Obj_p &oled, const InstArgs &args) {
      const ptr<OLED> oled_state = get_state(oled);
      oled_state->oled_obj = const_cast<Obj *>(oled.get());
      oled_state->ssd1306.clear(
          args->arg("top")->lst_get(0)->int_value(),
          args->arg("bottom")->lst_get(0)->int_value(),
          args->arg("top")->lst_get(1)->int_value(),
          args->arg("bottom")->lst_get(1)->int_value());
      return oled;
    }*/

  public:
    static void *import() {
      ////////////////////////// TYPE ////////////////////////////////
      Typer::singleton()->save_type(
          OLED_FURI, Obj::to_rec({{"pos", Obj::to_type(LST_FURI)},
                                  {"config", rec({
                                       {"i2c", Obj::to_type(URI_FURI)},
                                       {"addr", Obj::to_type(UINT8_FURI)}})}}));
      ////////////////////////// INSTS ////////////////////////////////
      InstBuilder::build(OLED_FURI->add_component("print"))
          ->domain_range(OLED_FURI, {1, 1}, OLED_FURI, {1, 1})
          ->inst_args(rec({{"text", Obj::to_type(STR_FURI)}}))
          ->inst_f([](const Obj_p &oled, const InstArgs &args) {
            return OLED::print_inst(oled, args);
          })
          ->save();
      return nullptr;
    }

    static void parse(const char *buffer, const uint16_t buffer_length, PPrinter *p) {
      string temp;
      for(uint16_t i = 0; i < buffer_length; i++) {
        if(buffer[i] > 126)
          continue;
        if(buffer[i] == '\\' && buffer[i + 1] == 'n') {
          p->newline();
          i++;
        } else if(buffer[i] == '!') {
          const char j = buffer[i + 1];
          if('!' == j) {
            p->normal();
          } ////////////////////////////////// POSITION !^d##^ = down
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
            /* if(dir == 'S')
               this->save_cursor(steps);
             else if(dir == 'L')
               this->load_cursor(steps);*/
            if(dir == 'u')
              p->up(steps);
            else if(dir == 'l')
              p->left(steps);
            else if(dir == 'd')
              p->down(steps);
            else if(dir == 'r')
              p->right(steps);
            else if(dir == 't') {
              string t;
              for(int m = i + 3; m < buffer_length; m++) {
                if(buffer[m] == '^')
                  break;
                t += buffer[m];
                i = m;
              }
              p->teleport(steps, strtol(t.c_str(), &end, 10));
            }
          }
          ////////////////////////////// FONT
          // else if('*' == j)
          // this->background();
          /*else if('_' == j)
            this->underline();
          else if('-' == j)
            this->strike_through();*/
          else if('B' == j)
            p->bold();
          else if('~' == j)
            p->italic();
            /*else if('*' == j)
              this->blink();*/
          else if('X' == j)
            p->clear();
            /* else if('Q' == j)                ////////////// !Q
               p->top_left();
             else if('Z' == j)
               p->bottom_left();*/
          else if('H' == j) ////////////// !H
            p->home();
          else if(!isalpha(j)) {
            p->print(buffer[i]);
            p->print(j);
          } else {
            ////////////////////////////// COLOR
            if(isupper(j))
              p->bold();
            /*
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
               else {*/
            p->print(buffer[i]);
            p->print(j);
            // }
          }
          i++;
        } else {
          p->print(buffer[i]);
        }
      }
      //p->print(buffer);
      p->flush();
    }
  };

}
#endif
#endif
