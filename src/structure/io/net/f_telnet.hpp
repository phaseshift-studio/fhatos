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

#ifndef fhatosf_telnet_hpp
#define fhatosf_telnet_hpp

#include <fhatos.hpp>
//
#include <ESPTelnet.h>
#include <process/actor/actor.hpp>
#include <util/ansi.hpp>
#include <language/binary_obj.hpp>
#include <language/parser.hpp>
#include <language/fluent.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  /////////////////////////////////////////////////////////////////////

#define tthis fTelnet::singleton()
#define TAB "  "
  /////////////////////////////////////////////////////////////////////

  template<typename PROCESS = Thread, typename ROUTER = Router>
  class fTelnet : public Actor<PROCESS, ROUTER> {
  public:
    static fTelnet *singleton() {
      static fTelnet singleton = fTelnet();
      return &singleton;
    }

    explicit fTelnet(const ID &id = Router::mintID("telnet"),
                     const uint16_t port = 23, const bool useAnsi = true)
            : Actor<PROCESS, ROUTER>(id), port(port), useAnsi(useAnsi),
              currentTopic(new ID(id)), previousMessage(nullptr) {
      this->xtelnet = new ESPTelnet();
      this->xtelnet->setLineMode(true);
      this->ansi = new Ansi<ESPTelnet>(this->xtelnet);
    }

    ~fTelnet() override {
      delete this->currentTopic;
      delete this->ansi;
      delete this->xtelnet;
    }

    virtual void setup() override {
      PROCESS::setup();
      ////////// ON CONNECT //////////
      this->xtelnet->onConnect([](const String ipAddress) {
        // LOG_PROCESS(INFO, &T, "Telnet connection made from %s\n",
        // ipAddress.c_str());
        tthis->ansi->println(ANSI_ART);
        tthis->ansi->printf("Telnet server on !m%s!!\n" TAB "Connection from !m%s!!\n" TAB
                            ":help for help menu\n",
                            fWIFI::ip().c_str(), ipAddress.c_str());
        // delete tthis->currentTopic;
        tthis->currentTopic = new ID(tthis->id());
        tthis->printPrompt();
      });

      ////////// ON INPUT //////////
      this->xtelnet->onInputReceived([](String line) {
        LOG(DEBUG, "Telnet input received: %s\n", line.c_str());
        line.trim();
        if (line.isEmpty()) {
          // do nothing
        } else if (line.startsWith("^")) {
          Parser *parser = new Parser(tthis->id());
          parser->parseToFluent(line.substring(1).c_str())->forEach([](const Obj *obj) {
            tthis->ansi->printf("!g==>!!%s\n", obj->toString().c_str());
          });
          delete parser;
        } else if (line.equals("/+")) {
          // todo ??
          tthis->subscribe(*tthis->currentTopic / "+", [](const auto &message) {
            tthis->ansi->printf("[/+]=>!g%s!!\n",
                                message.target.toString().c_str());
          });
        } else if (line.startsWith("<=")) {
          const string payload =
                  (line.length() == 2) ? "" : line.substring(2).c_str();
          BinaryObj<> conversion = BinaryObj<>::interpret(payload);
          LOG(DEBUG, "Telnet publishing: %s::%s\n",
              OTypes.toChars(conversion.pattern()).c_str(),
              conversion.toString().c_str());
          tthis->publish(*tthis->currentTopic, &conversion, TRANSIENT_MESSAGE);
        } else if (line.startsWith("?")) {
          tthis->query(tthis->currentTopic->query(line.c_str()),
                       [](const Message &message) {
                         tthis->ansi->println(message.payload->toStr().toString().c_str());
                       });
        } else if (line.startsWith("=>")) {
          const RESPONSE_CODE _rc =
                  tthis->subscribe(*tthis->currentTopic, [](const Message &message) {
                    /*if (!tthis->previousMessage ||
                        !tthis->previousMessage->first.equals(message.source) ||
                        !tthis->previousMessage->second.equals(message.target)) {
                      if (tthis->previousMessage)
                        delete tthis->previousMessage;
                      tthis->ansi->printf(
                        "[!b%s!!]=!gpublish!![!mretain:%s!!]=>[!b%s!!]\n",
                        message.source.toString().c_str(),
                        FOS_BOOL_STR(message.retain),
                        message.target.toString().c_str());
                      tthis->previousMessage =
                          new Pair<ID, ID>(message.source, message.target);
                    }*/
                    tthis->ansi->println(
                            message.payload->toStr().toString().c_str()); // TODO: ansi off/on
                  });

          tthis->ansi->printf("[%s!!] Subscribed to !b%s!!\n",
                              _rc ? "!RERROR" : "!GOK",
                              tthis->currentTopic->toString().c_str());
        } else if (line.startsWith("=|")) {
          const RESPONSE_CODE _rc = tthis->unsubscribe(*tthis->currentTopic);
          tthis->ansi->printf("[%s!!] Unsubscribed from !b%s!!\n",
                              _rc ? "!RERROR" : "!GOK",
                              tthis->currentTopic->toString().c_str());
        } else if (line.startsWith("/..")) {
          // delete tthis->currentTopic;
          tthis->currentTopic = new ID(tthis->currentTopic->retract());
        } else if (line.startsWith("/") && !line.equals("/")) {
          //  delete tthis->currentTopic;
          tthis->currentTopic =
                  new ID(tthis->currentTopic->extend(line.substring(1).c_str()));
        } else if (line.startsWith(":")) {
          // : global commands
          if (line.equals(":help")) {
            tthis->ansi->print(
                    "!mHelp Menu!!\n"
                    "!gactor commands!!\n" TAB "!b<=!! publish to topic\n" TAB
                    "!b=>!! subscribe to topic\n" TAB
                    "!b=|!! unsubscribe from topic\n"
                    "!g: global commands!!\n" TAB "!b:help!! [help menu]\n" TAB
                    "!b:quit!! [drop connection]\n" TAB
                    "!b:ansi!! [toggle ansi mode]\n"
                    "!g? local commands!!\n");
          } else if (line.equals(":quit")) {
            tthis->xtelnet->disconnectClient();
          } else if (line.equals(":ansi")) {
            //    TCHECK(true, PRINTF("ANSI turned %s\n",
            //                       ((T.useAnsi = !T.useAnsi) ? "ON" :
            //                       "OFF")));
          } else {
            tthis->ansi->printf("[!RERROR!!] Unknown command: %s\n",
                                line.substring(1).c_str());
          }
        } else if (line.startsWith("?")) {
        } else {
          tthis->currentTopic = new ID(ID(line.c_str()).resolve(*tthis->currentTopic));
        }
        tthis->printPrompt();
      });

      ////////// ON DISCONNECT //////////
      this->xtelnet->onDisconnect([](const String ipAddress) {
        // delete tthis->currentTopic;
        tthis->currentTopic = new ID(tthis->id());
        ROUTER::singleton()->unsubscribeSource(tthis->id());
        LOG_PROCESS(INFO, tthis, "Client %s disconnected from Telnet server",
                    ipAddress.c_str());
      });

      const bool success = this->xtelnet->begin(this->port, true);
      LOG_PROCESS(success ? INFO : ERROR, this, "Telnet server initialized on %s:%i",
                  this->id().host().c_str(), this->port);
    }

    void loop() override {
      Actor<PROCESS, ROUTER>::loop();
      tthis->xtelnet->loop();
      if (Serial.available()) {
        tthis->ansi->print(Serial.read());
      }
    }

  private:
    void printPrompt() {
      tthis->ansi->printf("[!b%s!!]", tthis->currentTopic->toString().c_str());
    }

  protected:
    uint16_t port;
    bool useAnsi;
    ESPTelnet *xtelnet;
    Ansi<ESPTelnet> *ansi;
    ID *currentTopic;
    Pair<ID, ID> *previousMessage;
  };
} // namespace fhatos

#endif
