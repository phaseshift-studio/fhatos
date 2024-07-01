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

#ifndef fhatos_actor_hpp
#define fhatos_actor_hpp

#include <fhatos.hpp>
//
#include <process/actor/mailbox.hpp>
#include <process/router/local_router.hpp>
#include <process/router/message.hpp>
#include <process/router/publisher.hpp>
#include <process/router/router.hpp>
#include <util/mutex_deque.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  template<typename PROCESS = Thread>
  class Actor : public PROCESS, public Publisher, public Mailbox<ptr<Mail>> {
  public:
    explicit Actor(const ID &id, const Consumer<Actor<PROCESS> *> setupFunction = nullptr,
                   const Consumer<Actor<PROCESS> *> loopFunction = nullptr) :
        _setupFunction(setupFunction), _loopFunction(loopFunction), PROCESS(id), Publisher(this, this) {
      static_assert(std::is_base_of_v<Process, PROCESS>);
      // static_assert(std::is_base_of_v<Router, ROUTER>);
    }

    /* ~Actor<PROCESS, ROUTER>() override {
       this->Publisher<ROUTER>::~Publisher();
       this->Mailbox<Mail>::~Mailbox();
     }*/

    Pair<fbyte *, uint> serialize() const {
      auto *bytes = static_cast<fbyte *>(malloc(sizeof(*this)));
      memcpy(bytes, reinterpret_cast<const fbyte *>(this), sizeof(*this));
      return {bytes, sizeof(*this)};
    }

    static Actor<PROCESS> *deserialize(const fbyte *bytes) { return (Actor<PROCESS> *) bytes; }

    // PAYLOAD BOX METHODS
    bool push(const ptr<Mail> mail) override { return this->running() && this->inbox.push_back(mail); }

    void query(const TargetID &queryId, const Consumer<const Message> &onRecv = nullptr) {
      if (onRecv) {
        this->subscribe(queryId, [this, queryId, onRecv](const Message &message) {
          if (message.retain) {
            this->unsubscribe(queryId);
            onRecv(message);
          }
        });
        this->yield();
        this->loop();
      }
      this->publish(queryId, Obj::to_noobj(), TRANSIENT_MESSAGE);
    }

    uint16_t size() override { return inbox.size(); }

    /// PROCESS METHODS
    virtual void setup() override {
      if (this->_running) {
        LOG(ERROR, "Actor %s has already started [setup()]\n", this->id()->toString().c_str());
        return;
      }
      ///////////////////////////////////////////////////////
      PROCESS::setup();
      if (this->_setupFunction) {
        this->_setupFunction(this);
      }
    }

    virtual void stop() override {
      if (!this->_running) {
        LOG(ERROR, "Actor %s has already stopped [stop()]\n", this->id()->toString().c_str());
        return;
      }
      PROCESS::stop();
      ///////////////////////////////////////////////////////
      if (const RESPONSE_CODE _rc = this->unsubscribeSource()) {
        LOG(ERROR, "Actor %s stop error: %s\n", this->id()->toString().c_str(), RESPONSE_CODE_STR(_rc));
      }
      this->inbox.clear();
    }

    virtual void loop() override {
      if (!this->running()) {
        LOG(ERROR, "Actor %s has already stopped [loop()]\n", this->id()->toString().c_str());
        return;
      }
      ///////////////////////////////////////////////////////
      PROCESS::loop();
      while (this->running() && this->next()) {
      }
      if (this->running() && this->_loopFunction)
        this->_loopFunction(this);
    }

    void onLoop(const Consumer<Actor *> &loopFunction) { this->_loopFunction = loopFunction; }

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
  };
} // namespace fhatos
#endif
