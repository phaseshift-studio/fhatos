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
#ifndef fhatos_obj_process_hpp
#define fhatos_obj_process_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/router.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  class ThreadObj : public Thread {
  public:
    explicit ThreadObj(const ID &id): Thread(id) {
    }

    void setup() override {
      try {
        Thread::setup();
        router()->route_subscription(subscription_p(*this->id(), *this->id(), QoS::_1, Insts::to_bcode(
                                                      [this](const Message_p &message) {
                                                        if (this->running()) {
                                                          if (message->retain && message->payload->is_noobj()) {
                                                            router()->route_unsubscribe(this->id(), p_p(*this->id()));
                                                            this->stop();
                                                          }
                                                        }
                                                      })));
        const BCode_p setup_bcode = router()->read(id_p(this->id()->resolve(":setup")));
        LOG_PROCESS(DEBUG, this, "Executing setup()-bcode: %s\n", setup_bcode->toString().c_str());
        process(setup_bcode, uri(this->id()));
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
        this->stop();
      }
    }

    void loop() override {
      try {
        if (this->running_.load()) {
          const BCode_p loop_bcode = router()->read(id_p(this->id()->resolve(":loop")));
          process(loop_bcode, uri(this->id()));
        }
      } catch (const fError &error) {
        LOG_EXCEPTION(error);
        this->stop();
      }
    }

    void stop() override {
      const BCode_p stop_bcode = router()->read(id_p(this->id()->resolve(":stop")));
      LOG_PROCESS(DEBUG, this, "Executing stop()-bcode: %s\n", stop_bcode->toString().c_str());
      process(stop_bcode, uri(this->id()));
      router()->route_unsubscribe(this->id(), p_p(*this->id()));
      Thread::stop();
    }
  };

  inline void load_threader() {
    THREAD_SPAWNER = [](const ID_p &thread_id) {
      scheduler()->spawn(std::make_shared<ThreadObj>(ID(*thread_id)));
    };
  }
} // namespace fhatos
#endif
