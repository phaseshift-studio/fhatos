#ifndef fhatos_native_hpp
#define fhatos_native_hpp
#ifdef  NATIVE

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

// fhatos utility
#include <util/ansi.hpp>

// C++ standard template library common data structures
#include <string>
#include <deque>
#include <exception>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <memory>
#include <unordered_map>

namespace fhatos {

  static const char *ANSI_ART =
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

  ///////////////////////
  /// CONTAINER TYPES ///
  ///////////////////////
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
  template<typename K, typename V>
  using Map = std::map<K, V>;
  template<typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<K> >
  using UnorderedMap = std::unordered_map<K, V, H, E>;

  using string = std::string;
  typedef uint8_t byte;
  typedef unsigned int uint;

  ///////////////////////
  /// EXCEPTION TYPES ///
  ///////////////////////
  class fError :  std::exception {
  protected:
    char *_message;

  public:
    explicit fError(const char *format, ...) noexcept {
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

    // ~fError() override { delete _message; }

     const char * what() const noexcept  {
      return this->_message;
    }
  };

  ////////////
  // MACROS //
  ////////////
#define FOS_TAB_2 "  "
#define FOS_TAB_3 "   "
#define FOS_TAB_4 "    "
#define FOS_TAB_5 "     "
#define FOS_TAB_6 "      "
#define FOS_TAB_7 "       "
#define FOS_TAB_8 "        "
#define FOS_TAB "  "
  enum LOG_TYPE { DEBUG = 0, INFO = 1, ERROR = 2, NONE = 3 };
#define LOG_EXCEPTION(ex) LOG(ERROR, ex.what())
#define LOG(logtype, format, ...) MAIN_LOG((logtype), (format), ##__VA_ARGS__)
  static void MAIN_LOG(const LOG_TYPE type, const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    char temp[64];
    char *buffer = temp;
    const size_t len = vsnprintf(temp, sizeof(temp), format, arg);
    va_end(arg);
    if (len > sizeof(temp) - 1) {
      buffer = new(std::nothrow) char[len + 1];
      if (!buffer) {
        return;
      }
      va_start(arg, format);
      vsnprintf(buffer, len + 1, format, arg);
      va_end(arg);
    }
    static auto ansi = Ansi(new CPrinter());
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
    fflush(stdout);
  }
}
#endif
#endif
