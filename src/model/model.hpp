#ifndef fhatos_model_hpp
#define fhatos_model_hpp

#include "../lang/obj.hpp"
#include "../fhatos.hpp"

namespace fhatos {
  class MODEL_STATES;

  class MODEL_STATES {
    std::shared_mutex map_mutex;
    const unique_ptr<Map<ID_p, Any, furi_p_less>> data_ = make_unique<Map<ID_p, Any, furi_p_less>>();

  public:
    static ptr<MODEL_STATES> singleton() {
      static auto global = make_shared<MODEL_STATES>();
      return global;
    }

    template<typename T>
    T store(const ID_p &id, const T &thing) {
      auto lock = std::lock_guard(this->map_mutex);
      this->data_->insert_or_assign(id, thing);
      return thing;
    }

    bool exists(const ID_p &id) {
      auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
      return this->data_->count(id) > 0;
    }

    template<typename T>
    T load(const ID_p &id) {
      auto lock = std::shared_lock<std::shared_mutex>(this->map_mutex);
      if(!this->data_->count(id))
        throw fError::create(id->toString(), "no global value associated with !b%s!!", id->toString().c_str());
      const Any thing = this->data_->at(id);
      const T t = std::any_cast<T>(thing);
      return t;
    }

    void remove(const ID_p &id) {
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
    static ptr<FORCED_MODEL_STATE> get_state(const Obj_p &model_obj) {
      if(!model_obj->vid)
        throw fError("!ystateful objs !rmust have !ya value id!!: %s", model_obj->toString().c_str());
      if(MODEL_STATES::singleton()->exists(model_obj->vid))
        return MODEL_STATES::singleton()->load<ptr<FORCED_MODEL_STATE>>(model_obj->vid);
      else {
        const ptr<FORCED_MODEL_STATE> model_state = FORCED_MODEL_STATE::create_state(model_obj);
        MODEL_STATES::singleton()->store(model_obj->vid, model_state);
        return model_state;
      }
    }

    static Obj_p get_inst(const ID &inst_id, const Obj_p &obj, const InstArgs &args) {
      // nat[23].plus()
      Inst_p inst = Router::singleton()->read(id_p(obj->vid->add_component(inst_id)));
      if(inst->is_applicable_inst())
        return inst; //inst->apply(obj, args);
      // nat.plus()
      inst = Router::singleton()->read(id_p(obj->tid->add_component(inst_id)));
      if(inst->is_applicable_inst())
        return inst; //inst->apply(obj, args);
      // plus()
      inst = Router::singleton()->read(id_p(inst_id));
      if(inst->is_applicable_inst())
        return inst; //inst->apply(obj, args);
      ////
      /// int[23].plus(), int.plus(), plus() ...
      const Obj_p objs = Router::singleton()->read(id_p(obj->tid->add_component("#")));
      if(objs->is_objs()) {
        for(const Inst_p &i: *objs->objs_value()) {
          if(i->range()->no_query() == *obj->tid && i->domain()->no_query() != *obj->tid) {
            inst = get_inst(i->domain()->no_query(),
                            make_shared<Obj>(obj->value_, obj->otype, i->domain(), obj->vid), args);
            if(inst->is_applicable_inst())
              return inst;
          }
        }
      }
      return nullptr;
    }

    static void *import();
  };
}

#endif
