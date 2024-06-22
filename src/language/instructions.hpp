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
#include <language/obj.hpp>
#include <language/otype/mono.hpp>
#include <language/otype/serializer.hpp>
#include <process/router/local_router.hpp>
#include <process/router/publisher.hpp>

namespace fhatos {
  template<typename S>
  static List<ptr<Obj>> cast(const List<ptr<S>> list) {
    List<ptr<Obj>> *newList = new List<ptr<Obj>>();
    for (const auto s: list) {
      newList->push_back(s);
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
    explicit StartInst(const List<ptr<Obj>> &starts) :
        OneToOneInst("start", starts, [](const ptr<Obj> &start) { return start; }) {}
  };

  class ExplainInst final : public ManyToOneInst {
  public:
    explicit ExplainInst() : ManyToOneInst("explain", {}, [this](const ptr<Obj>) { return this->bcode(); }) {}
  };

  /*  class CountInst final : public ManyToOneInst {
    public:
      explicit CountInst() :
          ManyToOneInst("count", {}, [](const Obj *obj) { return new Int(((Objs *) obj)->value()->size()); }) {}
    };*/

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class SelectInst final : public OneToOneInst {
  public:
    explicit SelectInst(const List<ptr<Obj>> *uris) :
        OneToOneInst({"select", uris, [this](const ptr<Obj> obj) -> const ptr<Obj> {
                        RecMap<> map;
                        for (const ptr<Obj> uri: this->v_args()) {
                          const ptr<Uri> u = ObjHelper::checkType<OType::URI, Uri>(uri->apply(obj));
                          ptr<Obj> key = (ptr<Obj>) u;
                          ptr<Obj> value = ROUTER::singleton()->read(fURI("123"), u->value()).get();
                          map.insert({key, value});
                        }
                        return ptr<Rec>(new Rec(map));
                      }}) {}
    explicit SelectInst(const ptr<Rec> branches) :
        OneToOneInst({"select", {branches}, [this](const ptr<Obj> lhs) -> const ptr<Obj> {
                        const RecMap<> split = this->arg(0)->template as<Rec>()->value();
                        RecMap<> map;
                        for (const auto& [k, v]: split) {
                          const ptr<Uri> key = ObjHelper::checkType<OType::URI, Uri>(k->apply(lhs));
                          const ptr<Obj> value = v->apply( ObjHelper::clone<Obj>(
                              ROUTER::singleton()->read(fURI("123"), key->value()).get()));
                          map.insert({key, value});
                        }
                        return ptr<Rec>(new Rec(map));
                      }}) {}
  };

  /*template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class AsInst final : public OneToOneInst {
  public:
    explicit AsInst(const ptr<Type> &utype = NoObj::self_ptr<Type>()) :
        OneToOneInst({"as", {utype}, [this](ptr<Obj> obj) -> const ptr<Obj> {
                        if (this->arg(0)->isNoObj())
                          return ptr<Uri>(new Uri(obj->type()->v_furi()));
                        const fURI utype = this->arg(0)->apply(obj)->template as<Uri>()->value();
                        const ptr<const Obj> typeDefinition = this->_bcode->template getType<ROUTER>(utype);
                        if (typeDefinition->type() != OType::BYTECODE && typeDefinition->type() != obj->type())
                          return NoObj::singleton()->obj();
                        if (typeDefinition->apply(obj)->isNoObj()) {
                          LOG(ERROR, "%s is not a !y%s!!%s\n", obj->toString().c_str(), utype.toString().c_str(),
                              typeDefinition->toString().c_str());
                          return NoObj::singleton()->obj();
                        }
                        return cast(obj, utype);
                      }}) {}
  };*/

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class DefineInst final : public OneToOneInst {
  public:
    explicit DefineInst(const ptr<Type> &utype, const ptr<Bytecode> &typeDefinition) :
        OneToOneInst("define", {utype, typeDefinition}, [this](const ptr<Obj> obj) -> const ptr<Obj> {
          /* this->_bcode->template createType<ROUTER>(this->arg(0)->apply(obj)->template as<Uri>()->value(),
                                                     ptr<const Obj>(this->arg(1)->obj()));*/
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

    explicit BranchInst(const typename ALGEBRA::BRANCH_SEMANTIC branch, const ptr<Rec> &branches) :
        OneToOneInst(ALGEBRA::BRNCH_TO_STR(branch), {branches},
                     [this, branch](const ptr<Obj> &lhs) -> const ptr<Obj> {
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
    explicit ReferenceInst(const ptr<Uri> &uri) :
        OneToOneInst("ref", {uri}, [this](const ptr<Obj> toStore) -> const ptr<Obj> {
          RESPONSE_CODE response = ROUTER::singleton()->write(
              ptr<const Obj>(ObjHelper::clone<Obj>(toStore.get())), /*this->_bcode->id()*/ ID("123"),
              this->arg(0)->apply(toStore)->template as<Uri>()->v_furi());
          // if(!RESPONSE_CODE)
          //  LOG(ERROR,"")
          return toStore;
        }) {}
  };

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class DereferenceInst final : public OneToOneInst {
  public:
    explicit DereferenceInst(const ptr<Uri> &target) :
        OneToOneInst("dref", {target}, [this](const ptr<Obj> obj) -> const ptr<Obj> {
          return ObjHelper::clone<Obj>(ROUTER::singleton()
                                           ->template read<Obj>(/*this->_bcode->id()*/ ID("123"),
                                                                this->arg(0)->apply(obj)->template as<Uri>()->v_furi())
                                           .get());
        }) {}
  };

  template<typename PRINTER = FOS_DEFAULT_PRINTER>
  class PrintInst final : public OneToOneInst {
  public:
    explicit PrintInst(const ptr<Obj> &toPrint) :
        OneToOneInst("print", {toPrint}, [this](const ptr<Obj> lhs) -> const ptr<Obj> {
          PRINTER::singleton()->printf("%s\n", this->arg(0)->apply(lhs)->toString().c_str());
          return lhs;
        }) {}
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////


  template<typename ALGEBRA = FOS_DEFAULT_ALGEBRA>
  class RelationalInst final : public OneToOneInst {
  public:
    const typename ALGEBRA::RELATION_PREDICATE predicate;

    explicit RelationalInst(const typename ALGEBRA::RELATION_PREDICATE& predicate, const ptr<Obj> &rhs) :
        OneToOneInst(ALGEBRA::REL_TO_STR(predicate), {rhs},
                     [this, predicate](const ptr<Obj> lhs) -> ptr<Obj> {
                       return ALGEBRA::singleton()->relate(predicate, lhs, this->arg(0)->apply(lhs));
                     }),
        predicate(predicate) {}
  };


  class IsInst final : public OneToOneInst {
  public:
    explicit IsInst(const ptr<Bool> &test) :
        OneToOneInst("is", {test}, [this](const ptr<Obj> &input) -> const ptr<Obj> {
          return ((Bool *) this->arg(0)->apply(input).get())->value() ? input : NoObj::self_ptr();
        }) {}
  };

  /*class WhereInst final : public OneToOneInst {
  public:
    explicit WhereInst(const OBJ_OR_BYTECODE &test) :
        OneToOneInst("where", {test}, [this](const Obj *input) -> const Obj * {
          return this->arg(0)->apply(input)->isNoObj() ? NoObj::singleton() : input;
        }) {}
  };*/


  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename ALGEBRA = FOS_DEFAULT_ALGEBRA>
  class CompositionInst final : public OneToOneInst {
  public:
    const typename ALGEBRA::COMPOSITION_OPERATOR op;

    explicit CompositionInst(const typename ALGEBRA::COMPOSITION_OPERATOR& op, const ptr<Obj> &rhs) :
        OneToOneInst(ALGEBRA::COMP_TO_STR(op), {rhs},
                     [this, op](const ptr<Obj> &lhs) {
                       return ALGEBRA::singleton()->compose(op, lhs, this->arg(0)->apply(lhs));
                     }),
        op(op) {}
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class PublishInst final : public OneToOneInst {
  public:
    explicit PublishInst(const ptr<Obj> &target, const ptr<Obj> &payload, const ID &bcodeId) :
        OneToOneInst("<=", {target, payload}, [this, bcodeId](const Obj *incoming) -> const Obj * {
          ROUTER::singleton()->publish(
              Message{.source = bcodeId,
                      .target = this->arg(0)->apply(incoming)->template as<Uri>()->value(),
                      .payload = ptr<const Obj>(ObjHelper::clone<>(this->arg(1))) /*->apply(incoming)*/,
                      .retain = TRANSIENT_MESSAGE});
          return incoming;
        }) {}
  };

  template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class SubscribeInst final : public OneToOneInst {
  public:
    explicit SubscribeInst(const ptr<Obj> &pattern, const ptr<Obj> &onRecv, const ID &bcodeId) :
        OneToOneInst("=>", {pattern, onRecv}, [this, bcodeId](const Obj *incoming) -> const Obj * {
          ROUTER::singleton()->subscribe(
              Subscription{.mailbox = nullptr,
                           .source = bcodeId,
                           .pattern = this->arg(0)->apply(incoming)->template as<Uri>()->v_furi(),
                           .onRecv = [this](const ptr<Message> &message) {
                             const Obj *outgoing = this->arg(1)->apply(message->payload.get());
                             LOG(INFO, "subscription result: %s\n", outgoing->toString().c_str());
                           }});
          return incoming;
        }) {}
  };
} // namespace fhatos

#endif
