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

  class Button :  public Model<Button> {


  public:
    bool state = false;


    explicit Button(const Obj_p &button_obj) {

    };

    static ptr<Button> create_state(const Obj_p &button_obj) {
      const ptr<Button> button_state = make_shared<Button>(button_obj);
      return button_state;
    }

    static void *import() {
      Typer::singleton()->save_type(*BUTTON_FURI,
                                    Obj::to_rec({{"pin", Obj::to_bcode()},
                                                 {"on_push",Obj::to_bcode()}}));
      InstBuilder::build(BUTTON_FURI->add_component("loop"))
          ->domain_range(BUTTON_FURI,{1,1},OBJ_FURI,{0,1})
          ->inst_args(Obj::to_rec())
          ->inst_f([](const Obj_p& button, const InstArgs &args) {
 			const ptr<Button> button_state = Button::get_state(button);
       		const uint8_t pin = button->rec_get("pin")->int_value();
            const uint8_t value = Obj::to_inst(Obj::to_inst_args(), id_p(GPIO_FURI->add_component("read")))
                          	->apply(jnt(pin))->int_value();
        	LOG_WRITE(INFO,button.get(),L("button {}: current {}/past {}",pin, value, button_state->state));
            if(!button_state->state && value > 0)
            	button->rec_get("on_push")->apply(button);
            button_state->state = value > 0;
            return Obj::to_noobj();
            })->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif