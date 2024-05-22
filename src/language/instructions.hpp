#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <language/binary_obj.hpp>
#include <language/algebra.hpp>
#include <language/obj.hpp>
#include <language/processor.hpp>

#include "fluent.hpp"
#ifndef NATIVE
#include <process/router/local_router.hpp>

#endif

namespace fhatos {
  template<typename S>
  static List<Obj *> cast(const List<S *> *list) {
    List<Obj *> *newList = new List<Obj *>();
    for (const auto s: *list) {
      newList->push_back((Obj *) s);
    }
    return *newList;
  }

  template<typename _ARG>
  static List<Obj *> cast(const std::initializer_list<_ARG *> list) {
    List<Obj *> *newList = new List<Obj *>();
    for (const auto s: list) {
      newList->push_back((Obj *) s);
    }
    return *newList;
  }

  template<typename S>
  class StartInst final : public Inst {
  public:
    explicit StartInst(const List<S *> *starts)
      : Inst({
        "start", cast(starts),
        [starts](Obj *b){
          S *s = (new S(((S *) starts->front())->value()));
          return (S *) s;
        }
      }) {
    }
  };

  template<typename E, typename ALGEBRA = Algebra>
  class PlusInst final : public Inst {
  public:
    explicit PlusInst(const E *a)
      : Inst({
        "plus", cast({a}),
        [this](const Obj  *b) {
          return (E *) ALGEBRA::singleton()->plus((E*)this->arg<E>(0)->apply(b), (E*)b);
        }
      }) {
    }
  };

  template<typename E, typename ALGEBRA=Algebra>
  class MultInst final : public Inst {
  public:
    explicit MultInst(const E *a)
      : Inst({
        "mult", cast({a}),
        [this](const Obj  *b) {
          return (E *) ALGEBRA::singleton()->mult((E*)this->arg<E>(0)->apply(b), (E*)b);
        }
      }) {
    }
  };

  template<typename E>
  class PublishInst final : public Inst {
  public:
    explicit PublishInst(const E *a) : Inst({
      "<=", cast({a}),
      [this](const Obj*uri) {
        const BinaryObj<> *payload2 = (BinaryObj<> *) BinaryObj
            <>::fromObj((Obj *) this->template arg<E>(0)->apply(uri));
#ifndef NATIVE
        FOS_DEFAULT_ROUTER::singleton()->publish(Message{
          .source = ID("anoymous"),
          .target = ((Uri*)uri)->value(),
          .payload = payload2,
          .retain = TRANSIENT_MESSAGE
        });
        return (Obj *) uri;
#else
        LOG(DEBUG, "!rPublished!! %s\n", payload2->toString().c_str());
        return (Uri*) uri;
#endif
      }
    }) {
    }
  };

  class SubscribeInst final : public Inst {
  public:
    explicit SubscribeInst(const Uri *a, const Bytecode *b) : Inst({
      "<=", cast({(Obj *) a, (Obj *) b}),
      [this](const Obj* uri) {
        const Pattern *pattern2 = new Pattern(((Uri *) this->arg<Uri>(0)->apply(uri))->value());
        const Bytecode *bcode2 = this->arg<Bytecode>(1);
#ifndef NATIVE
        FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
          .mailbox = nullptr,
          .source = ID("anonymous"),
          .pattern = *pattern2,
          .onRecv = [this,bcode2](const Message &message) {
            bcode2->apply(new Str(message.payload->toStr().value()));
          }
        });
        return (Obj *) uri;
#else
        LOG(DEBUG, "!rSubscribed!! %s [bcode:%s]\n", pattern2->toString().c_str(), bcode2->toString().c_str());
        return (Uri*) uri;
#endif
      }
    }) {
    }
  };
} // namespace fhatos

#endif
