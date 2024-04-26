#ifndef fhatos_kernel__telnet_hpp
#define fhatos_kernel__telnet_hpp

#include <ESPTelnet.h>
#include <EscapeCodes.h>
#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/process/actor/message.hpp>
#include <kernel/structure/structure.hpp>
#include FOS_PROCESS(thread.hpp)

namespace fhatos::kernel {

/////////////////////////////////////////////////////////////////////

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
      : Actor<PROCESS, MESSAGE, ROUTER>(id), port(port), useAnsi(useAnsi) {
    this->xtelnet = new ESPTelnet();
    this->xtelnet->setLineMode(true);
    this->currentTopic =  ID(id);
  }

  virtual void setup() override {
    Actor<PROCESS, MESSAGE, ROUTER>::setup();
    ////////// ON CONNECT //////////
    this->xtelnet->onConnect([](const String ipAddress) {
      LOG_TASK(INFO, &T, "Telnet connection made from %s\n", ipAddress.c_str());
      PRINTLN(ANSI_ART);
      PRINTF("Telnet server on %s\n" TAB ":help for help menu\n",
             WIFI::singleton()->ip().toString().c_str());
      T.currentTopic =  ID(T.id());
      T.printPrompt();
    });

    ////////// ON INPUT //////////
    this->xtelnet->onInputReceived([](String line) {
      LOG(DEBUG, "Telnet input received: %s\n", line.c_str());
      line.trim();
      if (line.isEmpty()) {
        // do nothing
      } else if (line.equals("/+")) {
        T.subscribe(T.currentTopic / "+", [](const MESSAGE &message) {
          PRINT("\n[");
          TAnsi(ansi.setFG(ANSI_GREEN), PRINT("/+"));
          PRINT("]=>");
          TAnsi(ansi.setFG(ANSI_GREEN),
                PRINTLN(message.target.toString().c_str()));
        });
      } else if (line.startsWith("<=")) {
        String payload = line.substring(2);
        payload.trim();
        T.publish(T.currentTopic, payload, false);
      } else if (line.startsWith("=>")) {
        TCHECK(T.subscribe(T.currentTopic,
                           [](const MESSAGE &message) {
                             PRINT("\n[");
                             TAnsi(ansi.setFG(ANSI_GREEN),
                                   PRINTF(message.target.toString().c_str()));
                             PRINT("]=>");
                             TAnsi(ansi.setFG(ANSI_GREEN),
                                   PRINTLN(message.payloadString().c_str()));
                           }),
               PRINTF("Subscribed to %s\n", T.currentTopic.toString().c_str()));
      } else if (line.startsWith("=|")) {
        TCHECK(T.unsubscribe(T.currentTopic),
               PRINTF("Unsubscribed from %s\n",
                      T.currentTopic.toString().c_str()));
      } else if (line.startsWith("/..")) {
        T.currentTopic = T.currentTopic.retract();
      } else if (line.startsWith("/") && !line.equals("/")) {
        T.currentTopic = T.currentTopic.extend(line.substring(1).c_str());
      } else if (line.startsWith(":")) {
        // : global commands
        if (line.equals(":help")) {
          TAnsi(
              ansi.setFG(ANSI_RED),
              PRINT("Help Menu\n"
                    "mqtt commands\n" TAB "<= publish to topic\n" TAB
                    "=> subscribe to topic\n" TAB "=| unsubscribe from topic\n"
                    ": global commands\n" TAB ":help [help menu]\n" TAB
                    ":quit [drop connection]\n" TAB ":ansi [toggle ansi mode]\n"
                    "? local commands\n"));
        } else if (line.equals(":quit")) {
          T.xtelnet->disconnectClient();
        } else if (line.equals(":ansi")) {
          TCHECK(true, PRINTF("ANSI turned %s\n",
                              ((T.useAnsi = !T.useAnsi) ? "ON" : "OFF")));
        } else {
          TCHECK(false, PRINTF("Unknown command: %s\n", line.substring(1)));
        }
      } else if (line.startsWith("?")) {
        // ? current topic commands (beyond publish/subscribe/unsubscribe)
      } else {
        T.currentTopic = ID(line);
      }
      T.printPrompt();
    });

    ////////// ON DISCONNECT //////////
    this->xtelnet->onDisconnect([](String ipAddress) {
      T.currentTopic = T.id();
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
      this->xtelnet->print(::Serial.read());
    }
  }

private:
  void printPrompt() {
    PRINT("[");
    TAnsi(ansi.setFG(ANSI_MAGENTA), PRINT(T.currentTopic.toString().c_str()));
    PRINT("]");
  }

protected:
  uint16_t port;
  bool useAnsi;
  static EscapeCodes ansi;
  ESPTelnet *xtelnet;
  ID currentTopic;
};

} // namespace fhatos::kernel

#endif