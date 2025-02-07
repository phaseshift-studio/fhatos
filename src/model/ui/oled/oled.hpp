#pragma once
#ifndef fhatos_oled_hpp
#define fhatos_oled_hpp

#ifndef NATIVED
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

  class OLED final : public Rec {
  public:
    explicit OLED(const ID &id, const ID &i2c_id, const int addr, ptr<SSD1306AsciiWire> ssd1306) :
      Rec(rmap({
              {"config", rec({{"addr", jnt(addr)}, {"i2c_id", vri(i2c_id)}})},
              {"pos", lst({jnt(0), jnt(0)})}}), OType::REC, OLED_FURI, id_p(id)) {
      // value inst
      InstBuilder::build(this->vid_->add_component("clear"))
          ->domain_range(OLED_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_args(rec({{"top", lst({jnt(0), jnt(0)})},
                           {"bottom", lst({jnt(ssd1306->displayHeight()), jnt(ssd1306->displayWidth())})}}))
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            const ptr<SSD1306AsciiWire> ssd1306 =
                GLOBAL::singleton()->take<ptr<SSD1306AsciiWire>>(obj->vid_);
            ssd1306->clear(
                args->arg("top")->lst_get(1)->int_value(),
                args->arg("bottom")->lst_get(1)->int_value(),
                args->arg("top")->lst_get(0)->int_value(),
                args->arg("bottom")->lst_get(0)->int_value());
            return obj;
          })
          ->save();
      GLOBAL::singleton()->offer(this->vid_, ssd1306);
      ssd1306->begin(&Adafruit128x64, addr);
      ssd1306->setFont(Verdana12_bold);
      ssd1306->clear();
    }


    void stop() {
      const ptr<SSD1306AsciiWire> ssd1306 = GLOBAL::singleton()->take<ptr<SSD1306AsciiWire>>(this->vid_, true);
      ssd1306->clear();
    }

    static ptr<OLED> create(const ID &id, const ID &i2c_id, const int addr) {
      const Rec_p i2c = Router::singleton()->read(id_p(i2c_id));
      const uint8_t i2c_sda = i2c->rec_get("sda")->int_value();
      const uint8_t i2c_scl = i2c->rec_get("scl")->int_value();
      Wire.begin(i2c_sda, i2c_scl);
      Wire.setClock(400000L);
      const auto s = make_shared<SSD1306AsciiWire>();
      return std::make_shared<OLED>(id, i2c_id, addr, s);
    }

    static void *import(const ID &lib_id = "/io/lib/oled") {
      // type definition
      Typer::singleton()->save_type(
          OLED_FURI, Obj::to_rec({{"pos", Obj::to_type(LST_FURI)},
                                  {"config", rec({
                                       {"i2c_id", Obj::to_type(URI_FURI)},
                                       {"addr", Obj::to_type(INT_FURI)}})}}));
      // library insts
      InstBuilder::build(ID(lib_id.extend(":create")))
          ->domain_range(OBJ_FURI, {0, 1}, OLED_FURI, {1, 1})
          ->inst_args(rec({{"id", Obj::to_type(URI_FURI)}, {"i2c_id", Obj::to_type(URI_FURI)},
                           {"addr", jnt(I2C_ADDRESS)}}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return OLED::create(
                ID(args->arg("id")->uri_value()),
                args->arg("i2c_id")->uri_value(),
                args->arg("addr")->int_value());
          })->save();
      // type insts
      InstBuilder::build(OLED_FURI->add_component("print"))
          ->domain_range(OLED_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->type_args(x(0))
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            const Lst_p pos = obj->rec_get("pos");
            const int row = pos->lst_value()->at(0)->int_value();
            const int col = pos->lst_value()->at(1)->int_value();
            const ptr<SSD1306AsciiWire> ssd1306 =
                GLOBAL::singleton()->take<ptr<SSD1306AsciiWire>>(obj->vid_);
            ssd1306->setCursor(col, row);
            string text = args->arg(0)->str_value();
            const bool lf =
                text.length() > 1 &&
                text[text.length() - 1] == 'n' &&
                text[text.length() - 2] == '\\';
            if(lf)
              text = text.substr(0, text.length() - 2);
            if(!(lf ? ssd1306->println(text.c_str()) : ssd1306->print(text.c_str())))
              throw fError("unable to write to oled at %i", obj->rec_get("addr")->int_value());
            obj->rec_set("pos", lst({jnt(ssd1306->row()), jnt(ssd1306->col())}));
            return obj;
          })
          ->save();
      return nullptr;
    }
  };
}
#endif
#endif
