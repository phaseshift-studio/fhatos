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
        [](const Obj *start) {
          return start;
        }
      }) {
      for (const auto start: *starts) {
        this->output.push_back(start);
      }
    }
  };

  class BranchInst final : public Inst {
  public:
    explicit BranchInst(const OBJ_OR_BYTECODE &branches)
      : Inst({
        "branch", cast({branches.cast<Obj>()}),
        [this](const Obj *incoming) -> const Obj *{
          const Rec *rec = this->arg<Rec>(0)->apply(incoming);
          for (const auto &[key, value]: *rec->value()) {
            if (!key->apply(incoming)->isNoObj()) {
              return value->apply(incoming);
            }
          }
          return NoObj::singleton();
        }
      }) {
    }
  };

  class IsInst final : public Inst {
  public:
    explicit IsInst(const OBJ_OR_BYTECODE &test)
      : Inst({
        "is", cast({test.cast<Obj>()}),
        [this](const Obj *input) {
          return this->arg<Bool>(0)->apply(input)->value() ? input : NoObj::singleton();
        }
      }) {
    }
  };


  class EqInst final : public Inst {
  public:
    explicit EqInst(const OBJ_OR_BYTECODE &rhs)
      : Inst({
        "eq", cast({rhs.cast<Obj>()}),
        [this](const Obj *lhs) {
          return new Bool(*this->arg<Obj>(0)->apply(lhs) == *lhs);
        }
      }) {
    }
  };


  template<typename ALGEBRA = Algebra>
  class PlusInst final : public Inst {
  public:
    explicit PlusInst(const OBJ_OR_BYTECODE &rhs)
      : Inst({
        "plus", cast({rhs.cast<Obj>()}),
        [this](const Obj *lhs) {
          return
              ALGEBRA::singleton()->plus(this->arg<Obj>(0)->apply(lhs), lhs);
        }
      }) {
    }
  };

  template<typename ALGEBRA = Algebra>
  class MultInst final : public Inst {
  public:
    explicit MultInst(const OBJ_OR_BYTECODE &rhs)
      : Inst({
        "plus", cast({rhs.cast<Obj>()}),
        [this](const Obj *lhs) {
          return
              ALGEBRA::singleton()->plus(this->arg<Obj>(0)->apply(lhs), lhs);
        }
      }) {
    }
  };

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
