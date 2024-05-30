/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <language/binary_obj.hpp>
#include <language/algebra.hpp>
#include <language/obj.hpp>
#include <process/router/local_router.hpp>
#include <atomic>

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
    explicit StartInst(const List<Obj *> *starts)
      : Inst({
        "start", cast(starts),
        [](const Obj *start) {
          return start;
        }
      }) {
    }
  };

  /*class EndInst final : public Inst {
  public:
    explicit EndInst() : Inst({
      "end", {}, [](const Obj *end) {
        return end;
      }
    }) {
    }
  };*/

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename ALGEBRA=Algebra>
  class BranchInst final : public Inst {
  public:
    const typename ALGEBRA::BRANCH_SEMANTIC branch;

    explicit BranchInst(const typename ALGEBRA::BRANCH_SEMANTIC branch, const OBJ_OR_BYTECODE &branches)
      : Inst({
          ALGEBRA::BRNCH_TO_STR(branch), {branches},
          [this,branch](const Obj *lhs)-> Obj *{
            return (Obj *) ALGEBRA::singleton()->branch(branch, lhs, this->arg(0));
          }
        }), branch(branch) {
    }
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename ROUTER = FOS_DEFAULT_ROUTER >
  class ReferenceInst final : public Inst {
  public:
    explicit ReferenceInst(const URI_OR_BYTECODE &uri,
                    const URI_OR_BYTECODE &context) : Inst({
      "ref", {uri, context}, [this](const Obj *toStore) -> const Obj *{
        ROUTER::singleton()->publish(
          Message{
            .source = this->arg(1)->apply(toStore)->template as<Uri>()->value(),
            .target = this->arg(0)->apply(toStore)->template as<Uri>()->value(),
            .payload = BinaryObj<>::fromObj(toStore),
            .retain = RETAIN_MESSAGE
          });
        return toStore;
      }
    }) {
    }
  };

  template<typename ROUTER = FOS_DEFAULT_ROUTER >
  class DereferenceInst final : public Inst {
  public:
    explicit DereferenceInst(const URI_OR_BYTECODE &uri, const URI_OR_BYTECODE &context) : Inst({
      "dref", {uri, context}, [this](const Obj *obj) -> const Obj *{
        std::atomic<Obj *> *thing = new std::atomic<Obj *>(nullptr);
        std::atomic<bool> *done = new std::atomic<bool>(false);
        const fURI uri = this->arg(0)->apply(obj)->template as<Uri>()->value();
        const fURI context = this->arg(1)->apply(obj)->template as<Uri>()->value();
        ROUTER::singleton()->subscribe(
          Subscription{
            .source = context,
            .pattern = uri,
            .onRecv = [thing,done](const Message &message) {
              thing->store(message.payload->toObj());
              done->store(true);
            }
          });
        while (!done->load()) {
          // wait until done
        }
        const Obj *ret = thing->load();
        delete thing;
        delete done;
        ROUTER::singleton()->unsubscribe(context, uri);
        return ret;
      }
    }) {
    }
  };


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  template<typename ALGEBRA=Algebra>
  class RelationalInst final : public Inst {
  public:
    const typename ALGEBRA::RELATION_PREDICATE predicate;

    explicit RelationalInst(const typename ALGEBRA::RELATION_PREDICATE predicate, const OBJ_OR_BYTECODE &rhs)
      : Inst({
          ALGEBRA::REL_TO_STR(predicate), {rhs},
          [this,predicate](const Obj *lhs)-> const Obj *{
            return ALGEBRA::singleton()->relate(predicate, lhs, this->arg(0)->apply(lhs));
          }
        }), predicate(predicate) {
    }
  };


  class IsInst final : public Inst {
  public:
    explicit IsInst(const OBJ_OR_BYTECODE &test)
      : Inst({
        "is", {test},
        [this](const Obj *input) -> const Obj *{
          return this->arg(0)->apply(input)->as<Bool>()->value() ? input : NoObj::singleton();
        }
      }) {
    }
  };

  class WhereInst final : public Inst {
  public:
    explicit WhereInst(const OBJ_OR_BYTECODE &test) : Inst({
      "where", {test}, [this](const Obj *input) -> const Obj *{
        return this->arg(0)->apply(input)->isNoObj() ? NoObj::singleton() : input;
      }
    }) {
    }
  };


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename ALGEBRA = Algebra>
  class CompositionInst final : public Inst {
  public:
    const typename ALGEBRA::COMPOSITION_OPERATOR op;

    explicit CompositionInst(const typename ALGEBRA::COMPOSITION_OPERATOR op, const OBJ_OR_BYTECODE &rhs)
      : Inst({
          ALGEBRA::COMP_TO_STR(op), {rhs},
          [this,op](const Obj *lhs) {
            return (const Obj *) ALGEBRA::singleton()->compose(op, this->arg(0)->apply(lhs), lhs);
          }
        }), op(op) {
    }
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename _URI, typename _PAYLOAD>
  class PublishInst final : public Inst {
  public:
    explicit PublishInst(const _URI *uri, const _PAYLOAD *payload, const ID &context = ID("anonymous")) : Inst({
      "<=", cast({(Obj *) uri, (Obj *) payload}),
      [this,context](const Obj *incoming) {
        const ID target = ID(((Uri *) (this->arg(0)->apply(incoming)))->value());
        const BinaryObj<> *payload2 = BinaryObj<>::fromObj((Obj *) this->arg(1)->apply(incoming));
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
        const Pattern pattern2 = Pattern(((Uri *) this->arg(0)->apply(incoming))->value());
#ifndef NATIVE
        FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
          .mailbox = nullptr,
          .source = context,
          .pattern = pattern2,
          .onRecv = [this](const Message &message) {
            this->arg(1)->apply(message.payload);
          }
        });
#else
        LOG(DEBUG, "!rSubscribed!! %s [bcode:%s]\n", pattern2.toString().c_str(),
            this->arg(1)->toString().c_str());
#endif
        return (Obj *) incoming;
      }
    }) {
    }
  };
} // namespace fhatos

#endif
