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

// define ESP_ARCH as generalized ESPXXXX flag
#ifdef ESP32
#define ESP_ARCH
#endif
#ifdef ESP8266
#define ESP_ARCH
#endif

#ifdef ESP_ARCH
#include <Arduino.h>
#include <esp_heap_caps.h>
#ifndef FOS_SERIAL_BAUDRATE
#define FOS_SERIAL_BAUDRATE 115200
#endif
#ifndef FOS_SERIAL_TIMEOUT
#define FOS_SERIAL_TIMEOUT 10
#endif
#ifndef FOS_EXTERNAL_MEMORY_LIMIT
#define FOS_EXTERNAL_MEMORY_LIMIT 128
#endif
//#include <structure/stype/esp32/allocator.hpp>
#endif

#include "util/ansi.hpp"
#include "util/fhat_error.hpp"

// C++ standard template library common data structures
#include <any>
#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include "util/logger.hpp"
#include <random>
#include "util/options.hpp"


namespace fhatos {
  using std::string;
  using std::to_string;
  using std::shared_ptr;
  using std::weak_ptr;
  using std::unique_ptr;
  using std::make_pair;
  using std::make_tuple;
  using std::make_shared;
  using std::make_unique;
  using std::atomic_int;
  using std::atomic;
  using std::thread;
  using std::initializer_list;
  using std::stringstream;
  using std::fstream;
  using std::ios;
  using std::enable_shared_from_this;


  [[maybe_unused]] static auto ANSI_ART =
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
  using up = std::unique_ptr<A>;
  template<typename A>
  using wp = std::weak_ptr<A>;

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
  template<typename A, typename C = std::less<>>
  using Set = std::set<A, C>;
  template<typename A>
  using Deque = std::deque<A>;
  template<typename A>
  using Deque_p = ptr<std::deque<A>>;
  template<typename K, typename V>
  using Pair = std::pair<K, V>;
  template<typename K, typename V>
  using Pair_p = ptr<std::pair<K, V>>;
  template<typename A, typename B, typename C>
  using Trip = std::tuple<A, B, C>;
  template<typename A, typename B, typename C, typename D>
  using Quad = std::tuple<A, B, C, D>;
  template<typename K, typename V, typename C = std::less<>, typename A = std::allocator<std::pair<const K, V>>>
  using Map = std::map<K, V, C, A>;
  template<typename K, typename V, typename C = std::less<>, typename A = std::allocator<std::pair<const K, V>>>
  using Map_p = ptr<std::map<K, V, C, A>>;

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
#define FOS_IS_DOC_BUILD 0 == strcmp(STR(BUILD_DOCS), "ON")
#define FOS_NOOBJ_TOKEN noobj
#define FOS_OBJ_TOKEN obj
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
#define FURI_WRAP "!g[!b%s!g]!!"
#define FURI_WRAP_C(color) STR(!g[!color%s!g]!!)
#define SCHEDULER_FURI_WRAP "!G[!Y%s!G]!!"
#define FOS_BYTES_MB_STR "%i (%.2f MB)"
#define FOS_BYTES_MB(a) a, (((float) a) / (1024.0f * 1024.0f))
#define LOG(logtype, format, ...) Logger::MAIN_LOG((logtype), (format), ##__VA_ARGS__)
#define LOG_EXCEPTION(obj,ex) LOG(ERROR, FURI_WRAP " %s\n", (obj)->vid()->toString().c_str(), (ex).what())
#define LOG_KERNEL_OBJ(logtype, obj, format, ...)                                                                               \
LOG((logtype), (string("!G[!Y%s!G]!! ") + (format)).c_str(),(obj)->vid()->toString().c_str(), ##__VA_ARGS__)
#define LOG_SCHEDULER_STATIC(logtype, format, ...)                                                                      \
LOG((logtype), (string("!G[!Y%s!G]!! ") + (format)).c_str(), Options::singleton()->scheduler<Scheduler>()->vid()->toString().c_str(), ##__VA_ARGS__)
#define LOG_PROCESS(logtype, process, format, ...)                                                                     \
  LOG((logtype), (string("!g[!b%s!g]!! ") + (format)).c_str(), (process)->vid()->toString().c_str(), ##__VA_ARGS__)
#define LOG_OBJ(logtype, obj, format, ...)                                                                     \
LOG((logtype), (string("!g[!m%s!g]!! ") + (format)).c_str(), (obj)->vid_or_tid()->toString().c_str(), ##__VA_ARGS__)
#define LOG_STRUCTURE(logtype, structure, format, ...)                                                                 \
  LOG((logtype), (string("!g[!b%s!g]!! ") + (format)).c_str(), (structure)->pattern()->toString().c_str(),             \
      ##__VA_ARGS__)
#define FOS_DOMAIN "domain"
#define FOS_RANGE "range"

#define ROUTER_FURI_WRAP SCHEDULER_FURI_WRAP
#ifdef NATIVE
#define CONST_CHAR(__var_name__, __chars__) const char *__var_name__ = (__chars__)
#else
#define CONST_CHAR(__var_name__, __chars__) const char *__var_name__ = (__chars__)
#endif
  ////////////////////////////
  // ARCHITECTURE LIBRARIES //
  ////////////////////////////
#if defined(ESP32)
#ifndef FOS_MACHINE_NAME
#define FOS_MACHINE_NAME fhatos_esp32
#endif
#ifndef FOS_MACHINE_MODEL
#if CONFIG_IDF_TARGET_ESP32
  #define FOS_MACHINE_MODEL ESP32
#elif CONFIG_IDF_TARGET_ESP32S2
#define FOS_MACHINE_MODEL ESP32S2
#elif CONFIG_IDF_TARGET_ESP32C3
#define FOS_MACHINE_MODEL ESP32C3
#elif CONFIG_IDF_TARGET_ESP32S3
#define FOS_MACHINE_MODEL ESP32S3
#elif CONFIG_IDF_TARGET_ESP32H4
#define FOS_MACHINE_MODEL ESP32H4
#elif CONFIG_IDF_TARGET_ESP32C2
#define FOS_MACHINE_MODEL ESP32C2
#elif CONFIG_IDF_TARGET_ESP32C6
#define FOS_MACHINE_MODEL ESP32C6
#elif CONFIG_IDF_TARGET_ESP32H2
#define FOS_MACHINE_MODEL ESPH2
#endif
#endif
#define FOS_PROCESS(__process__) <process/ptype/esp32/__process__>
#define FOS_MQTT(__mqtt__) <structure/stype/mqtt/esp/__mqtt__>
#define FOS_UTIL(__util__) <util/esp/__util__>
#define FOS_MUTEX(__mutex__) <process/util/mutex/esp/__mutex__>
#define FOS_FILE_SYSTEM(__fs__) <model/fs/esp32/__fs__>
#define FOS_MEMORY(__memory__) <model/soc/memory/esp32/__memory__>
#define FOS_BLE(__ble__) <structure/stype/ble/esp/__ble__>
#define FOS_TIMER(__timer__) <model/timer/esp/__timer__>
#elif defined(ESP8266)
#ifndef FOS_MACHINE_NAME
#define FOS_MACHINE_NAME fhatos_esp8266
#endif
#define FOS_PROCESS(__process__) <process/esp8266/__process__>
#define FOS_MQTT(__mqtt__) <structure/stype/mqtt/esp/__mqtt__>
#define FOS_UTIL(__util__) <util/esp/__util__>
#elif defined(NATIVE)
#ifndef FOS_MACHINE_NAME
#if defined(NANOPI)
#define FOS_MACHINE_NAME fhatos_nanopi
#elif defined(ORANGEPI)
#define FOS_MACHINE_NAME fhatos_orangepi
#elif defined (RASPBERRYPI)
#define FOS_MACHINE_NAME fhatos_raspberrypi
#else
#define FOS_MACHINE_NAME fhatos_native
#endif
#endif
#ifndef FOS_MACHINE_MODEL
#define FOS_MACHINE_MODEL
#endif
#define FOS_PROCESS(__process__) <process/ptype/native/__process__>
#define FOS_MQTT(__mqtt__) <structure/stype/mqtt/native/__mqtt__>
#define FOS_UTIL(__util__) <util/std/__util__>
#define FOS_MUTEX(__mutex__) <process/util/mutex/native/__mutex__>
#define FOS_FILE_SYSTEM(__fs__) <model/fs/native/__fs__>
#define FOS_MEMORY(__memory__) <model/soc/memory/native/__memory__>
#define DRAM_ATTR
#define IRAM_ATTR
#else
#error "Unknown architecture."
#endif
} // namespace fhatos
#endif
