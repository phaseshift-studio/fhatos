#ifndef fhatos_kernel_f_telnet_hpp
#define fhatos_kernel_f_telnet_hpp

#include <fhatos.hpp>
//
#include <ESPTelnet.h>
#include <kernel/furi.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/util/ansi.hpp>
#include <kernel/util/string_stream.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

/////////////////////////////////////////////////////////////////////

#define tthis fTelnet::singleton()
#define TAB "  "
/////////////////////////////////////////////////////////////////////

template <typename PROCESS = Thread, typename ROUTER = LocalRouter<>>
class fTelnet : public Actor<PROCESS, ROUTER> {

public:
  static fTelnet *singleton() {
    static fTelnet singleton = fTelnet();
    return &singleton;
  }

  explicit fTelnet(const ID &id = fWIFI::idFromIP("telnet"),
                   const uint16_t port = 23, const bool useAnsi = true)
      : Actor<PROCESS, ROUTER>(id), port(port), useAnsi(useAnsi),
        currentTopic(new ID(id)) {
    this->xtelnet = new ESPTelnet();
    this->xtelnet->setLineMode(true);
    this->ansi = new Ansi<ESPTelnet>(this->xtelnet);
  }

  ~fTelnet() {
    delete this->currentTopic;
    delete this->ansi;
    delete this->xtelnet;
  }

  void setup() override {
    ////////// ON CONNECT //////////
    this->xtelnet->onConnect([](const String ipAddress) {
      // LOG_TASK(INFO, &T, "Telnet connection made from %s\n",
      // ipAddress.c_str());
      tthis->ansi->println(ANSI_ART);
      tthis->ansi->printf("Telnet server on !m%s!!\n" TAB
                          ":help for help menu\n",
                          fWIFI::ip().toString().c_str());
      tthis->currentTopic = new ID(tthis->id());
      tthis->printPrompt();
    });

    ////////// ON INPUT //////////
    this->xtelnet->onInputReceived([](String line) {
      LOG(DEBUG, "Telnet input received: %s\n", line.c_str());
      line.trim();
      if (line.isEmpty()) {
        // do nothing
      } else if (line.equals("/+")) {
        // todo ??
        tthis->subscribe(*tthis->currentTopic / "+", [](const auto &message) {
          tthis->ansi->printf("[/+]=>!g%s!!\n",
                              message.target.toString().c_str());
        });
      } else if (line.startsWith("<=")) {
        const String payload = (line.length() == 2) ? "" : line.substring(2);
        Payload conversion = Payload::interpret(payload);
        tthis->publish(*tthis->currentTopic, conversion, TRANSIENT_MESSAGE);
        LOG(DEBUG, "Telnet publishing: %s::%s\n",
            MTYPE_NAMES.at(conversion.type).c_str(),
            conversion.toString().c_str());
      } else if (line.startsWith("?")) {
        tthis->query(tthis->currentTopic->query(line.substring(1)),
                     [](const Message &message) {
                       tthis->ansi->println(message.payload.toString().c_str());
                     });
      } else if (line.startsWith("=>")) {
        const RESPONSE_CODE _rc =
            tthis->subscribe(*tthis->currentTopic, [](const Message message) {
              /*if (!tthis->previousMessage ||
                  !tthis->previousMessage->first.equals(message.source) ||
                  !tthis->previousMessage->second.equals(message.target)) {
                if (tthis->previousMessage)
                  delete tthis->previousMessage;
                tthis->ansi->printf(
                    "[!b%s!!]=!gpublish!![!mretain:%s!!]=>[!b%s!!]\n",
                    message.source.toString().c_str(),
                    FP_BOOL_STR(message.retain),
                    message.target.toString().c_str());
                tthis->previousMessage =
                    new Pair<ID, ID>(message.source, message.target);*/
              //}

              tthis->xtelnet->println(
                  message.payload.toString().c_str()); // TODO: ansi off/on
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
        tthis->currentTopic = new ID(tthis->currentTopic->retract());
      } else if (line.startsWith("/") && !line.equals("/")) {
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
        tthis->currentTopic = new ID(ID(line).resolve(*tthis->currentTopic));
      }
      tthis->printPrompt();
    });

    ////////// ON DISCONNECT //////////
    this->xtelnet->onDisconnect([](const String ipAddress) {
      tthis->currentTopic = new ID(tthis->id());
      ROUTER::singleton()->unsubscribeSource(tthis->id());
      LOG_TASK(INFO, tthis, "Client %s disconnected from Telnet server\n",
               ipAddress.c_str());
    });

    bool success = this->xtelnet->begin(this->port, true);
    LOG_TASK(success ? INFO : ERROR, this, "Initializing Telnet server %s\n",
             this->id().toString().c_str());
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

} // namespace fhatos::kernel

#endif