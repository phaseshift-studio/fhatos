#pragma once
#ifndef fhatos_oled_hpp
#define fhatos_oled_hpp

#ifdef ARDUINO
#define I2C_ADDRESS 0x3C

#include "../../../fhatos.hpp"
#include "../../../lang/obj.hpp"
#include "../../../structure/router.hpp"
#include "ext/SSD1306Ascii.h"
#include "ext/SSD1306AsciiWire.h"
#include <Wire.h>
#include <Arduino.h>
#include "../../../util/global.hpp"
#include "../../../lang/type.hpp"

namespace fhatos {
  static ID_p OLED_FURI = id_p(FOS_URI "/oled");

  class OLED {

  protected:
    SSD1306AsciiWire ssd1306;

    static ptr<OLED> get_or_create(const Obj_p &oled) {
      if(!GLOBAL::singleton()->exists(oled->vid)) {
        auto oled_state = GLOBAL::singleton()->load<ptr<OLED>>(oled->vid);
        oled_state->ssd1306.begin(&Adafruit128x64, oled->rec_get("config/addr")->int_value());
        oled_state->ssd1306.setFont(Verdana12_bold);
        oled_state->ssd1306.clear();
        GLOBAL::singleton()->store(oled->vid, oled_state);
        return oled_state;
      }
      return GLOBAL::singleton()->load<ptr<OLED>>(oled->vid);
    }

    static Obj_p print_inst(const Obj_p &oled, const InstArgs &args) {
      const ptr<OLED> oled_state = OLED::get_or_create(oled);
      oled_state->ssd1306.setCursor(
          oled->rec_get("pos/0")->int_value(),
          oled->rec_get("pos/1")->int_value());
      string text = args->arg(0)->str_value();
      const bool lf =
          text.length() > 1 &&
          text[text.length() - 1] == 'n' &&
          text[text.length() - 2] == '\\';
      if(lf)
        text = text.substr(0, text.length() - 2);
      if(!(lf ? oled_state->ssd1306.println(text.c_str()) : oled_state->ssd1306.print(text.c_str())))
        throw fError("unable to write to oled at %i", oled->rec_get("addr")->int_value());
      oled->rec_set("pos", lst({jnt(oled_state->ssd1306.col()), jnt(oled_state->ssd1306.row())}));
      return oled;
    }

    static Obj_p clear_inst(const Obj_p &oled, const InstArgs &args) {
      const ptr<OLED> oled_state = get_or_create(oled);
      oled_state->ssd1306.clear(
          args->arg("top")->lst_get(0)->int_value(),
          args->arg("bottom")->lst_get(0)->int_value(),
          args->arg("top")->lst_get(1)->int_value(),
          args->arg("bottom")->lst_get(1)->int_value());
      return oled;
    }

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
      ////////////////
      InstBuilder::build(OLED_FURI->add_component("clear"))
          ->domain_range(OLED_FURI, {1, 1}, OLED_FURI, {1, 1})
          ->inst_args(rec({{"top", lst({jnt(0), jnt(0)})},
                           {"bottom", lst({jnt(128), jnt(64)})}}))
          ->inst_f([](const Obj_p &oled, const InstArgs &args) {
            return OLED::clear_inst(oled, args);
          })
          ->save();
      return nullptr;
    }
  };

}
#endif
#endif
