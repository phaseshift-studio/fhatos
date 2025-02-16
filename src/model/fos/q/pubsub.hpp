#pragma once
#ifndef fhatos_q_pubsub_hpp
#define fhatos_q_pubsub_hpp

/*#include "../../../fhatos.hpp"
#include "../../../lang/obj.hpp"
#include "../../../structure/router.hpp"
#include "../../../lang/type.hpp"*/
#include "../../model.hpp"

namespace fhatos {
  //  static const ID_p MESSAGE_FURI = id_p(FOS_URI "/q/msg");
  //  static const ID_p SUBSCRIPTION_FURI = id_p(FOS_URI "/q/sub");

  class PubSub final : public Model<PubSub> {

    /*public:
      Map<ID_p, MutexDeque<Message>> messages_ = Map<ID_p, MutexDeque<Message>>();
      Map<ID_p, MutexDeque<Subscription>> subscriptions_ = Map<ID_p, MutexDeque<Subscription>>();


      static ptr<PubSub> create_state(const Obj_p &oled) {
        return nullptr;
      }

      static Obj_p on_read(const Obj_p &obj, const InstArgs &args) {
        const ptr<PubSub> pubsub_state = PubSub::get_state(obj);
        return Obj::to_noobj();
      }

    public:
      static void *import() {
        ////////////////////////// TYPE ////////////////////////////////
        Typer::singleton()->save_type(
            *MESSAGE_FURI, Obj::to_rec({
                {"dst", Obj::to_type(URI_FURI)},
                {"data", Obj::to_bcode()},
                {"retain", Obj::to_type(BOOL_FURI)}}));
        Typer::singleton()->save_type(
            *SUBSCRIPTION_FURI, Obj::to_rec({
                {"src", Obj::to_type(URI_FURI)},
                {"match", Obj::to_type(URI_FURI)},
                {"on_recv", Obj::to_type(INST_FURI)}}));
        ////////////////////////// INSTS ////////////////////////////////
        InstBuilder::build(SUBSCRIPTION_FURI->extend("on_read"))
            ->inst_args(rec({{"src", Obj::to_noobj()}, {"data", Obj::to_noobj()}}))
            ->inst_f([](const Obj_p &obj, const InstArgs &args) {
              return Obj::to_noobj();
            });
        return nullptr;
      }*/
  };
}
#endif
