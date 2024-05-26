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

  class StartInst final : public Inst {
  public:
    explicit StartInst(List<Obj *> *starts)
      : Inst({
        "start", cast(starts),
        [](Obj *b) {
          return b;
        }
      }) {
      for (const auto start: *starts) {
        this->output.push_back(start);
      }
    }
  };

  class BranchInst final : public Inst {
  public:
    explicit BranchInst(const Rec *rec)
      : Inst({
        "branch", cast({rec}),
        [this](Obj *b) {
          for (const Rec *rec1 = this->arg<Rec>(0); const auto &kv: *rec1->value()) {
            if (kv.first->apply(b)) {
              return (Obj *) kv.second->apply(b);
            }
          }
          return (Obj *) nullptr;
        }
      }) {
    }
  };

  class IsInst final : public Inst {
  public:
    explicit IsInst(const BOOL_OR_BYTECODE obj)
      : Inst({
        "is", cast({obj.cast<Obj>()}),
        [this](Obj *b) {
          return (this->arg<BOOL_OR_BYTECODE>(0)->apply(b)->value()) ? b : (Obj *) NoObj::singleton();
        }
      }) {
    }
  };


  class EqInst final : public Inst {
  public:
    explicit EqInst(const Obj *obj)
      : Inst({
        "eq", cast({obj}),
        [this](Obj *b) {
          return new Bool(*this->arg<Obj>(0)->apply(b) == *b);
        }
      }) {
    }
  };


  template<typename ALGEBRA = Algebra>
  class PlusInst final : public Inst {
  public:
    explicit PlusInst(const OBJ_OR_BYTECODE<Obj> obj)
      : Inst({
        "plus", cast({obj.template cast<Obj>()}),
        [this](const Obj *b) {
          return (Obj *) (
            ALGEBRA::singleton()->plus((Obj *) this->arg<OBJ_OR_BYTECODE<Obj> >(0)->apply(b), (Obj *) b));
        }
      }) {
    }
  };

  // int -> mult(int') => int * int'
  template<typename ALGEBRA=Algebra>
  class MultInst final : public Inst {
  public:
    explicit MultInst(const Obj *obj)
      : Inst({
        "mult", cast({obj}),
        [this](const Obj *b) {
          return (Obj *) ALGEBRA::singleton()->mult((Obj *) this->arg<Obj>(0)->apply(b), (Obj *) b);
        }
      }) {
    }
  };

  // uri -> publish(message) => {uri,message} => uri
  template<typename _URI, typename _PAYLOAD>
  class PublishInst final : public Inst {
  public:
    explicit PublishInst(const _URI *uri, const _PAYLOAD *payload, const ID &context = ID("anonymous")) : Inst({
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
