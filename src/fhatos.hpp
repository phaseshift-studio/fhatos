#ifndef fhatos_hpp
#define fhatos_hpp

/***
 *            PhaseShift Studio Presents
 * <`--'>____  ______  __  __  ______  ______  ______  ______
 * /. .  `'  \/\  ___\/\ \_\ \/\  __ \/\__  _\/\  __ \/\  ___\
 *('')  ,     @ \  __\\ \  __ \ \  __ \/_/\ \/\ \ \/\ \ \___  \
 * `-._,     / \ \_\   \ \_\ \_\ \_\ \_\ \ \_\ \ \_____\/\_____\
 *    )-)_/-(>  \/_/    \/_/\/_/\/_/\/_/  \/_/  \/_____/\/_____/
 *                                    A Dogturd Stynx Production
 */

#ifndef ESP32
#ifndef ESP8266
#ifndef NATIVE
#error FhatOS requires ESP32 or ESP8266 architectures (or NATIVE for testing)
#endif
#endif
#endif

// Arduino programming framework
#ifndef NATIVE
#include <Arduino.h>
#include <util/ansi.hpp>
#endif

// C++ standard template library common data structures
#include <deque>
#include <exception>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>

namespace fhatos {
  /*static const char *ANSI_ART =
      "            PhaseShift Studio Presents\n"
      " <`--'>____  ______  __  __  ______  ______  ______  ______     \n"
      " /. .  `'  \\/\\  ___\\/\\ \\_\\ \\/\\  __ \\/\\__  _\\/\\  __ \\/\\  "
      "___\\    \n"
      "('')  ,     @ \\  __\\\\ \\  __ \\ \\  __ \\/_/\\ \\/\\ \\ \\/\\ \\ \\___ "
      " \\ \n"
      " `-._,     / \\ \\_\\   \\ \\_\\ \\_\\ \\_\\ \\_\\ \\ \\_\\ \\ "
      "\\_____\\/\\_____\\  \n"
      "    )-)_/-(>  \\/_/    \\/_/\\/_/\\/_/\\/_/  \\/_/  \\/_____/\\/_____/  \n"
      "                                    A Dogturd Stynx Production  \n";*/

  static const char *ANSI_ART =
      "!r            !_PhaseShift Studio Presents!! \n"
      "!m <`--'>____!g  ______ __  __  ______  ______  !b______  ______!! \n"
      "!m /. .  `'  \\!g/\\  ___/\\ \\_\\ \\/\\  __ \\/\\__  _\\!b/\\  __ \\/\\  "
      "___\\!! \n"
      "!m('')  ,     !M@!g \\  __\\ \\  __ \\ \\  __ \\/_/\\ \\/!b\\ \\ \\_\\ \\ "
      "\\___  \\!! \n"
      "!m `-._,     /!g \\ \\_\\  \\ \\_\\ \\_\\ \\_\\ \\_\\ \\ \\_\\ !b\\ "
      "\\_____\\/\\_____\\!! \n"
      "!m    )-)_/-(>!g  \\/_/   \\/_/\\/_/\\/_/\\/_/  \\/_/  "
      "!b\\/_____/\\/_____/!! \n"
      "!r                                   !_A Dogturd Stynx Production!! \n";

  ////////////////////
  /// LAMBDA TYPES ///
  ////////////////////
  typedef void (*VoidPtr)();

  typedef std::function<void(void)> Void0;
  template<typename A>
  using Consumer = std::function<void(A)>;
  template<typename A, typename B>
  using BiConsumer = std::function<void(A, B)>;
  template<typename A, typename B, typename C>
  using TriConsumer = std::function<void(A, B, C)>;
  template<typename A, typename B, typename C, typename D>
  using QuadConsumer = std::function<void(A, B, C, D)>;
  template<typename A>
  using Supplier = std::function<A()>;
  template<typename A, typename B>
  using Function = std::function<B(A)>;
  template<typename A, typename B, typename C>
  using BiFunction = std::function<C(A, B)>;
  template<typename A, typename B, typename C, typename D>
  using TriFunction = std::function<D(A, B, C)>;
  template<typename A>
  using Predicate = std::function<bool(A)>;

  ///////////////////////
  /// CONTAINER TYPES ///
  ///////////////////////
  template<typename A>
  using Option = std::optional<A>;
  template<typename A>
  using List = std::list<A>;
  template<typename A>
  using Set = std::set<A>;
  template<typename A>
  using Queue = std::queue<A>;
  template<typename A>
  using Deque = std::deque<A>;
  template<typename K, typename V>
  using Pair = std::pair<K, V>;
  template<typename A, typename B, typename C>
  using Triple = std::tuple<A, B, C>;
  template<typename K, typename V>
  using Map = std::map<K, V>;

  ///////////////////////
  /// EXCEPTION TYPES ///
  ///////////////////////
  class fError : public std::exception {
  protected:
    char *_message;

  public:
    explicit fError(const char *format, ...) {
      va_list arg;
      va_start(arg, format);
      char temp[64];
      _message = temp;
      size_t len = vsnprintf(temp, sizeof(temp), format, arg);
      va_end(arg);
      if (len > sizeof(temp) - 1) {
        _message = new(std::nothrow) char[len + 1];
        if (!_message) {
          return;
        }
        va_start(arg, format);
        vsnprintf(_message, len + 1, format, arg);
        va_end(arg);
      }
      if (_message != temp) {
        delete[] _message;
      }
    };

    ~fError() override { delete _message; }

    [[nodiscard]] const char *
    what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
      return this->_message;
    }
  };

  ////////////
  // MACROS //
  ////////////
  enum LOG_TYPE { DEBUG = 0, INFO = 1, ERROR = 2, NONE = 3 };

  inline const char *LOG_TYPE_c_str(const LOG_TYPE type) {
    switch (type) {
      case DEBUG:
        return "DEBUG";
      case INFO:
        return "INFO";
      case ERROR:
        return "ERROR";
      default:
        return "NONE";
    };
  }

#define FOS_TAB "  "
#define FOS_I2C_ADDR_STR "0x%x/%i"
#define FOS_I2C_ADDR(a) a, a
#define FOS_BOOL_STR(a) a ? "true" : "false"
#define CONCAT(a, b) XCONCAT(a, b)
#define XCONCAT(a, b) a##b
#define STR(a) XSTR(a)
#define XSTR(a) #a
#define FSTR(a) F(STR(a))
#define LOG(logtype, format, ...) MAIN_LOG((logtype), F(format), ##__VA_ARGS__)
#define LOG_EXCEPTION(ex) LOG(ERROR, ex.what())
#define LOG_TASK(logtype, process, format, ...)                                \
  LOG((logtype), (String("[!M%s!!] ") + (format) + "\n").c_str(),                  \
      (process)->id().toString().c_str(), ##__VA_ARGS__)
#define LOG_SUBSCRIBE(rc, subscription)                                        \
  LOG((rc == OK ? INFO : ERROR),                                               \
      "[%s][!b%s!!]=!gsubscribe!m[qos:%i]!!=>[!b%s!!]\n",                      \
      (String(rc == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(), \
      (subscription).source.toString().c_str(), (uint8_t)(subscription).qos,   \
      (subscription).pattern.toString().c_str())
#define LOG_UNSUBSCRIBE(rc, source, pattern)                                   \
  LOG((rc == OK ? INFO : ERROR), "[%s][!b%s!!]=!gunsubscribe!!=>[!b%s!!]\n",   \
      (String(rc == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(), \
      ((source).toString().c_str()),                                           \
      nullptr == (pattern) ? "ALL" : (pattern)->toString().c_str())
#define LOG_PUBLISH(rc, message)                                               \
  LOG((rc == OK ? INFO : ERROR),                                               \
      "[%s][!b%s!!]=!gpublish!m[retain:%s]!!=!r%s!!=>[!b%s!!]\n",              \
      (String(rc == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(), \
      (message.source.toString().c_str()), (FOS_BOOL_STR(message.retain)),      \
      (message.payload.toString().c_str()),                                    \
      (message.target.toString().c_str()))
#define LOG_RECEIVE(rc, subscription, message)                                 \
  LOG((rc == OK ? INFO : ERROR),                                               \
      (((subscription).pattern.equals((message).target))                       \
           ? "[%s][!b%s!!]<=!greceive!m[pattern|target:%s]!!=!r%s!!=[!b%s!!]"  \
             "\n"                                                              \
           : "[%s][!b%s!!]<=!greceive!m[pattern:%s][target:%s]!!=!r%s!!=[!b%"  \
             "s!!]\n"),                                                        \
      (String(rc == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(), \
      (subscription.source.toString().c_str()),                                \
      (subscription.pattern.toString().c_str()),                               \
      (subscription.pattern.equals(message.target))                            \
          ? (message.payload.toString().c_str())                               \
          : (message.target.toString().c_str()),                               \
      (subscription.pattern.equals(message.target))                            \
          ? (message.source.toString().c_str())                                \
          : (message.payload.toString)().c_str(),                              \
      (message.source.toString().c_str()))

  ////////////////////////////
  // ARCHITECTURE LIBRARIES //
  ////////////////////////////

#if defined(ESP32)
#define FOS_PROCESS(proc) <process/esp32/proc>
#elif defined(ESP8266)
#define FOS_PROCESS(proc) <process/esp8266/proc>
#elif defined(NATIVE)
#define FOS_PROCESS
#else
#error "Unknown architecture."
#endif
#define FOS_ROUTER(rout) <process/router/rout>
#define FOS_MODULE(modu) <structure/modu>

  ///////////////////
  // !!TO REMOVE!! //
  ///////////////////

  template<typename K, typename V>
  static String mapString(const Map<K, V> map) {
    String temp = "[\n";
    for (const auto &kv: map) {
      temp = temp + kv.first + ":" + kv.second->toString() + "\n";
    }
    temp = temp + "]";
    return temp;
  }

  static void MAIN_LOG(const LOG_TYPE type, const String &format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char *buffer = temp;
    const size_t len = vsnprintf(temp, sizeof(temp), format.c_str(), arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
      buffer = new(std::nothrow) char[len + 1];
      if (!buffer) {
        return;
      }
      va_start(arg, format);
      vsnprintf(buffer, len + 1, format.c_str(), arg);
      va_end(arg);
    }
    static auto ansi = Ansi(&::Serial);
    if (type == NONE)
      ansi.print("");
    else if (type == ERROR)
      ansi.print("!r[ERROR]!!  ");
    else if (type == INFO)
      ansi.print("!g[INFO]!!  ");
    else
      ansi.print("!y[DEBUG]!!  ");
    ansi.print(buffer);
    if (buffer != temp) {
      delete[] buffer;
    }
  }
} // namespace fhatos
#endif
