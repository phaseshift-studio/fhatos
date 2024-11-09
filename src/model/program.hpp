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
#ifndef fhatos_program_hpp
#define fhatos_program_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <process/process.hpp>
#include <structure/structure.hpp>
#include <util/options.hpp>

namespace fhatos {
  class Program;
  using Program_p = ptr<Program>;

  class Program final {
  protected:
    Structure_p structure_;
    Process_p process_;

  public:
    explicit Program(const Structure_p &structure,
                     const Process_p &process) : structure_(structure), process_(process) {
    }

    /*void publish(
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
      router()->route_subscription(subscription_p(*this->vid(), pattern, Insts::to_bcode(on_recv)));
      .executeAtSource(this)));
    }


    void unsubscribe(const Pattern_p &pattern = p_p("#")) {
      this->should_be_active();
      router()->route_unsubscribe(this->vid(), pattern);
    }*/

    bool active() const {
      return this->structure_->available() && this->process_->running;
    }

    //////////////////////////////////////////////////// SETUP
    void setup() const {
      //this->process_->setup();
      LOG(INFO, FURI_WRAP FURI_WRAP " !mactor!! activated\n", this->structure_->pattern()->toString().c_str(),
          this->process_->vid()->toString().c_str());
    }

    //////////////////////////////////////////////////// STOP
    void stop() const {
      this->structure_->stop();
      this->process_->stop();
    }

    //////////////////////////////////////////////////// LOOP
    void loop() const {
      this->process_->loop();
      this->structure_->loop();
    }

    string toString() const {
      return string("!mprogram!!!g[!cid!m=>!g<!y") + this->structure_->pattern()->toString() +
             "!g>!m,!cpattern!m=>!g<!y" + this->
             process_->vid()->toString() +
             "!g>]!!";
    }

  protected:
    void should_be_active() const {
      if (!this->active())
        throw fError(FURI_WRAP " is not active (spawned and attached)", this->toString().c_str());
    }
  };
} // namespace fhatos
#endif
