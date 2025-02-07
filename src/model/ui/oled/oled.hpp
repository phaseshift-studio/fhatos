#pragma once
#ifndef fhatos_oled_hpp
#define fhatos_oled_hpp

#ifndef NATIVE
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
  static ID_p OLED_FURI = id_p("/fos/oled");

  class OLED {
  public:
    static void *import() {
      // type definition
      Typer::singleton()->save_type(
          OLED_FURI, Obj::to_rec({{"pos", Obj::to_type(LST_FURI)},
                                  {"config", rec({
                                       {"i2c_id", Obj::to_type(URI_FURI)},
                                       {"addr", Obj::to_type(INT_FURI)}})}}));
      //////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////
      // type insts
      InstBuilder::build(OLED_FURI->add_component("print"))
          ->domain_range(OLED_FURI, {1, 1}, OLED_FURI, {1, 1})
          ->inst_args(rec({{"text", Obj::to_type(STR_FURI)}}))
          ->inst_f([](const Obj_p &oled, const InstArgs &args) {
            const ptr<SSD1306AsciiWire> ssd1306 = get_ssd1306(oled);
            ssd1306->setCursor(
                oled->rec_get("pos/0")->int_value(),
                oled->rec_get("pos/1")->int_value());
            string text = args->arg(0)->str_value();
            const bool lf =
                text.length() > 1 &&
                text[text.length() - 1] == 'n' &&
                text[text.length() - 2] == '\\';
            if(lf)
              text = text.substr(0, text.length() - 2);
            if(!(lf ? ssd1306->println(text.c_str()) : ssd1306->print(text.c_str())))
              throw fError("unable to write to oled at %i", oled->rec_get("addr")->int_value());
            oled->rec_set("pos", lst({jnt(ssd1306->col()), jnt(ssd1306->row())}));
            return oled;
          })
          ->save();
      ///////////////////////////////////////////////////////////////////
      InstBuilder::build(OLED_FURI->add_component("clear"))
          ->domain_range(OLED_FURI, {1, 1}, OLED_FURI, {1, 1})
          ->inst_args(rec({{"top", lst({jnt(0), jnt(0)})},
                           {"bottom", lst({jnt(128), jnt(64)})}}))
          ->inst_f([](const Obj_p &oled, const InstArgs &args) {
            const ptr<SSD1306AsciiWire> ssd1306 = get_ssd1306(oled);
            ssd1306->clear(
                args->arg("top")->lst_get(0)->int_value(),
                args->arg("bottom")->lst_get(0)->int_value(),
                args->arg("top")->lst_get(1)->int_value(),
                args->arg("bottom")->lst_get(1)->int_value());
            return oled;
          })
          ->save();
      return nullptr;
    }
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    static ptr<SSD1306AsciiWire> get_ssd1306(const Obj_p &oled) {
      const ID_p state_id = id_p(OLED_FURI->extend(oled->rec_get("config/i2c_id")->uri_value()));
      const bool exists = GLOBAL::singleton()->exists(state_id);
      const ptr<SSD1306AsciiWire> ssd1306 =
          exists
            ? GLOBAL::singleton()->load<ptr<SSD1306AsciiWire>>(state_id)
            : GLOBAL::singleton()->store<ptr<SSD1306AsciiWire>>(
                state_id, make_shared<SSD1306AsciiWire>());
      if(!exists) {
        ssd1306->begin(&Adafruit128x64, oled->rec_get("config/addr")->int_value());
        ssd1306->setFont(Verdana12_bold);
        ssd1306->clear();
      }
      return ssd1306;
    }
  };

}
#endif
#endif
