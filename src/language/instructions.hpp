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
          [this,branch](const Obj *lhs)-> const Obj *{
            return ALGEBRA::singleton()->branch(branch, lhs, this->arg(0));
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
                           const URI_OR_BYTECODE &target) : Inst({
      "ref", {uri, target}, [this](const Obj *toStore) -> const Obj *{
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
    explicit DereferenceInst(const URI_OR_BYTECODE &uri, const URI_OR_BYTECODE &source) : Inst({
      "dref", {uri, source}, [this](const Obj *obj) -> const Obj *{
        std::atomic<Obj *> *thing = new std::atomic<Obj *>(nullptr);
        std::atomic<bool> *done = new std::atomic<bool>(false);
        const fURI uri = this->arg(0)->apply(obj)->template as<Uri>()->value();
        const fURI source = this->arg(1)->apply(obj)->template as<Uri>()->value();
        ROUTER::singleton()->subscribe(
          Subscription{
            .source = source,
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
        ROUTER::singleton()->unsubscribe(source, uri);
        return ret;
      }
    }) {
    }
  };


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  template<typename ALGEBRA=FOS_DEFAULT_ALGEBRA>
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

  template<typename ALGEBRA = FOS_DEFAULT_ALGEBRA>
  class CompositionInst final : public Inst {
  public:
    const typename ALGEBRA::COMPOSITION_OPERATOR op;

    explicit CompositionInst(const typename ALGEBRA::COMPOSITION_OPERATOR op, const OBJ_OR_BYTECODE &rhs)
      : Inst({
          ALGEBRA::COMP_TO_STR(op), {rhs},
          [this,op](const Obj *lhs) {
            return ALGEBRA::singleton()->compose(op, this->arg(0)->apply(lhs), lhs);
          }
        }), op(op) {
    }
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  class PublishInst final : public Inst {
  public:
    explicit PublishInst(const URI_OR_BYTECODE &target, const OBJ_OR_BYTECODE &payload, const ID &bcodeId) : Inst({
      "<=", {target, payload},
      [this,bcodeId](const Obj *incoming)-> const Obj *{
        FOS_DEFAULT_ROUTER::singleton()->publish(Message{
          .source = bcodeId,
          .target = this->arg(0)->apply(incoming)->template as<Uri>()->value(),
          .payload = BinaryObj<>::fromObj(this->arg(1)->apply(incoming)),
          .retain = TRANSIENT_MESSAGE
        });
        return incoming;
      }
    }) {
    }
  };

  class SubscribeInst final : public Inst {
  public:
    explicit SubscribeInst(const URI_OR_BYTECODE &pattern, const OBJ_OR_BYTECODE &onRecv, const ID &bcodeId) : Inst({
      "=>", {pattern, onRecv},
      [this,bcodeId](const Obj *incoming)-> const Obj *{
        FOS_DEFAULT_ROUTER::singleton()->subscribe(Subscription{
          .mailbox = nullptr,
          .source = bcodeId,
          .pattern = this->arg(0)->apply(incoming)->template as<Uri>()->value(),
          .onRecv = [this](const Message &message) {
            const Obj *outgoing = this->arg(1)->apply(message.payload->toObj());
            LOG(INFO, "subscription result: %s\n", outgoing->toString().c_str());
          }
        });
        return incoming;
      }
    }) {
    }
  };
} // namespace fhatos

#endif
