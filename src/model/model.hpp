#ifndef fhatos_model_hpp
#define fhatos_model_hpp

#include "../lang/obj.hpp"
#include "../fhatos.hpp"
#include "../util/global.hpp"

namespace fhatos {
  template<typename MODEL_STATE>
  class Model {
  public:
    virtual ~Model() = default;

  protected:
    virtual MODEL_STATE create_state(const Obj_p &model_obj);

    static MODEL_STATE get_or_create(const Obj_p &model_obj) {
      if(GLOBAL::singleton()->exists(model_obj->vid))
        return GLOBAL::singleton()->load<MODEL_STATE>(model_obj->vid);
      else {
        MODEL_STATE model_state = this->create_state(model_obj);
        GLOBAL::singleton()->store(model_obj->vid, model_state);
        return model_state;
      }
    }

  public:
    virtual void *import();
  };
}

#endif
