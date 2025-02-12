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

    static ptr<MODEL_STATE> create_state(const Obj_p &model_obj);

    static ptr<MODEL_STATE> get_or_create(const Obj_p &model_obj) {
      if(!model_obj->vid)
        throw fError("!ystateful objs !rmust have !ya value id!!: %s",model_obj->toString().c_str());
      if(GLOBAL::singleton()->exists(model_obj->vid))
        return GLOBAL::singleton()->load<ptr<MODEL_STATE>>(model_obj->vid);
      else {
        ptr<MODEL_STATE> model_state = MODEL_STATE::create_state(model_obj);
        GLOBAL::singleton()->store(model_obj->vid, model_state);
        return model_state;
      }
    }

    static Obj_p get_inst(const ID &inst_id, const Obj_p &obj, const InstArgs &args) {
      // nat[23].plus()
      Inst_p inst = Router::singleton()->read(id_p(obj->vid->add_component(inst_id)));
      if(inst->is_applicable_inst()) return inst; //inst->apply(obj, args);
      // nat.plus()
      inst = Router::singleton()->read(id_p(obj->tid->add_component(inst_id)));
      if(inst->is_applicable_inst()) return inst; //inst->apply(obj, args);
      // plus()
      inst = Router::singleton()->read(id_p(inst_id));
      if(inst->is_applicable_inst()) return inst; //inst->apply(obj, args);
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
