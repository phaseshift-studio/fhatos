#ifndef fhatos_kernel__actor_monoid_hpp
#define fhatos_kernel__actor_monoid_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename ACTOR, typename MESSAGE, typename PAYLOAD>
class ActorMonoid {

protected:
  PAYLOAD *payload;
  ACTOR *actor;

public:
  ActorMonoid(ACTOR *actor) { this->actor = actor; }
  ActorMonoid<ACTOR, MESSAGE, PAYLOAD> operator>(PAYLOAD payload) {
    this->payload = &payload;
    return *this;
  }
  ActorMonoid<ACTOR, MESSAGE, PAYLOAD> operator>(ACTOR *other) {
    this->actor->publish(*other, *this->payload, false);
    payload = nullptr;
    return *this;
  }
  ActorMonoid<ACTOR, MESSAGE, PAYLOAD>
  operator<(Pair<Pattern, OnRecvFunction<MESSAGE>> pair) {
    this->actor->subscribe(pair.first, pair.second);
    return *this;
  }
};

} // namespace fhatos::kernel

#endif