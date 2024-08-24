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
#ifndef FOS_SERIAL_BAUDRATE
#define FOS_SERIAL_BAUDRATE 115200
#endif
#ifndef FOS_SERIAL_TIMEOUT
#define FOS_SERIAL_TIMEOUT 10
#endif
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
#include <util/logger.hpp>
#include <util/options.hpp>

namespace fhatos {
  [[maybe_unused]] static const char *ANSI_ART =
          "!r            !_PhaseShift Studio Presents!! \n"
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
  ///////////////////////
  /// CONTAINER TYPES ///
  ///////////////////////
  template<typename A>
  using ptr = std::shared_ptr<A>;

  template<typename A>
  ptr<A> share(const A a) {
    return std::make_shared<A>(a);
  }

  using Any = std::any;
  template<typename A>
  using Option = std::optional<A>;
  template<typename A>
  using List = std::vector<A>;
  template<typename A>
  using List_p = ptr<List<A>>;
  template<typename A>
  using Deque = std::deque<A>;
  template<typename A>
  using Deque_p = ptr<std::deque<A>>;
  template<typename K, typename V>
  using Pair = std::pair<K, V>;
  template<typename A, typename B, typename C>
  using Trip = std::tuple<A, B, C>;
  template<typename A, typename B, typename C, typename D>
  using Quad = std::tuple<A, B, C, D>;
  template<typename K, typename V, typename C = std::less<>, typename A = std::allocator<std::pair<const K, V>>>
  using Map = std::map<K, V, C, A>;
  template<typename K, typename V, typename C = std::less<>, typename A = std::allocator<std::pair<const K, V>>>
  using Map_p = ptr<std::map<K, V, C, A>>;
  template<typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<K>>
  using OrderedMap = tsl::ordered_map<K, V, H, E>;

  using string = std::string;
  using fbyte = uint8_t;
  using uint = unsigned int;
  static const char *EMPTY_CHARS = "";

  ////////////
  // MACROS //
  ////////////
#define FOS_SAFE_FREE(p)                                                                                               \
  {                                                                                                                    \
    if ((p) != nullptr)                                                                                                \
      (void) free((void *) (p));                                                                                       \
    (p) = nullptr;                                                                                                     \
  }

#define FOS_SAFE_DELETE(p)                                                                                             \
  {                                                                                                                    \
    if (p)                                                                                                             \
      delete (p);                                                                                                      \
  }
#define FOS_NOOBJ_TOKEN noobj
#define FOS_MAX_FURI_SEGMENTS 10
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
#define LOG(logtype, format, ...) Logger::MAIN_LOG((logtype), (format), ##__VA_ARGS__)
#define LOG_EXCEPTION(ex) LOG(ERROR, "%s", (ex).what())
#define LOG_PROCESS(logtype, process, format, ...)                                                                     \
  LOG((logtype), (string("!g[!b%s!g]!! ") + (format)).c_str(), (process)->id()->toString().c_str(), ##__VA_ARGS__)
#define LOG_STRUCTURE(logtype, structure, format, ...)                                                                 \
  LOG((logtype), (string("!g[!b%s!g]!! ") + (format)).c_str(), (structure)->pattern()->toString().c_str(),             \
      ##__VA_ARGS__)
#define FOS_LOG_INST(inst)                                                                                             \
  LOG(DEBUG, "[!rINST!!] [!gop!!:%s] !minst added!!: [!garg!!:[!gtype!!:%s,!gotype!!:%s,!gbcode!!:%s]!m=>!!%s]\n",     \
      (inst)->opcode().c_str(),                                                                                        \
      (inst)->v_args().empty() ? NOOBJ_FURI->toString().c_str()                                                        \
                               : (inst)->v_args().at(0)->pattern()->v_furi()->toString().c_str(),                      \
      (inst)->v_args().empty() ? OTypes.toChars(OType::NOOBJ) : OTypes.toChars((inst)->v_args().at(0)->otype()),       \
      (inst)->v_args().empty() ? "false" : FOS_BOOL_STR((inst)->v_args().at(0)->isBytecode()),                         \
      (inst)->v_args().empty() ? NoObj::self_ptr()->toString().c_str() : (inst)->v_args().at(0)->toString().c_str());
#define FOS_LOG_OBJ(obj)                                                                                               \
  LOG(DEBUG, "[!rOBJ!!] %s [id:!yN/A!!][stype:!y%s!!][utype:!y%s!!]\n", (obj)->toString().c_str(),                     \
      OTypes.toChars((obj)->otype()), (obj)->pattern()->toString().c_str());
#define NOTE(message) LOG(INFO, "%s\n", (message))
#define FURI_WRAP "!g[!b%s!g]!!"

#ifndef FOS_MAILBOX_WARNING_SIZE
#define FOS_MAILBOX_WARNING_SIZE 15
#endif

#ifndef FOS_DEFAULT_SOURCE_ID
#define FOS_DEFAULT_SOURCE_ID STR(anon)
#endif

  ////////////////////////////
  // ARCHITECTURE LIBRARIES //
  ////////////////////////////

#if defined(ESP32)
#define FOS_PROCESS(proc) <process/ptype/esp32/proc>
#define FOS_MQTT(mqtt) <structure/router/esp/mqtt>
#define FOS_UTIL(utl) <util/esp/utl>
#define FOS_FILE_SYSTEM(__fs__) <model/fs/esp32/__fs__>
#elif defined(ESP8266)
#define FOS_PROCESS(__process__) <process/esp8266/__process__>
#define FOS_MQTT(mqtt) <structure/router/esp/mqtt>
#define FOS_UTIL(utl) <util/esp/utl>
#elif defined(NATIVE)
#define FOS_PROCESS(__process__) <process/ptype/native/__process__>
#define FOS_MQTT(__mqtt__) <structure/stype/mqtt/native/__mqtt__>
#define FOS_UTIL(utl) <util/std/utl>
#define FOS_FILE_SYSTEM(__fs__) <model/fs/native/__fs__>
#else
#error "Unknown architecture."
#endif
} // namespace fhatos
#endif
