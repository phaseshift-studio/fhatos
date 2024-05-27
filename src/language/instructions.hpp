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
    explicit StartInst(const List<Obj *> *starts)
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

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  enum class BRANCH_SEMANTIC {
    SPLIT,
    CHAIN,
    SWITCH
  };

  class BranchInst final : public Inst {
  public:
    explicit BranchInst(const OBJ_OR_BYTECODE &branches)
      : Inst({
        "branch", cast({branches.cast<>()}),
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


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  template<typename ALGEBRA=Algebra>
  class RelationalInst final : public Inst {
  public:
    const RELATION_PREDICATE predicate;

    explicit RelationalInst(const RELATION_PREDICATE predicate, const OBJ_OR_BYTECODE &rhs)
      : Inst({
          REL_TO_STR.at(predicate), cast({rhs.cast<Obj>()}),
          [this,predicate](const Obj *lhs)-> Obj *{
            return (Obj*) ALGEBRA::singleton()->relate(predicate, this->arg<Obj>(0)->apply(lhs), lhs);
          }
        }), predicate(predicate) {
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


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  template<typename ALGEBRA = Algebra>
  class CompositionInst final : public Inst {
  public:
    const COMPOSITION_OPERATOR op;

    explicit CompositionInst(const COMPOSITION_OPERATOR op, const OBJ_OR_BYTECODE &rhs)
      : Inst({
          COMP_TO_STR.at(op), cast({rhs.cast<Obj>()}),
          [this,op](const Obj *lhs) {
            return ALGEBRA::singleton()->compose(op, this->arg<Obj>(0)->apply(lhs), lhs);
          }
        }), op(op) {
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
