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
    explicit StartInst(List<S *> *starts)
      : Inst({
        "start", cast(starts),
        [starts](Obj *b) {
          return b;
        }
      }) {
      for (S *start: *starts) {
        this->output.push_back((Obj *) start);
      }
    }
  };

  template<typename E>
  class BranchInst final : public Inst {
  public:
    explicit BranchInst(const Rec *rec)
      : Inst({
        "branch", cast({rec}),
        [this](Obj *b) {
          Rec *rec = this->arg<Rec>(0);
          for (const auto &kv: *rec->value()) {
            if (kv.first->apply(b)) {
              return (E *) kv.second->apply(b);
            }
          }
          return (E *) nullptr;
        }
      }) {
    }
  };

  template<typename E>
  class IsInst final : public Inst {
  public:
    explicit IsInst(const E *obj)
      : Inst({
        "is", cast({obj}),
        [this](Obj *b) {
          return ((Bool *) this->arg<Obj>(0)->apply(b))->value() ? b : (Obj*)NoObj::singleton();
        }
      }) {
    }
  };


  template<typename E>
  class EqInst final : public Inst {
  public:
    explicit EqInst(const E *obj)
      : Inst({
        "eq", cast({obj}),
        [this](Obj *b) {
          return new Bool(*this->arg<E>(0)->apply(b) == *b);
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
        [this](const Obj *b) {
          return (E *) ALGEBRA::singleton()->plus((E *) this->arg<E>(0)->apply(b), (E *) b);
        }
      }) {
    }
  };

  // int -> mult(int') => int * int'
  template<typename E, typename ALGEBRA=Algebra>
  class MultInst final : public Inst {
  public:
    explicit MultInst(const E *a)
      : Inst({
        "mult", cast({a}),
        [this](const Obj *b) {
          return (E *) ALGEBRA::singleton()->mult((E *) this->arg<E>(0)->apply(b), (E *) b);
        }
      }) {
    }
  };

  // uri -> publish(message) => {uri,message} => uri
  template<typename _URI, typename _PAYLOAD>
  class PublishInst final : public Inst {
  public:
    explicit PublishInst(const _URI *uri, const _PAYLOAD *payload, const ID context = ID("anonymous")) : Inst({
      "<=", cast({(Obj *) uri, (Obj *) payload}),
      [this,context](const Obj *incoming) {
        const ID target = ID(((Uri *) (this->arg<Obj>(0)->apply(incoming)))->value());
        const BinaryObj<> *payload2 = BinaryObj<>::fromObj((Obj *) this->arg<Obj>(1)->apply(incoming));
#ifndef NATIVE
        FOS_DEFAULT_ROUTER::singleton()->publish(Message{
          .source = context,
          .target = target,
          .payload = payload2,
          .retain = TRANSIENT_MESSAGE
        });
#else
        LOG(DEBUG, "!rPublished!! %s <= %s\n", target.toString().c_str(), payload2->toString().c_str());
#endif
        return (Obj *) incoming;
      }
    }) {
    }
  };

  template<typename _URI, typename _ONRECV>
  class SubscribeInst final : public Inst {
  public:
    explicit SubscribeInst(const _URI *pattern, const _ONRECV *onRecv, const ID context = ID("anonymous")) : Inst({
      "<=", cast({(Obj *) pattern, (Obj *) onRecv}),
      [this,context](const Obj *incoming) {
        const Pattern pattern2 = Pattern(((Uri *) this->arg<Obj>(0)->apply(incoming))->value());
#ifndef NATIVE
        FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
          .mailbox = nullptr,
          .source = context,
          .pattern = pattern2,
          .onRecv = [this](const Message &message) {
            this->arg<Obj>(1)->apply(message.payload);
          }
        });
#else
        LOG(DEBUG, "!rSubscribed!! %s [bcode:%s]\n", pattern2.toString().c_str(),
            this->arg<_ONRECV>(1)->toString().c_str());
#endif
        return (Obj *) incoming;
      }
    }) {
    }
  };
} // namespace fhatos

#endif
