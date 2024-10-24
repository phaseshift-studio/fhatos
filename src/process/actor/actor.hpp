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
#pragma once
#ifndef fhatos_actor_hpp
#define fhatos_actor_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <process/process.hpp>
#include <structure/pubsub.hpp>
#include <structure/structure.hpp>
#include <util/options.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <language/insts.hpp>
#include <structure/stype/heap.hpp>

namespace fhatos {
  //template<class P, class S, class Process, class Structure>
  //concept CheckStructureProcess = std::is_base_of_v<Process, P> && std::is_base_of_v<Structure, S>;
  //requires CheckStructureProcess<PROCESS, STRUCTURE, Process, Structure>
  template<typename PROCESS = Process, typename STRUCTURE = Structure>
  class Actor
      : public PROCESS,
        public STRUCTURE {
  public:
    explicit Actor(const ID &id, const Pattern &pattern): PROCESS(id),
                                                          STRUCTURE(pattern) {
      static_assert(std::is_base_of_v<Process, PROCESS>);
      static_assert(std::is_base_of_v<Structure, STRUCTURE>);
    }

    /* virtual ~Actor() {
       STRUCTURE::~STRUCTURE();
       PROCESS::~PROCESS();
     }*/

    explicit Actor(const ID &id): Actor(id, id.extend("#")) {
    }

    void publish(
      const ID &target, const Obj_p &payload,
      const bool retain = TRANSIENT_MESSAGE) {
      this->should_be_active();
      // rename send_mail
      router()->write(id_p(target), payload, retain);
      //router()->route_message(
      //share(Message{.target = target, .payload = payload, .retain = retain}));
    }

    void subscribe(const Pattern &pattern, const Consumer<Message_p> &on_recv) {
      this->should_be_active();
      router()->route_subscription(subscription_p(*this->id(), pattern, Insts::to_bcode(on_recv)));
      /*.executeAtSource(this)));*/
    }


    void unsubscribe(const Pattern_p &pattern = p_p("#")) {
      this->should_be_active();
      router()->route_unsubscribe(this->id(), pattern);
    }

    virtual bool active() {
      return this->available() && this->running();
    }

    /// PROCESS METHODS
    //////////////////////////////////////////////////// SETUP
    void setup() override {
      STRUCTURE::setup();
      PROCESS::setup();
      if (!this->id_->matches(*this->pattern())) {
        const ptr<Heap> id_struct = Heap::create(*this->id());
        router()->attach(id_struct);
        id_struct->setup();
      }
      this->subscribe(*this->id(), [this](const Message_p &message) {
        if (message->payload->is_noobj() &&
            message->retain &&
            this->active()) {
          this->stop();
        }
      });
      LOG(INFO, FURI_WRAP FURI_WRAP " !mactor!! activated\n", this->id()->toString().c_str(),
          this->pattern()->toString().c_str());
    }

    //////////////////////////////////////////////////// STOP
    void stop() override {
      if (!this->id_->matches(*this->pattern()))
        router()->detach(p_p(*this->id_));
      router()->detach(this->pattern());
      PROCESS::stop();
      STRUCTURE::stop();
    }

    //////////////////////////////////////////////////// LOOP
    void loop() override {
      if (this->running())
        PROCESS::loop();
      if (this->available())
        STRUCTURE::loop();
    }

    virtual string toString() const {
      return string("!mACTOR!!!g[!cid!m=>!g<!y") + this->id()->toString() + "!g>!m,!cpattern!m=>!g<!y" + this->pattern()
             ->toString() +
             "!g>]!!";
    }

  protected:
    virtual void distribute_to_subscribers(const Message_p &message) override {
      STRUCTURE::distribute_to_subscribers(message);
      if (std::is_base_of_v<Coroutine, PROCESS>)
        STRUCTURE::loop();
    }

    void should_be_active() {
      if (!this->active())
        throw fError(FURI_WRAP " is not active (spawned and attached)", this->toString().c_str());
    }
  };

  template<typename PROCESS, typename STRUCTURE>
  using Actor_p = ptr<Actor<PROCESS, STRUCTURE>>;
} // namespace fhatos
#endif
