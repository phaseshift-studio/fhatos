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

namespace fhatos {
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

  using string = std::string;
  typedef uint8_t byte;
  typedef	unsigned int	uint;
}
#endif
#endif
