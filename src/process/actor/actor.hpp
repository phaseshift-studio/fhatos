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
#include <process/process.hpp>
#include <structure/furi.hpp>
#include <structure/router/pubsub_artifacts.hpp>
#include <structure/structure.hpp>

namespace fhatos {
  template<typename PROCESS, typename STRUCTURE>
  class Actor : public PROCESS, public STRUCTURE, public Mailbox {

  public:
    explicit Actor(const ID &id, const Pattern &pattern) : PROCESS(id), STRUCTURE(pattern), Mailbox() {
      static_assert(std::is_base_of_v<Process, PROCESS>);
      static_assert(std::is_base_of_v<Structure, STRUCTURE>);
    }
    explicit Actor(const ID &id) : Actor(id, id.extend("#")) {}

    virtual ~Actor() = default;
    void recv_mail(Mail_p mail) override { this->outbox->push_back(mail); }
    virtual RESPONSE_CODE publish(const Message_p &outgoing) { return Rooter::singleton()->route_message(outgoing); }
    RESPONSE_CODE publish(const ID &target, const Obj_p &payload, const bool retain = false) {
      return this->publish(
          share(Message{.source = *this->id(), .target = target, .payload = payload, .retain = retain}));
    }
    RESPONSE_CODE subscribe(const Pattern &pattern, const Consumer<Message_p> &onRecv) {
      return this->subscribe(share(Subscription{
          .source = *this->id(), .pattern = this->pattern()->resolve(pattern), .qos = QoS::_1, .onRecv = onRecv}));
                                       /*.executeAtSource(this)));*/
    }
    virtual RESPONSE_CODE unsubscribeSource() { return OK; }
    virtual RESPONSE_CODE subscribe(const Subscription_p &outgoing) {
      return Rooter::singleton()->route_subscription(outgoing);
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
      PROCESS::stop();
      STRUCTURE::stop();
      if (const RESPONSE_CODE _rc = this->unsubscribeSource()) {
        LOG(ERROR, "Actor %s stop error: %s\n", this->id()->toString().c_str(), ResponseCodes.toChars(_rc));
      }
    }
    //////////////////////////////////////////////////// LOOP
    virtual void loop() override {
      PROCESS::loop();
      STRUCTURE::loop();
    }
  };
} // namespace fhatos
#endif
