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

#ifndef fhatos_f_ping_hpp
#define fhatos_f_ping_hpp

#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include <ESPping.h>

namespace fhatos {
  template<typename ROUTER = Router>
  struct PingRoutine final : public Coroutine, public Publisher<ROUTER> {
    string ip;
    uint16_t counter = 0;
    uint16_t success = 0;
    float totalTime = 0.0f;

    explicit PingRoutine(const ID &id) : Coroutine(id), Publisher<ROUTER>(this) {
      this->ip = fWIFI::resolve(this->id().path().c_str());
    }

    void setup() override {
      Coroutine::setup();
      this->onQuery(this->id().query("?loop"), [this]() { this->loop(); });
    }

    void stop() override {
      this->unsubscribe(this->id().query("?loop"));
    }

    void loop() override {
      Coroutine::loop();
      ++this->counter;
      char message[100];
      if (Ping.ping(this->id().path().c_str(), 1)) {
        ++this->success;
        this->totalTime += Ping.averageTime();
        sprintf(message, "64 bytes from %s [%s]: icmp_seq=%i time=%.3f ms",
                this->id().path().c_str(), this->ip.c_str(), this->counter,
                this->averageTime());
      } else {
        sprintf(
                message,
                "Request timeout for %s [unknown] icmp_seq %i failure_rate=%.2f%%",
                this->id().path().c_str(), this->counter, this->failureRate());
      }
      this->publish(this->id(), new BinaryObj(message), TRANSIENT_MESSAGE);
    }

    float failureRate() const {
      return (static_cast<float>(counter - success) / static_cast<float>(counter)) * 100.0f;
    }

    float averageTime() const {
      return this->totalTime / (float) this->counter;
    }
  };

  template<typename PROCESS = Thread, typename ROUTER = Router>
  class fPing final : public Actor<PROCESS, ROUTER> {
  public:
    explicit fPing(const ID &id = Router::mintID("ping"))
            : Actor<PROCESS, ROUTER>(id) {
    }

    void stop() override {
      Actor<PROCESS, ROUTER>::stop();
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      this->updateBBS(this->id().query("?"));
      this->onQuery(this->id().extend("+"), {
              {
                      "?start",  [this](const SourceID &, const TargetID &target) {
                Scheduler::singleton()->spawn(new PingRoutine<ROUTER>(target.query("")));
              }
              },
              {
                      "?stop",   [this](const SourceID &, const TargetID &target) {
                Scheduler::singleton()->kill(target.query(""));
              }
              },
              {
                      "default", [](const SourceID &source, const TargetID &target) {
                LOG(ERROR, "Unknown query by !m%s!! of !m%s!!: !y%s!!\n",
                    source.toString().c_str(),
                    target.query("").toString().c_str(),
                    target.query().c_str());
              }
              }
      }, [this](const SourceID &, const TargetID &) {
        this->updateBBS(this->id().query("?"));
      });
    }

    void loop() override {
      Actor<PROCESS, ROUTER>::loop();
      List < Process * > *results = Scheduler::singleton()->find(this->id().extend("+"));
      for (Process *child: *results) {
        this->publish(child->id().query("?loop"), "", TRANSIENT_MESSAGE);
      }
      delete results;
      delay(1000);
    }

  protected:
    void updateBBS(const ID &queryId) {
      string message = string("!m!_").append(this->id().toString()).append("!!\n");
      const List<Process *> *results = Scheduler::singleton()->find(
              this->id().extend("+"));
      for (Process *pinger: *results) {
        char line[256];
        sprintf(line,
                FOS_TAB
        "!b\\_!!!r%s!!\n"
        FOS_TAB
        ""
        FOS_TAB
        ""
        FOS_TAB
        "[!gip!!:!b%s!!][!gcounter!!:!b%i!!][!gsuccess!!:!b%.2f%%"
        "!!][!gaverage!!:!b%.2fms!!]\n",
                pinger->id().lastSegment().c_str(), "a", 1, 10.0f, 2.0f); /* pinger->ip.c_str(),
                pinger->counter, 100.0f - pinger->failureRate(),
                pinger->averageTime());*/
        message = message + line;
      }
      delete results;
      this->publish(queryId, message, RETAIN_MESSAGE);
    }
  };
} // namespace fhatos

#endif
