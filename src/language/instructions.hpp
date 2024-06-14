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
#include <atomic>
#include <language/algebra.hpp>
#include <language/binary_obj.hpp>
#include <language/obj.hpp>
#include <process/router/local_router.hpp>
#include <process/router/publisher.hpp>

namespace fhatos {
  template<typename S>
  static List<Obj *> cast(const List<S *> *list) {
    List<Obj *> *newList = new List<Obj *>();
    for (const auto s: *list) {
      newList->push_back((Obj *) s);
    }
    return *newList;
  }

  /*template<typename _ARG>
  static List<Obj *> cast(const std::initializer_list<_ARG *> list) {
    List<Obj *> *newList = new List<Obj *>();
    for (const auto s: list) {
      newList->push_back((Obj *) s);
    }
    return *newList;
  }*/

  class StartInst final : public OneToOneInst {
  public:
    explicit StartInst(const List<Obj *> *starts) :
        OneToOneInst("start", cast(starts), [](const Obj *start) { return start; }) {}
  };

  class ExplainInst final : public ManyToOneInst {
  public:
    explicit ExplainInst() : ManyToOneInst("explain", {}, [this](const Obj *) { return (Obj *) this->_bcode; }) {}
  };

  class CountInst final : public ManyToOneInst {
  public:
    explicit CountInst() :
        ManyToOneInst("count", {}, [](const Obj *obj) { return new Int(((Objs *) obj)->value()->size()); }) {}
  };

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class AsInst final : public OneToOneInst {
  public:
    explicit AsInst(const OBJ_OR_BYTECODE &utype) :
        OneToOneInst({"as", {utype}, [this](const Obj *obj) {
                        const UType utype = this->arg(0)->apply(obj)->template as<Uri>()->value();
                        const Obj *typeDefinition = this->_bcode->template getType<ROUTER>(utype);
                        if (typeDefinition->type() != OType::BYTECODE && typeDefinition->type() != obj->type())
                          return NoObj::singleton()->obj();
                        if (typeDefinition->apply(obj)->isNoObj()) {
                          LOG(ERROR, "%s is not a !y%s!!%s\n", obj->toString().c_str(), utype.toString().c_str(),
                              typeDefinition->toString().c_str());
                          return NoObj::singleton()->obj();
                        }
                        return cast(obj, utype);
                      }}) {}
  };

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class DefineInst final : public OneToOneInst {
  public:
    explicit DefineInst(const URI_OR_BYTECODE &utype, const OBJ_OR_BYTECODE &typeDefinition) :
        OneToOneInst("define", {utype, typeDefinition.cast<Obj>()}, [this](const Obj *obj) -> const Obj * {
          this->_bcode->template createType<ROUTER>(this->arg(0)->apply(obj)->template as<Uri>()->value(),
                                                    this->arg(1)->obj());
          return obj;
        }) {}
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

  template<typename ALGEBRA = Algebra>
  class BranchInst final : public OneToOneInst {
  public:
    const typename ALGEBRA::BRANCH_SEMANTIC branch;

    explicit BranchInst(const typename ALGEBRA::BRANCH_SEMANTIC branch, const OBJ_OR_BYTECODE &branches) :
        OneToOneInst(ALGEBRA::BRNCH_TO_STR(branch), {branches},
                     [this, branch](const Obj *lhs) -> const Obj * {
                       return ALGEBRA::singleton()->branch(branch, lhs, this->arg(0));
                     }),
        branch(branch) {}
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class ReferenceInst final : public OneToOneInst {
  public:
    explicit ReferenceInst(const URI_OR_BYTECODE &uri) :
        OneToOneInst("ref", {uri}, [this](const Obj *toStore) -> const Obj * {
          RESPONSE_CODE response = ROUTER::singleton()->write(
              toStore, this->_bcode->id(), this->arg(0)->apply(toStore)->template as<Uri>()->value());
          // if(!RESPONSE_CODE)
          //  LOG(ERROR,"")
          return toStore;
        }) {}
  };

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class DereferenceInst final : public OneToOneInst {
  public:
    explicit DereferenceInst(const URI_OR_BYTECODE &target) :
        OneToOneInst("dref", {target}, [this](const Obj *obj) -> const Obj * {
          return ROUTER::singleton()->read(this->_bcode->id(), this->arg(0)->apply(obj)->template as<Uri>()->value());
        }) {}
  };


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  template<typename ALGEBRA = FOS_DEFAULT_ALGEBRA>
  class RelationalInst final : public OneToOneInst {
  public:
    const typename ALGEBRA::RELATION_PREDICATE predicate;

    explicit RelationalInst(const typename ALGEBRA::RELATION_PREDICATE predicate, const OBJ_OR_BYTECODE &rhs) :
        OneToOneInst(ALGEBRA::REL_TO_STR(predicate), {rhs},
                     [this, predicate](const Obj *lhs) -> const Obj * {
                       return ALGEBRA::singleton()->relate(predicate, lhs, this->arg(0)->apply(lhs));
                     }),
        predicate(predicate) {}
  };


  class IsInst final : public OneToOneInst {
  public:
    explicit IsInst(const OBJ_OR_BYTECODE &test) :
        OneToOneInst("is", {test}, [this](const Obj *input) -> const Obj * {
          return this->arg(0)->apply(input)->as<Bool>()->value() ? input : NoObj::singleton();
        }) {}
  };

  class WhereInst final : public OneToOneInst {
  public:
    explicit WhereInst(const OBJ_OR_BYTECODE &test) :
        OneToOneInst("where", {test}, [this](const Obj *input) -> const Obj * {
          return this->arg(0)->apply(input)->isNoObj() ? NoObj::singleton() : input;
        }) {}
  };


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename ALGEBRA = FOS_DEFAULT_ALGEBRA>
  class CompositionInst final : public OneToOneInst {
  public:
    const typename ALGEBRA::COMPOSITION_OPERATOR op;

    explicit CompositionInst(const typename ALGEBRA::COMPOSITION_OPERATOR op, const OBJ_OR_BYTECODE &rhs) :
        OneToOneInst(
            ALGEBRA::COMP_TO_STR(op), {rhs},
            [this, op](const Obj *lhs) { return ALGEBRA::singleton()->compose(op, lhs, this->arg(0)->apply(lhs)); }),
        op(op) {}
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  class PublishInst final : public Publisher<FOS_DEFAULT_ROUTER>, public OneToOneInst {
  public:
    explicit PublishInst(const URI_OR_BYTECODE &target, const OBJ_OR_BYTECODE &payload, const ID &bcodeId) :
        Publisher<FOS_DEFAULT_ROUTER>(bcodeId),
        OneToOneInst("<=", {target, payload}, [this](const Obj *incoming) -> const Obj * {
          this->publish(this->arg(0)->apply(incoming)->template as<Uri>()->value(),
                        BinaryObj<>::fromObj(this->arg(1)->apply(incoming)), TRANSIENT_MESSAGE);
          return incoming;
        }) {}
  };

  class SubscribeInst final : public OneToOneInst {
  public:
    explicit SubscribeInst(const URI_OR_BYTECODE &pattern, const OBJ_OR_BYTECODE &onRecv, const ID &bcodeId) :
        OneToOneInst("=>", {pattern, onRecv}, [this, bcodeId](const Obj *incoming) -> const Obj * {
          FOS_DEFAULT_ROUTER::singleton()->subscribe(
              Subscription{.mailbox = nullptr,
                           .source = bcodeId,
                           .pattern = this->arg(0)->apply(incoming)->template as<Uri>()->value(),
                           .onRecv = [this](const Message &message) {
                             const Obj *outgoing = this->arg(1)->apply(message.payload->toObj());
                             LOG(INFO, "subscription result: %s\n", outgoing->toString().c_str());
                           }});
          return incoming;
        }) {}
  };
} // namespace fhatos

#endif
