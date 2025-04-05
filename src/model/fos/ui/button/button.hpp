#pragma once
#ifndef fhatos_button_hpp
#define fhatos_button_hpp

#ifdef ARDUINO


namespace fhatos {
  const static ID_p BUTTON_FURI = id_p("/fos/ui/button");

  class Button : public Int {
  public:
    static void *import() {
      Typer::singleton()->save_type(*BUTTON_FURI, Obj::to_type(INT_FURI));
      InstBuilder::build(BUTTON_FURI->add_component("on_push"))
          ->domain_range(BUTTON_FURI, {1, 1}, OBJ_FURI, {0, 0})
          ->inst_args(rec({{"code", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &button, const InstArgs &args) {
            ROUTER_WRITE(button->vid()->extend("on_push"),args->arg(0));
            return Obj::to_noobj();
          })->save();
      ///////////////////////////////////////////////////////
      InstBuilder::build(PWM_FURI->add_component("read"))
          ->domain_range(PWM_FURI, {1, 1}, INT_FURI, {1, 1})
          ->inst_f([](const Obj_p &pwm, const InstArgs &) {
            return jnt(analogRead(pwm->int_value()));
          })->save();
      return nullptr;
    }
  };
} // namespace fhatos

#endif
#endif