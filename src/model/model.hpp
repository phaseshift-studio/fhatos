#ifndef fhatos_model_hpp
#define fhatos_model_hpp

#include <shared_mutex>
#include "../fhatos.hpp"
#include "../lang/obj.hpp"

namespace fhatos {
  class MODEL_STATES;
  static uptr<Map<ID, Function<Obj_p, Any>>> MODEL_CREATOR = make_unique<Map<ID, Function<Obj_p, Any>>>();
  class MODEL_STATES {
    std::shared_mutex map_mutex;
    const ptr<Map<ID, Any, furi_less>> data_ = make_shared<Map<ID, Any, furi_less>>();


  public:
    static ptr<MODEL_STATES> singleton() {
      static auto global = make_shared<MODEL_STATES>();
      return global;
    }

    void drop(const ID &id) {
      auto lock = std::lock_guard(this->map_mutex);
      this->data_->erase(ID(id));
    }

    template<typename T>
    T store(const ID &id, const T &thing) {
      auto lock = std::lock_guard(this->map_mutex);
      this->data_->insert_or_assign(ID(id), thing);
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

    template<typename FORCED_MODEL_STATE = MODEL_STATE>
    static void drop_state(const Obj_p &model_obj) {
      if(!model_obj->vid)
        throw fError("!ystateful objs !rmust have !ya value id!!: %s", model_obj->toString().c_str());
      if(MODEL_STATES::singleton()->exists(*model_obj->vid))
        MODEL_STATES::singleton()->drop(*model_obj->vid);
      else
        LOG_WRITE(WARN, model_obj.get(), L("model {} already removed", model_obj->vid->toString()));
    }

    template<typename FORCED_MODEL_STATE = MODEL_STATE>
    static ptr<FORCED_MODEL_STATE> get_state(const ID &model_id) {
      if(MODEL_STATES::singleton()->exists(model_id))
        return MODEL_STATES::singleton()->load<ptr<FORCED_MODEL_STATE>>(model_id);
      return nullptr;
    }

    template<typename FORCED_MODEL_STATE = MODEL_STATE>
    static ptr<FORCED_MODEL_STATE> get_state(const Obj_p &model_obj) {
      if(!model_obj->vid)
        throw fError("!ystateful objs !rmust have !ya value id!!: %s", model_obj->toString().c_str());
      if(MODEL_STATES::singleton()->exists(*model_obj->vid))
        return MODEL_STATES::singleton()->load<ptr<FORCED_MODEL_STATE>>(*model_obj->vid);
      else {
        const ptr<FORCED_MODEL_STATE> model_state =
            MODEL_CREATOR->count(*model_obj->tid)
                ? std::any_cast<ptr<FORCED_MODEL_STATE>>(MODEL_CREATOR->at(*model_obj->tid)(model_obj))
                : FORCED_MODEL_STATE::create_state(model_obj);
        MODEL_STATES::singleton()->store(*model_obj->vid, model_state);
        return model_state;
      }
    }

    static void *import();
  };
} // namespace fhatos

#endif
