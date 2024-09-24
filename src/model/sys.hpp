/*******************************************************************************
FhatOS: A Distributed Operating System
  Copyright (c) 2024 PhaseShift Studio, LLC

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_sys_hpp
#define fhatos_sys_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  class System : public Actor<Thread, KeyValue> {
    friend class XScheduler;
    friend class Router;

  protected:
    System(): Actor<Thread, KeyValue>("/sys/") {
    }

    ID_p PROCESS_ID(const PType *ptype = nullptr) const {
      static ID_p process_id = id_p(this->id()->extend("process/"));
      if (nullptr == ptype)
        return process_id;
      return id_p(process_id->extend(ProcessTypes.to_chars(*ptype)));
    }

    ID_p STRUCTURE_ID(const SType *stype = nullptr) const {
      static ID_p structure_id = id_p(this->id()->extend("structure/"));
      if (nullptr == stype)
        return structure_id;
      return id_p(structure_id->extend(StructureTypes.to_chars(*stype)));
    }

    const ID_p SUBSCRIPTION_ID = id_p(this->id()->extend("subscription/"));

    ID_p BARRIER_ID(const ID_p &barrier_label = nullptr) const {
      static ID_p process_id = id_p(this->id()->extend("barrier/"));
      if (nullptr == barrier_label)
        return process_id;
      return id_p(process_id->extend(*barrier_label));
    }

    ID_p ACTOR_ID(const ID_p &actor_label = nullptr) const {
      static ID_p actor_id = id_p(this->id()->extend("actor"));
      if (nullptr == actor_label)
        return actor_id;
      return id_p(actor_id->extend(*actor_label));
    }

  public:
    static ptr<System> singleton() {
      static ptr<System> system = ptr<System>(new System());
      return system;
    }

    Obj_p read(const fURI_p &furi) override {
      const Option<Obj_p> meta = this->try_meta(furi);
      if (meta.has_value()) return meta.value();
      // TODO: source
      if (furi->is_pattern()) {
        // PATTERN
        Objs_p objs = Obj::to_objs();
        Objs *objs_ptr = objs.get();

        /*  for (const auto &actor: *Swarm::singleton()->actors_) {
            if (ACTOR_ID()->extend(*actor->id()).matches(*furi))
              objs->add_obj(uri(ACTOR_ID()->extend(*actor->id())));
          }*/
        scheduler()->processes_->forEach([this,furi,objs_ptr](const Process_p &process) {
          if (PROCESS_ID(&process->ptype)->extend(*process->id()).matches(*furi))
            objs_ptr->add_obj(uri(PROCESS_ID(&process->ptype)->extend(*process->id())));
        });
        if (BARRIER_ID(scheduler()->current_barrier_)->matches(*furi))
          objs->add_obj(uri(BARRIER_ID(scheduler()->current_barrier_)));

        for (const auto &[patt, struc]: *router()->structures_) {
          if (STRUCTURE_ID(&struc->stype)->extend(*struc->pattern()).matches(*furi))
            objs->add_obj(uri(STRUCTURE_ID(&struc->stype)->extend(*struc->pattern())));
          for (const auto &sub: struc->get_subscription_objs(p_p(*furi))) {
            objs->add_obj(sub->rec_get(uri(":source")));
          }
        }
        return objs;
      } else {
        // ID
        if (furi->equals(*BARRIER_ID(scheduler()->current_barrier_)))
          return dool(true);
          /*else if (ACTOR_ID()->is_subfuri_of(*furi)) {
            for (const auto& actor: *Swarm::singleton()->actors_) {
              if (furi->matches(ACTOR_ID()->extend(*actor->id())))
                return rec({
                  {uri("process"), rec({
                                         {uri("id"), uri(actor->id())},
                                         {uri("setup"), bcode()},
                                         {uri("loop"), bcode()}},
                                       id_p(REC_FURI->resolve(ProcessTypes.to_chars(actor->ptype))))},
                  {uri("structure"), rec({
                                           {uri("pattern"), uri(*((Structure*)actor.get())->pattern_)},
                                           {uri("setup"), bcode()},
                                           {uri("loop"), bcode()}},
                                         id_p(REC_FURI->resolve(StructureTypes.to_chars(((Structure*)actor.get())->stype))))}});
            }*/
        else if (PROCESS_ID()->is_subfuri_of(*furi)) {
          const Option<Process_p> option = scheduler()->processes_->find([this,furi](const Process_p &process) {
            return furi->matches(PROCESS_ID(&process->ptype)->extend(*process->id()));
          });
          if (option.has_value()) {
            const Process_p process = option.value();
            return rec({
                         {uri("id"), uri(process->id())},
                         {uri("setup"), bcode()},
                         {uri("loop"), bcode()}}, id_p(REC_FURI->resolve(ProcessTypes.to_chars(process->ptype))));
          }
        } else if (SUBSCRIPTION_ID->is_subfuri_of(*furi)) {
          Objs_p objs = Obj::to_objs();
          for (const auto &[patt, struc]: *router()->structures_) {
            for (const auto &sub: struc->get_subscription_objs(
                   p_p(furi->toString().substr(0, SUBSCRIPTION_ID->toString().length()).c_str()))) {
              objs->add_obj(sub);
            }
          }
          return objs;
        }
      }
      return noobj();
    }
  };
}
#endif
