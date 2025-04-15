#ifndef fhatos_model_hpp
#define fhatos_model_hpp

#include "../lang/obj.hpp"
#include "../fhatos.hpp"
#include <shared_mutex>

namespace fhatos {
  class MODEL_STATES;

  class MODEL_STATES {
    std::shared_mutex map_mutex;
    const unique_ptr<Map<ID, Any, furi_less>> data_ = make_unique<Map<ID, Any, furi_less>>();

  public:
    static ptr<MODEL_STATES> singleton() {
      static auto global = make_shared<MODEL_STATES>();
      return global;
    }

    template<typename T>
    T store(const ID &id, const T &thing) {
      auto lock = std::lock_guard(this->map_mutex);
      this->data_->insert_or_assign(id, thing);
      return thing;
    }

    bool exists(const ID &id) {
      auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
      return this->data_->count(id) > 0;
    }

    template<typename T>
    T load(const ID &id) {
      auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
      if(!this->data_->count(id))
        throw fError::create(id.toString(), "no global value associated with !b%s!!", id.toString().c_str());
      const std::any thing = this->data_->at(id);
      const T t = std::any_cast<T>(thing);
      return t;
    }

    void remove(const ID &id) {
      auto lock = std::lock_guard(this->map_mutex);
      if(!this->data_->count(id))
        return;
      this->data_->erase(id);
    }
  };

  template<typename MODEL_STATE>
  class Model {
  public:
    virtual ~Model() = default;

    static ptr<MODEL_STATE> create_state(const Obj_p &model_obj);

    template<typename FORCED_MODEL_STATE=MODEL_STATE>
    static ptr<FORCED_MODEL_STATE> get_state(const ID &model_id) {
      if(MODEL_STATES::singleton()->exists(model_id))
        return MODEL_STATES::singleton()->load<ptr<FORCED_MODEL_STATE>>(model_id);
      return nullptr;
    }

    template<typename FORCED_MODEL_STATE=MODEL_STATE>
    static ptr<FORCED_MODEL_STATE> get_state(const Obj_p &model_obj) {
      if(!model_obj->vid)
        throw fError("!ystateful objs !rmust have !ya value id!!: %s", model_obj->toString().c_str());
      if(MODEL_STATES::singleton()->exists(*model_obj->vid))
        return MODEL_STATES::singleton()->load<ptr<FORCED_MODEL_STATE>>(*model_obj->vid);
      else {
        const ptr<FORCED_MODEL_STATE> model_state = FORCED_MODEL_STATE::create_state(model_obj);
        MODEL_STATES::singleton()->store(*model_obj->vid, model_state);
        return model_state;
      }
    }

    static void *import();
  };
}

#endif
