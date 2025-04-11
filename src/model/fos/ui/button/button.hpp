#pragma once
#ifndef fhatos_button_hpp
#define fhatos_button_hpp

//
#include "../../../../fhatos.hpp"
#include "../../../../lang/type.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../../io/gpio/gpio.hpp"
#include "../../../model.hpp"
//


namespace fhatos {
  const static ID_p BUTTON_FURI = id_p("/fos/ui/button");

  class Button final : public Model<Button> {
  public:
    bool state = false;


    explicit Button(const Obj_p &button_obj) {
    };

    static ptr<Button> create_state(const Obj_p &button_obj) {
      const auto button_state = make_shared<Button>(button_obj);
      return button_state;
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
            const Int_p reading = __(pin).inst("read").compute().next();
            //LOG_WRITE(INFO, button.get(),L("button value {}\n", reading->toString()));
            if(!button_state->state && reading->is_int() && reading->int_value() > 0) {
              LOG_WRITE(INFO, button.get(),L("button {} pushed\n", pin->toString()));
              mmADT::delift(button->rec_get("on_push"))->apply(button);
            }
            button_state->state = reading->int_value() > 0;
            MODEL_STATES::singleton()->store<ptr<Button>>(*button->vid, button_state);
            return Obj::to_noobj();
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif
