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

#ifndef fhatos_hpp
#define fhatos_hpp

/***
 *            PhaseShift Studio Presents
 * <`--'>____  ______  __  __  ______  ______  ______  ______
 * /. .  `'  \/\  ___\/\ \_\ \/\  __ \/\__  _\/\  __ \/\  ___\
 *(`')  ,     @ \  __\\ \  __ \ \  __ \/_/\ \/\ \ \/\ \ \___  \
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

#ifndef NATIVE
#include <Arduino.h>
#endif
#include <util/ansi.hpp>
#include <util/fhat_error.hpp>

// C++ standard template library common data structures
#include <any>
#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <queue>
#include <set>
#include <string>
#include <tsl/ordered_map.h>
#include <util/options.hpp>


namespace fhatos {
  static const char *ANSI_ART = "!r            !_PhaseShift Studio Presents!! \n"
                                "!m <`--'>____!g  ______ __  __  ______  ______  !b______  ______!! \n"
                                "!m /. .  `'  \\!g/\\  ___/\\ \\_\\ \\/\\  __ \\/\\__  _\\!b/\\  __ \\/\\  "
                                "___\\!! \n"
                                "!m(`')  ,     !M@!g \\  __\\ \\  __ \\ \\  __ \\/_/\\ \\/!b\\ \\ \\_\\ \\ "
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

  using Runnable = std::function<void()>;
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
  //
  template<typename A>
  using IdentityFunction = Function<A, A>;
  template<typename A>
  static IdentityFunction<A> id_f() {
    return [](const A a) { return a; };
  }
  ///////////////////////
  /// CONTAINER TYPES ///
  ///////////////////////
  using Any = std::any;
  template<typename A>
  using Option = std::optional<A>;
  template<typename A>
  using List = std::vector<A>;
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
  template<typename A, typename B, typename C, typename D>
  using Quadruple = std::tuple<A, B, C, D>;
  template<typename K, typename V, typename C = std::less<>, typename A = std::allocator<std::pair<const K, V>>>
  using Map = std::map<K, V, C, A>;
  template<typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<K>>
  using OrderedMap = tsl::ordered_map<K, V, H, E>;

  template<typename A>
  using ptr = std::shared_ptr<A>;

  template<typename A>
  ptr<A> share(const A a) {
    return std::make_shared<A>(a);
  }

  using string = std::string;
  using fbyte = uint8_t;
  using uint = unsigned int;


  ////////////
  // MACROS //
  ////////////
  enum LOG_TYPE { NONE = 0, DEBUG_MORE = 1, DEBUG = 2, INFO = 3, ERROR = 4 };

#define FOS_TAB_1 " "
#define FOS_TAB_2 "  "
#define FOS_TAB_3 "   "
#define FOS_TAB_4 "    "
#define FOS_TAB_5 "     "
#define FOS_TAB_6 "      "
#define FOS_TAB_7 "       "
#define FOS_TAB_8 "        "
#define FOS_TAB "  "
#define FOS_I2C_ADDR_STR "0x%x/%i"
#define FOS_I2C_ADDR(a) a, a
#define FOS_BOOL_STR(a) a ? "true" : "false"
#define CONCAT(a, b) XCONCAT(a, b)
#define XCONCAT(a, b) a##b
#define STR(a) XSTR(a)
#define XSTR(a) #a
#define FSTR(a) STR(a)
#define FOS_BYTES_MB_STR "%i (%.2f MB)"
#define FOS_BYTES_MB(a) a, (((float) a) / (1024.0f * 1024.0f))
#define LOG(logtype, format, ...) MAIN_LOG((logtype), (format), ##__VA_ARGS__)
#define LOG_EXCEPTION(ex) LOG(ERROR, "%s", (ex).what())
#define LOG_TASK(logtype, process, format, ...)                                                                        \
  LOG((logtype), (string("[!M%s!!] ") + (format)).c_str(), (process)->id()->toString().c_str(), ##__VA_ARGS__)
#define LOG_SUBSCRIBE(rc, subscription)                                                                                \
  LOG(((rc) == OK ? INFO : ERROR), "!m[!!%s!m][!b%s!m]=!gsubscribe!m[qos:%i]=>[!b%s!m]!! | !m[onRecv:!!%s!m]!!\n",     \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      (subscription)->source.toString().c_str(), (uint8_t) (subscription)->qos,                                        \
      (subscription)->pattern.toString().c_str(),                                                                      \
      (subscription)->onRecvBCode ? (subscription)->onRecvBCode->toString().c_str() : "!bc/c++_impl!!")
#define LOG_UNSUBSCRIBE(rc, source, pattern)                                                                           \
  LOG(((rc) == OK ? INFO : ERROR), "!m[!!%s!m][!b%s!m]=!gunsubscribe!m=>[!b%s!m]!!\n",                                 \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(), ((source).toString().c_str()),        \
      nullptr == (pattern) ? "ALL" : (pattern)->toString().c_str())
#define LOG_PUBLISH(rc, message)                                                                                       \
  LOG(((rc) == OK ? INFO : ERROR), "!m[!!%s!m][!b%s!m]=!gpublish!m[retain:%s]!b=!!%s!b=>!m[!b%s!m]!!\n",               \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      ((message).source.toString().c_str()), (FOS_BOOL_STR((message).retain)),                                         \
      ((message).payload->toString().c_str()), ((message).target.toString().c_str()))
#define LOG_RECEIVE(rc, subscription, message)                                                                         \
  LOG(((rc) == OK ? INFO : ERROR),                                                                                     \
      (((subscription).pattern.equals((message).target))                                                               \
           ? "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern|target:!b%s!m]=!!%s!m=[!b%s!m]!!\n"                              \
           : "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern:%s][target:%s]=!!%s!m=[!b%s!m]!!\n"),                            \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      ((subscription).source.toString().c_str()), ((subscription).pattern.toString().c_str()),                         \
      ((subscription).pattern.equals((message).target)) ? ((message).payload->toString().c_str())                      \
                                                        : ((message).target.toString().c_str()),                       \
      ((subscription).pattern.equals((message).target)) ? ((message).source.toString().c_str())                        \
                                                        : ((message).payload->toString)().c_str(),                     \
      ((message).source.toString().c_str()))
#define FOS_LOG_INST(inst)                                                                                             \
  LOG(DEBUG, "[!rINST!!] [!gop!!:%s] !minst added!!: [!garg!!:[!gtype!!:%s,!gotype!!:%s,!gbcode!!:%s]!m=>!!%s]\n",     \
      (inst)->opcode().c_str(),                                                                                        \
      (inst)->v_args().empty() ? NOOBJ_FURI->toString().c_str()                                                        \
                               : (inst)->v_args().at(0)->type()->v_furi()->toString().c_str(),                         \
      (inst)->v_args().empty() ? OTYPE_STR.at(OType::NOOBJ) : OTYPE_STR.at((inst)->v_args().at(0)->otype()),           \
      (inst)->v_args().empty() ? "false" : FOS_BOOL_STR((inst)->v_args().at(0)->isBytecode()),                         \
      (inst)->v_args().empty() ? NoObj::self_ptr()->toString().c_str() : (inst)->v_args().at(0)->toString().c_str());
#define FOS_LOG_OBJ(obj)                                                                                               \
  LOG(DEBUG, "[!rOBJ!!] %s [id:!yN/A!!][stype:!y%s!!][utype:!y%s!!]\n", (obj)->toString().c_str(),                     \
      OTYPE_STR.at((obj)->otype()), (obj)->type()->toString().c_str());
#define NOTE(message) LOG(INFO, "%s\n", (message))


#ifndef FOS_MAILBOX_WARNING_SIZE
#define FOS_MAILBOX_WARNING_SIZE 15
#endif

#ifndef FOS_DEFAULT_ALGEBRA
#define FOS_DEFAULT_ALGEBRA fhatos::Algebra
#endif
#ifndef FOS_DEFAULT_PRINTER
#define FOS_DEFAULT_PRINTER fhatos::Ansi<fhatos::CPrinter>
#endif
#ifndef FOS_DEFAULT_SERIALIZER
#define FOS_DEFAULT_SERIALIZER fhatos::PtrSerializer
#endif

#ifndef FOS_DEFAULT_SOURCE_ID
#define FOS_DEFAULT_SOURCE_ID STR([anon])
#endif


  ////////////////////////////
  // ARCHITECTURE LIBRARIES //
  ////////////////////////////

#if defined(ESP32)
#define FOS_PROCESS(proc) <process/esp32/proc>
#define FOS_MQTT(mqtt) <process/router/esp/mqtt>
#define FOS_UTIL(utl) <util/esp/utl>
#elif defined(ESP8266)
#define FOS_PROCESS(proc) <process/esp8266/proc>
#define FOS_MQTT(mqtt) <process/router/esp/mqtt>
#define FOS_UTIL(utl) <util/esp/utl>
#elif defined(NATIVE)
#define FOS_PROCESS(proc) <process/native/proc>
#define FOS_MQTT(mqtt) <process/router/native/mqtt>
#define FOS_UTIL(utl) <util/std/utl>
#else
#error "Unknown architecture."
#endif
#define FOS_ROUTER(rout) <process/router/rout>
#define FOS_MODULE(modu) <structure/modu>

#ifndef FOS_LOGGING
#define FOS_LOGGING ERROR
#endif

#ifndef FOS_DEFAULT_PRINTER
#define FOS_DEFAULT_PRINTER Ansi<CPrinter>
#endif

#define FOS_TYPE_FUNCTION                                                                                              \
  [](void *typeId) {                                                                                                   \
    return (void *) GLOBAL_OPTIONS->router<Router>()->read(ID(FOS_DEFAULT_SOURCE_ID), *((ID *) typeId)).get();         \
  };

  ///////////////////
  // !!TO REMOVE!! //
  ///////////////////
  static const Map<string, LOG_TYPE> STR_LOGTYPE = {{{"DEBUG_MORE", LOG_TYPE::DEBUG_MORE},
                                                     {"DEBUG", LOG_TYPE::DEBUG},
                                                     {"INFO", LOG_TYPE::INFO},
                                                     {"ERROR", LOG_TYPE::ERROR},
                                                     {"NONE", LOG_TYPE::NONE}}};
  static const Map<LOG_TYPE, string> LOGTYPE_STR = {{{LOG_TYPE::DEBUG_MORE, "DEBUG_MORE"},
                                                     {LOG_TYPE::DEBUG, "DEBUG"},
                                                     {LOG_TYPE::INFO, "INFO"},
                                                     {LOG_TYPE::ERROR, "ERROR"},
                                                     {LOG_TYPE::NONE, "NONE"}}};

  static void MAIN_LOG(const LOG_TYPE type, const char *format, ...) {
    if ((uint8_t) type < (uint8_t) GLOBAL_OPTIONS->logger<LOG_TYPE>())
      return;
    va_list arg;
    va_start(arg, format);
    char temp[128];
    char *buffer = temp;
    const size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
      buffer = new (std::nothrow) char[len + 1];
      if (!buffer) {
        return;
      }
      va_start(arg, format);
      vsnprintf(buffer, len + 1, format, arg);
      va_end(arg);
    }
    if (type == NONE)
      GLOBAL_OPTIONS->printer()->print("");
    else if (type == ERROR)
      GLOBAL_OPTIONS->printer()->print("!r[ERROR]!!  ");
    else if (type == INFO)
      GLOBAL_OPTIONS->printer()->print("!g[INFO]!!  ");
    else if (type == DEBUG)
      GLOBAL_OPTIONS->printer()->print("!y[DEBUG]!!  ");
    else if (type == DEBUG_MORE)
      GLOBAL_OPTIONS->printer()->print("!y[DEBUG_MORE]!!  ");
    FOS_DEFAULT_PRINTER::singleton()->print(buffer);
    if (buffer != temp) {
      delete[] buffer;
    }
  }
} // namespace fhatos
#endif
