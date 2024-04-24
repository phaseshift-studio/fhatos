#ifndef fhatos_kernel__actor_monoid_hpp
#define fhatos_kernel__actor_monoid_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/actor.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename ACTOR, typename MESSAGE> class ActorMonoid {

protected:
  MESSAGE *message;
  ACTOR *actor;

public:
  ActorMonoid(ACTOR *actor) { this->actor = actor; }
  ActorMonoid<ACTOR, MESSAGE> operator>(MESSAGE message) {
    this->message = &message;
    return *this;
  }
  ActorMonoid<ACTOR, MESSAGE> operator>(ACTOR *other) {
    this->actor->publish(*other, this->message->payload, false);
    //this->message = nullptr;
    return *this;
  }
  ActorMonoid<ACTOR, MESSAGE>
  operator<(Pair<Pattern, OnRecvFunction<MESSAGE>> pair) {
    this->actor->subscribe(pair.first, pair.second);
    return *this;
  }
};

} // namespace fhatos::kernel

#endif