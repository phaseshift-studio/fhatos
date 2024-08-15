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

namespace fhatos {
  template<typename PROCESS = Process, typename STRUCTURE = Structure>
  class Actor : public PROCESS, public STRUCTURE, public Mailbox {

  public:
    explicit Actor(const ID &id, const Pattern &pattern) : PROCESS(id), STRUCTURE(pattern), Mailbox() {
      static_assert(std::is_base_of_v<Process, PROCESS>);
      static_assert(std::is_base_of_v<Structure, STRUCTURE>);
    }
    explicit Actor(const ID &id) : Actor(id, id.extend("#")) {
      // router()->attach(ptr<Structure>((Structure *) this));
      // scheduler()->spawn(ptr<Process>((Process *) this));
    }

    //~Actor() override {
      //PROCESS::~Process();
      //STRUCTURE::~Structure();
    //}

    bool recv_mail(const Mail_p &mail) override {
      if (!this->active())
        return false;
      return this->outbox_->push_back(mail);
    }
    RESPONSE_CODE publish(const ID &target, const Obj_p &payload,
                          const bool retain = TRANSIENT_MESSAGE) { // rename send_mail
      return router()->route_message(
          share(Message{.source = *this->id(), .target = target, .payload = payload, .retain = retain}));
    }
    RESPONSE_CODE subscribe(const Pattern &pattern, const Consumer<Message_p> &onRecv) {
      return router()->route_subscription(
          share(Subscription{.source = ID(*this->id()), .pattern = pattern, .qos = QoS::_1, .onRecv = onRecv}));
      /*.executeAtSource(this)));*/
    }
    RESPONSE_CODE unsubscribe(const Pattern_p &pattern = p_p("#")) { // todo: is this correct?
      router()->route_unsubscribe(this->id(), pattern);
      return OK;
    }

    bool active() { return this->available() && this->running(); }

    /// PROCESS METHODS
    //////////////////////////////////////////////////// SETUP
    virtual void setup() override {
      PROCESS::setup();
      STRUCTURE::setup();
      LOG(INFO, FURI_WRAP FURI_WRAP " !mactor!! activated\n", this->id()->toString().c_str(),
          this->pattern()->toString().c_str());
    }
    //////////////////////////////////////////////////// STOP
    virtual void stop() override {
      if (!this->active())
        return;
      if (const RESPONSE_CODE _rc = this->unsubscribe()) {
        LOG(ERROR, "Actor %s stop error: %s\n", this->id()->toString().c_str(), ResponseCodes.toChars(_rc).c_str());
      }
      PROCESS::stop();
      STRUCTURE::stop();
    }
    //////////////////////////////////////////////////// LOOP
    virtual void loop() override {
      if (!this->active())
        return;
      PROCESS::loop();
      STRUCTURE::loop();
    }
  };
} // namespace fhatos
#endif
