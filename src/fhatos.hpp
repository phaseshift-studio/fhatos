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

#include <Arduino.h>
#include <ArduinoJson.h>

// C++ standard template library common data structures
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <queue>
#include <set>

namespace fhatos {

typedef void (*VoidPtr)(void);
typedef std::function<void(void)> Void0;
template <typename A> using Consumer = std::function<void(A)>;
template <typename A, typename B> using BiConsumer = std::function<void(A, B)>;
template <typename A, typename B, typename C>
using TriConsumer = std::function<void(A, B, C)>;
template <typename A> using Supplier = std::function<A()>;
template <typename A, typename B> using Function = std::function<B(A)>;
template <typename A, typename B, typename C>
using BiFunction = std::function<C(A, B)>;
template <typename A, typename B, typename C, typename D>
using TriFunction = std::function<D(A, B, C)>;
template <typename K, typename V> using Map = std::map<K, V>;
template <typename A> using List = std::list<A>;
template <typename A> using Queue = std::queue<A>;
template <typename A> using Deque = std::deque<A>;
// template <typename A> using Set = std::set<A>;
template <typename A, typename B> using Pair = std::pair<A, B>;
template <typename A> using Option = std::optional<A>;

} // namespace fhat

#endif