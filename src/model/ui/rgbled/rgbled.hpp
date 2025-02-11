#pragma once
#ifndef fhatos_rgbled_hpp
#define fhatos_rgbled_hpp

#ifndef NATIVE
#include "ext/rgbledxx.h"
#include "../../../fhatos.hpp"
#include "../../../lang/type.hpp"
#include "../../../lang/obj.hpp"
#include "../../../util/obj_helper.hpp"
#include "../../../structure/router.hpp"
#include "../../../util/global.hpp"

namespace fhatos {
  static ID_p RGBLED_FURI = id_p(FOS_URI "/rgbled");

  class RGBLED final {
  protected:
    RGBLEDxx rgbledxx;

    static ptr<RGBLED> get_or_create(const Obj_p &rgbled) {
      if(!GLOBAL::singleton()->exists(rgbled->vid)) {
        GLOBAL::singleton()->store(rgbled->vid, make_shared<RGBLED>(
                                       rgbled->rec_get("config/pin/r")->int_value(),
                                       rgbled->rec_get("config/pin/g")->int_value(),
                                       rgbled->rec_get("config/pin/b")->int_value()));
        Router::singleton()->subscribe(
            Subscription::create(rgbled->vid, p_p(rgbled->vid->extend("color")),
                                 [](const Obj_p &payload, const InstArgs &) {
                                   LOG(INFO, "rgb payload: %s\n", payload->toString().c_str());
                                   // const int red = payload->rec_get("r")->or_else(jnt(0))->int_value();
                                   // const int green = payload->rec_get("g")->or_else(jnt(0))->int_value();
                                   // const int blue = payload->rec_get("b")->or_else(jnt(0))->int_value();
                                   //const auto xx = GLOBAL::singleton()
                                   //    ->load<ptr<RGBLEDxx>>(id_p(args->arg("target")->uri_value().retract()));
                                   //xx->writeRGB(red, green, blue);
                                   //if(red == 0 && green == 0 && blue == 0)
                                   //  xx->turnOff();
                                   return Obj::to_noobj();
                                 }));
      }
      return GLOBAL::singleton()->load<ptr<RGBLED>>(rgbled->vid);
    }

    static Obj_p refresh_inst(const Obj_p &rgbled, const InstArgs &) {
      const ptr<RGBLED> rgbled_state = RGBLED::get_or_create(rgbled);
      rgbled_state->rgbledxx.writeRGB(
          rgbled->rec_get("color/r")->int_value(),
          rgbled->rec_get("color/g")->int_value(),
          rgbled->rec_get("color/b")->int_value());
      return rgbled;
    }

  public:
    RGBLED(int r_pin, int g_pin, int b_pin) :
      rgbledxx(r_pin, g_pin, b_pin, COMMON_CATHODE) {
    }

    static void *import() {
      ////////////////////////// TYPE ////////////////////////////////
      Typer::singleton()->save_type(RGBLED_FURI,
                                    Obj::to_rec({
                                        {"color", Obj::to_rec({
                                             {"r", Obj::to_type(UINT8_FURI)},
                                             {"g", Obj::to_type(UINT8_FURI)},
                                             {"b", Obj::to_type(UINT8_FURI)}})},
                                        {"config", Obj::to_rec({
                                             {"pin", Obj::to_rec({
                                                  {"r", Obj::to_type(UINT8_FURI)},
                                                  {"g", Obj::to_type(UINT8_FURI)},
                                                  {"b", Obj::to_type(UINT8_FURI)}})}})}}));
      ///////////////////////// INSTS ///////////////////////////////////
      InstBuilder::build(RGBLED_FURI->add_component("refresh"))
          ->domain_range(RGBLED_FURI, {1, 1}, RGBLED_FURI, {1, 1})
          ->inst_f([](const Obj_p &rgbled, const InstArgs &args) {
            return RGBLED::refresh_inst(rgbled, args);
          })->save();
      return nullptr;
    }
  };
}
#endif
#endif
