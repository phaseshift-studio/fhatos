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
  class StartInst final : public Inst<S, S> {
  public:
    explicit StartInst(const List<S *> *starts)
      : Inst<S, S>({
        "start", cast(starts),
        [starts](S *b) -> S *{
          return new S(((S *) starts->front())->value());
        }
      }) {
    }
  };

  template<typename E, typename ALGEBRA = Algebra>
  class PlusInst final : public Inst<E, E> {
  public:
    explicit PlusInst(const E *a)
      : Inst<E, E>({
        "plus", cast({a}),
        [this](const E *b) {
          return (E *) ALGEBRA::singleton()->plus(this->template arg<E>(0)->apply(b), b);
        }
      }) {
    }
  };

  template<typename E, typename ALGEBRA=Algebra>
  class MultInst final : public Inst<E, E> {
  public:
    explicit MultInst(const E *a)
      : Inst<E, E>({
        "mult", cast({a}),
        [this](const E *b) {
          return (E *) ALGEBRA::singleton()->mult(this->template arg<E>(0)->apply(b), b);
        }
      }) {
    }
  };

  template<typename E, typename ALGEBRA=Algebra>
  class PublishInst final : public Inst<Uri, Uri> {
  public:
    explicit PublishInst(const E *a) : Inst<Uri, Uri>({
      "<=", cast({a}),
      [this](const Uri *uri) {
        const BinaryObj<> *payload2 = (BinaryObj<> *) BinaryObj
            <>::fromObj((Obj *) this->template arg<E>(0)->apply(uri));
#ifndef NATIVE
        FOS_DEFAULT_ROUTER::singleton()->publish(Message{
          .source = ID("anoymous"),
          .target = uri->value(),
          .payload = payload2,
          .retain = TRANSIENT_MESSAGE
        });
        return (Uri *) uri;
#else
        LOG(DEBUG, "!rPublished!! %s\n", payload2->toString().c_str());
        return (Uri*) uri;
#endif
      }
    }) {
    }
  };

  class SubscribeInst final : public Inst<Uri, Uri> {
  public:
    explicit SubscribeInst(const Uri *a, const Bytecode<Obj, Obj> *b) : Inst<Uri, Uri>({
      "<=", cast({(Obj *) a, (Obj *) b}),
      [this](const Uri *uri) {
        const Pattern *pattern2 = new Pattern(((Uri *) this->arg<Uri>(0)->apply(uri))->value());
        const Bytecode<Obj, Obj> *bcode2 = this->arg<Bytecode<Obj, Obj> >(1);
#ifndef NATIVE
        FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
          .mailbox = nullptr,
          .source = ID("anonymous"),
          .pattern = *pattern2,
          .onRecv = [this,bcode2](const Message &message) {
            bcode2->apply((Obj *) new Str(message.payload->toStr().value()));
          }
        });
        return (Uri *) uri;
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
