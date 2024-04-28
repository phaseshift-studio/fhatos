#ifndef fhatos_kernel__telnet_hpp
#define fhatos_kernel__telnet_hpp

#include <ESPTelnet.h>
#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/structure/structure.hpp>
#include <kernel/util/ansi2.hpp>
#include <kernel/util/string_stream.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

/////////////////////////////////////////////////////////////////////

#define tthis Telnet::singleton()
#define T (*Telnet::singleton())
#define TSerial (*(Telnet::singleton()->xtelnet))
#define TAnsiOn(x)                                                             \
  if (T.useAnsi) {                                                             \
    TSerial.print(x);                                                          \
  }
#define TAnsiOff() TAnsiOn("\033[0m")
#define TAnsi(x, y)                                                            \
  TAnsiOn(x) y;                                                                \
  TAnsiOff()
#define PRINTF(x, ...) TSerial.printf(x, ##__VA_ARGS__)
#define PRINT(x) TSerial.print(x)
#define PRINTLN(x) TSerial.println(x)
#define TAB "  "
#define TCHECK(x, y)                                                           \
  if (!x) {                                                                    \
    TAnsi(ansi.setFG(ANSI_GREEN), PRINT("[OK] "))                              \
  } else {                                                                     \
    TAnsi(ansi.setFG(ANSI_RED), PRINT("[ERROR] "))                             \
  }                                                                            \
  y;

/////////////////////////////////////////////////////////////////////

template <typename PROCESS = Thread, typename MESSAGE = StringMessage,
          typename ROUTER = LocalRouter<MESSAGE>>
class Telnet : public Actor<PROCESS, MESSAGE, ROUTER> {

public:
  static Telnet *singleton() {
    static Telnet singleton = Telnet();
    return &singleton;
  }

  Telnet(const ID &id = WIFI::idFromIP("telnet"), const uint16_t port = 23,
         const bool useAnsi = true)
      : Actor<PROCESS, MESSAGE, ROUTER>(id), port(port), useAnsi(useAnsi),
        currentTopic(new ID(id)) {
    this->xtelnet = new ESPTelnet();
    this->xtelnet->setLineMode(true);
    this->ansi = new Ansi2<ESPTelnet>(this->xtelnet);
  }

  ~Telnet() {
    delete this->currentTopic;
    delete this->ansi;
  }

  virtual void setup() override {
    Actor<PROCESS, MESSAGE, ROUTER>::setup();
    ////////// ON CONNECT //////////
    this->xtelnet->onConnect([](const String ipAddress) {
      // LOG_TASK(INFO, &T, "Telnet connection made from %s\n",
      // ipAddress.c_str());
      tthis->ansi->println(ANSI_ART);
      tthis->ansi->printf("Telnet server on !m%s!!\n" TAB
                          ":help for help menu\n",
                          WIFI::singleton()->ip().toString().c_str());
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
        tthis->subscribe(
            *tthis->currentTopic / "+", [](const MESSAGE &message) {
              tthis->ansi->printf("[/+]=>!g%s!!\n",
                                  message.target.toString().c_str());
            });
      } else if (line.startsWith("<=")) {
        String payload = line.substring(2);
        payload.trim();
        tthis->publish(*tthis->currentTopic, payload, TRANSIENT_MESSAGE);
      } else if (line.startsWith("=>")) {
        RESPONSE_CODE __rc =
            tthis->subscribe(*tthis->currentTopic, [](const MESSAGE &message) {
              tthis->ansi->printf("[!b%s!!]=!gpublish!![!mretain:%s!!]=>",
                                  message.source.toString().c_str(),
                                  FP_BOOL_STR(message.retain));
              tthis->xtelnet->println(message.payloadString().c_str()); // TODO: ansi off/on
            });
        tthis->ansi->printf("[%s!!] Subscribed to !b%s!!\n",
                            __rc ? "!RERROR" : "!GOK",
                            tthis->currentTopic->toString().c_str());
      } else if (line.startsWith("=|")) {
        RESPONSE_CODE __rc = tthis->unsubscribe(*tthis->currentTopic);
        tthis->ansi->printf("[%s!!] Unsubscribed from !b%s!!\n",
                            __rc ? "!RERROR" : "!GOK",
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
          //                       ((T.useAnsi = !T.useAnsi) ? "ON" : "OFF")));
        } else {
          tthis->ansi->printf("[!RERROR!!] Unknown command: %s\n",
                              line.substring(1).c_str());
        }
      } else if (line.startsWith("?")) {
        // ? current topic commands (beyond publish/subscribe/unsubscribe)
      } else {
        tthis->currentTopic = new ID(line);
      }
      tthis->printPrompt();
    });

    ////////// ON DISCONNECT //////////
    this->xtelnet->onDisconnect([](String ipAddress) {
      tthis->currentTopic = new ID(tthis->id());
      ROUTER::singleton()->unsubscribeSource(T.id());
      LOG_TASK(INFO, &T, "Client %s disconnected from Telnet server\n",
               ipAddress.c_str());
    });

    bool success = this->xtelnet->begin(this->port, true);
    LOG_TASK(success ? INFO : ERROR, this, "Initializing Telnet server %s\n",
             this->id().toString().c_str());
  }

  virtual void loop() override {
    Actor<PROCESS, MESSAGE, ROUTER>::loop();
    this->xtelnet->loop();
    if (::Serial.available()) {
      this->ansi->print(::Serial.read());
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
  Ansi2<ESPTelnet> *ansi;
  ID *currentTopic;
};

} // namespace fhatos::kernel

#endif