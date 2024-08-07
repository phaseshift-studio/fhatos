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
//
#include <process/actor/mailbox.hpp>
#include <process/actor/publisher.hpp>
#include <structure/router/local_router.hpp>
#include <structure/router/pubsub_artifacts.hpp>
#include <structure/router/router.hpp>
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(thread.hpp)

#include <language/exts.hpp>

namespace fhatos {
  template<typename PROCESS = Thread /*, typename STRUCTURE = XSpace*/>
  class Actor : public PROCESS, public Publisher, public Mailbox<ptr<Mail>> {

  protected:
    MutexDeque<ptr<Mail>> inbox;
    Consumer<Actor *> _setupFunction = nullptr;
    Consumer<Actor *> _loopFunction = nullptr;
    Option<ptr<Mail>> pop() override { return this->inbox.pop_front(); }

    virtual bool next() {
      const Option<ptr<Mail>> mail = this->pop();
      if (!mail.has_value())
        return false;
      mail->get()->first->execute(mail->get()->second);
      /// delete mail->second.payload;
      return true;
    }

  public:
    explicit Actor(const ID &id, const Consumer<Actor<PROCESS> *> &setupFunction = nullptr,
                   const Consumer<Actor<PROCESS> *> &loopFunction = nullptr) :
        PROCESS(id), Publisher(this, this), Mailbox<ptr<Mail>>(), _setupFunction(setupFunction),
        _loopFunction(loopFunction) {
      static_assert(std::is_base_of_v<XProcess, PROCESS>);
      // static_assert(std::is_base_of_v<Router, ROUTER>);
    }

    virtual ~Actor() override {
      this->inbox.clear();
      PROCESS::~PROCESS();
    }

    /*  Exts extension() {
        return Exts
      }*/

    // PAYLOAD BOX METHODS
    bool push(const ptr<Mail> mail) override { return this->running() && this->inbox.push_back(mail); }

    uint16_t size() override { return inbox.size(); }

    /// PROCESS METHODS
    //////////////////////////////////////////////////// SETUP
    virtual void setup() override {
      if (this->_running) {
        LOG(ERROR, "Actor %s has already started [setup()]\n", this->id()->toString().c_str());
        return;
      }
      PROCESS::setup();
      if (this->_setupFunction) {
        this->_setupFunction(this);
      }
    }
    //////////////////////////////////////////////////// STOP
    virtual void stop() override {
      if (!this->_running) {
        LOG(ERROR, "Actor %s has already stopped [stop()]\n", this->id()->toString().c_str());
        return;
      }
      PROCESS::stop();
      if (const RESPONSE_CODE _rc = this->unsubscribeSource()) {
        LOG(ERROR, "Actor %s stop error: %s\n", this->id()->toString().c_str(), RESPONSE_CODE_STR(_rc));
      }
      this->inbox.clear();
    }
    //////////////////////////////////////////////////// LOOP
    virtual void loop() override {
      if (!this->running()) {
        LOG(ERROR, "Actor %s has already stopped [loop()]\n", this->id()->toString().c_str());
        return;
      }
      PROCESS::loop();
      while (this->running() && this->next()) {
      }
      if (this->running() && this->_loopFunction)
        this->_loopFunction(this);
    }

    /// STRUCTURE METHODS
  };
} // namespace fhatos
#endif
