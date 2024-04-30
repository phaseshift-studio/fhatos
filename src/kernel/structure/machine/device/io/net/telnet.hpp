#ifndef fhatos_kernel_telnet_hpp
#define fhatos_kernel_telnet_hpp

#include <ESPTelnet.h>
#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/structure/structure.hpp>
#include <kernel/util/ansi.hpp>
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

template <typename PROCESS = Thread, typename PAYLOAD = String,
          typename ROUTER = LocalRouter<Message<PAYLOAD>>>
class Telnet : public Actor<PROCESS, PAYLOAD, ROUTER> {

public:
  static Telnet *singleton() {
    static Telnet singleton = Telnet();
    return &singleton;
  }

  explicit Telnet(const ID &id = WIFI::idFromIP("telnet"),
                  const uint16_t port = 23, const bool useAnsi = true)
      : Actor<PROCESS, PAYLOAD, ROUTER>(id), port(port), useAnsi(useAnsi),
        currentTopic(new ID(id)) {
    this->xtelnet = new ESPTelnet();
    this->xtelnet->setLineMode(true);
    this->ansi = new Ansi<ESPTelnet>(this->xtelnet);
  }

  ~Telnet() {
    delete this->currentTopic;
    delete this->ansi;
  }

  void setup() override {
    Actor<PROCESS, PAYLOAD, ROUTER>::setup();
    ////////// ON CONNECT //////////
    this->xtelnet->onConnect([](const String ipAddress) {
      // LOG_TASK(INFO, &T, "Telnet connection made from %s\n",
      // ipAddress.c_str());
      tthis->ansi->println(ANSI_ART);
      tthis->ansi->printf("Telnet server on !m%s!!\n" TAB
                          ":help for help menu\n",
                          WIFI::ip().toString().c_str());
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
            *tthis->currentTopic / "+", [](const Message<PAYLOAD> &message) {
              tthis->ansi->printf("[/+]=>!g%s!!\n",
                                  message.target.toString().c_str());
            });
      } else if (line.startsWith("<=")) {
        String payload = line.length() == 2 ? "" : line.substring(2);
        payload.trim();
        tthis->publish(*tthis->currentTopic, payload, TRANSIENT_MESSAGE);
      } else if (line.startsWith("=>") || line.equals("?")) {
        RESPONSE_CODE _rc = tthis->subscribe(
            *tthis->currentTopic, [](const Message<PAYLOAD> &message) {
              tthis->ansi->printf("[!b%s!!]=!gpublish!![!mretain:%s!!]=>",
                                  message.source.toString().c_str(),
                                  FP_BOOL_STR(message.retain));
              tthis->xtelnet->println();
              tthis->xtelnet->println(
                  message.payloadString().c_str()); // TODO: ansi off/on
            });
        if (line.equals("?")) {
          yield();
          tthis->unsubscribe(*tthis->currentTopic);
        } else {
          tthis->ansi->printf("[%s!!] Subscribed to !b%s!!\n",
                              _rc ? "!RERROR" : "!GOK",
                              tthis->currentTopic->toString().c_str());
        }
      } else if (line.startsWith("=|")) {
        RESPONSE_CODE _rc = tthis->unsubscribe(*tthis->currentTopic);
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
          //                       ((T.useAnsi = !T.useAnsi) ? "ON" : "OFF")));
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
      ROUTER::singleton()->unsubscribeSource(T.id());
      LOG_TASK(INFO, &T, "Client %s disconnected from Telnet server\n",
               ipAddress.c_str());
    });

    bool success = this->xtelnet->begin(this->port, true);
    LOG_TASK(success ? INFO : ERROR, this, "Initializing Telnet server %s\n",
             this->id().toString().c_str());
  }

  void loop() override {
    Actor<PROCESS, PAYLOAD, ROUTER>::loop();
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
  ESPTelnet *xtelnet{};
  Ansi<ESPTelnet> *ansi{};
  ID *currentTopic;
};

} // namespace fhatos::kernel

#endif