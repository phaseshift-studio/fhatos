#pragma once
#ifndef fhatos_button_hpp
#define fhatos_button_hpp

//
#include "../../../../fhatos.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../../../model.hpp"
#include "../../io/gpio/gpio.hpp"
#include "../../sys/typer/typer.hpp"
//


namespace fhatos {
  const static ID_p BUTTON_FURI = id_p("/fos/ui/button");

  class Button final : public Model<Button> {
  protected:
    Mutex mutex = Mutex();

  public:
    int state = 0;

    explicit Button() {
    };

    static ptr<Button> create_state(const Obj_p &) {
      return make_shared<Button>();
    }

    static void *import() {
      Typer::singleton()->save_type(*BUTTON_FURI,
                                    Obj::to_rec({{"pin", Obj::to_bcode()},
                                                 {"on_push", Obj::to_bcode()}}));
      InstBuilder::build(BUTTON_FURI->add_component("loop"))
          ->domain_range(BUTTON_FURI, {1, 1}, OBJ_FURI, {0, 1})
          ->inst_f([](const Obj_p &button, const InstArgs &) {
            const ptr<Button> button_state = Button::get_state(button);
            const Int_p pin = button->rec_get("pin");
            const int reading = pin->inst_apply("read")->or_else(jnt(-1))->int_value();
            if(reading != -1 && (button_state->state != reading)) {
              LOG_WRITE(INFO, button.get(),L("button {} pushed\n", pin->toString()));
              mmADT::delift(button->rec_get("on_push"))->apply(button);
              button_state->state = reading;
            }
            return Obj::to_noobj();
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif
