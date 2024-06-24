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
#include <process/router/local_router.hpp>
#include <process/router/publisher.hpp>

namespace fhatos {
  struct Insts {
    Insts() = delete;
    static Objp start(const List<Objp> &starts) {
      return Obj::to_inst("start", starts, [](const ptr<Obj> &start) { return start; });
    }

    static Objp explain() {
      return Obj::to_inst("explain", {}, [](const ptr<Obj>) { return nullptr; });
    }

    static Objp plus(const Objp rhs) {
      return Obj::to_inst("plus", {rhs}, [rhs](const Objp &lhs) { return share(*lhs + *rhs->apply(lhs)); });
    }

    static Objp mult(const Objp rhs) {
      return Obj::to_inst("mult", {rhs}, [rhs](const Objp &lhs) { return share(*lhs * *rhs->apply(lhs)); });
    }

    static Objp mod(const Objp rhs) {
      return Obj::to_inst("mod", {rhs}, [rhs](const Objp &lhs) { return share(*lhs % *rhs->apply(lhs)); });
    }

    static Objp as(const Objp uri) {
      return Obj::to_inst("as", {uri}, [uri](const Objp &lhs) { return lhs->as(share(uri->uri_value())); });
    }

    static Objp bswitch(const Objp rec) {
      return Obj::to_inst("switch", {rec}, [rec](const Objp &lhs) {
        for (const auto &pair: rec->rec_value()) {
          if (!pair.first->apply(lhs)->isNoObj())
            return pair.second->apply(lhs);
        }
        return Obj::to_noobj();
      });
    }

    static Objp is(const Objp xbool) {
      return Obj::to_inst("is", {xbool},
                          [xbool](const Objp &lhs) { return xbool->apply(lhs)->bool_value() ? lhs : Obj::to_noobj(); });
    }


    static Objp neq(const Objp rhs) {
      return Obj::to_inst("neq", {rhs}, [rhs](const Objp &lhs) { return Obj::to_bool(*lhs != *rhs->apply(lhs)); });
    }

    static Objp eq(const Objp rhs) {
      return Obj::to_inst("eq", {rhs}, [rhs](const Objp &lhs) { return Obj::to_bool(*lhs == *(rhs->apply(lhs))); });
    }

    static Objp gte(const Objp rhs) {
      return Obj::to_inst("gte", {rhs}, [rhs](const Objp &lhs) { return Obj::to_bool(*lhs >= *rhs->apply(lhs)); });
    }

    static Objp gt(const Objp rhs) {
      return Obj::to_inst("gt", {rhs}, [rhs](const Objp &lhs) { return Obj::to_bool(*lhs > *rhs->apply(lhs)); });
    }

    static Objp lte(const Objp rhs) {
      return Obj::to_inst("gte", {rhs}, [rhs](const Objp &lhs) { return Obj::to_bool(*lhs <= *rhs->apply(lhs)); });
    }

    static Objp lt(const Objp rhs) {
      return Obj::to_inst("gt", {rhs}, [rhs](const Objp &lhs) { return Obj::to_bool(*lhs < *rhs->apply(lhs)); });
    }

    static Objp define(const Objp uri, const Objp type) {
      return Obj::to_inst("define", {uri, type}, [uri, type](const Objp &lhs) {
        /*this->_bcode->template createType<ROUTER>(this->arg(0)->apply(obj)->template as<Uri>()->value(),
                                                  ptr<const Obj>(this->arg(1)->obj()));*/
        return lhs;
      });
    }

    template<typename PRINTER = FOS_DEFAULT_PRINTER>
    static Objp print(const Objp toprint) {
      return Obj::to_inst("print", {toprint}, [toprint](const Objp &lhs) {
        const Objp done = toprint->apply(lhs);
        PRINTER::singleton()->printf("%s\n", done->toString().c_str());
        return lhs;
      });
    }

    template<typename ROUTER = FOS_DEFAULT_ROUTER>
    static Objp publish(const Objp &target, const Objp &payload) {
      return Obj::to_inst("publish", {target, payload}, [target, payload](const Objp lhs) -> const Objp {
        ROUTER::singleton()->publish(Message{.source = "123",
                                             .target = fURI(target->apply(lhs)->uri_value()),
                                             .payload = Objp(payload) /*->apply(incoming)*/,
                                             .retain = TRANSIENT_MESSAGE});
        return lhs;
      });
    }

    template<typename ROUTER = FOS_DEFAULT_ROUTER>
    static Objp subscibe(const Objp &pattern, const Objp &onRecv) {
      return Obj::to_inst("subscribe", {pattern, onRecv}, [pattern, onRecv](const Objp lhs) -> const Objp {
        ROUTER::singleton()->subscribe(
            Subscription{.mailbox = nullptr,
                         .source = "123",
                         .pattern = fURI(pattern->apply(lhs)->uri_value()),
                         .onRecv = [onRecv](const ptr<Message> &message) {
                           const Objp outgoing = onRecv->apply(ptr<Obj>((Obj *) message->payload.get()));
                           LOG(INFO, "subscription result: %s\n", outgoing->toString().c_str());
                         }});
        return lhs;
      });
    }
    /*  class CountInst final : public ManyToOneInst {
      public:
        explicit CountInst() :
            ManyToOneInst("count", {}, [](const Obj *obj) { return new Int(((Objs *) obj)->value()->size()); }) {}
      };*/
  };
  /*template<typename ROUTER = FOS_DEFAULT_ROUTER>
  static Objp select(List<Obj> uris) {}
  class SelectInst final : public OneToOneInst {
  public:
    explicit SelectInst(const List<ptr<Uri>> uris) :
        OneToOneInst({"select", *(List<ptr<Obj>> *) (&uris), [this](ptr<Obj> obj) -> ptr<Obj> {
                        RecMap<> map;
                        for (const ptr<Obj> uri: this->v_args()) {
                          const ptr<Uri> u = ObjHelper::checkType<OType::URI, Uri>(uri->apply(obj));
                          ptr<Obj> key = (ptr<Obj>) u;
                          ptr<Obj> value = ROUTER::singleton()->template read<Obj>(fURI("123"), u->value());
                          map.insert({key, value});
                        }
                        return share(Rec(map));
                      }}) {}
    explicit SelectInst(const ptr<Rec> branches) :
        OneToOneInst({"select", {branches}, [this](const ptr<Obj> lhs) -> const ptr<Obj> {
                        const RecMap<> split = std::dynamic_pointer_cast<Rec>(this->arg(0))->value();
                        RecMap<> map;
                        for (const auto &[k, v]: split) {
                          const ptr<Uri> key = ObjHelper::checkType<OType::URI, Uri>(k->apply(lhs));
                          const ptr<Obj> value = v->apply(ROUTER::singleton()->read(fURI("123"), key->value()));
                          map.insert({key, value});
                        }
                        return share(Rec(map));
                      }}) {}
  };*/

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

  /* template<typename ROUTER = FOS_DEFAULT_ROUTER>
   class DefineInst final : public OneToOneInst {
   public:
     explicit DefineInst(const ptr<Type> &utype, const ptr<Bytecode> &typeDefinition) :
         OneToOneInst("define", {utype, typeDefinition}, [this](const ptr<Obj> obj) -> const ptr<Obj> {
           this->_bcode->template createType<ROUTER>(this->arg(0)->apply(obj)->template as<Uri>()->value(),
                                                      ptr<const Obj>(this->arg(1)->obj()));
           return obj;
         }) {}
   };*/


  /*template<typename ROUTER = FOS_DEFAULT_ROUTER>
  class ReferenceInst final : public OneToOneInst {
  public:
    explicit ReferenceInst(const ptr<Uri> &uri) :
        OneToOneInst("ref", {uri}, [this](const ptr<Obj> toStore) -> const ptr<Obj> {
          RESPONSE_CODE response = ROUTER::singleton()->write(
              ptr<const Obj>(ObjHelper::clone<Obj>(toStore.get())), this->_bcode->id() ID("123"),
              std::static_pointer_cast<Uri>(this->arg(0)->apply(toStore))->value());
          // if(!RESPONSE_CODE)
          //  LOG(ERROR,"")
          return toStore;
        }) {}
  };*/

  /* template<typename ROUTER = FOS_DEFAULT_ROUTER>
   class DereferenceInst final : public OneToOneInst {
   public:
     explicit DereferenceInst(const ptr<Uri> &target) :
         OneToOneInst("dref", {target}, [this](const ptr<Obj> obj) -> const ptr<Obj> {
           return ObjHelper::clone<Obj>(ROUTER::singleton()
                                            ->template read<Obj>(this->_bcode->id() ID("123"),
                                                                 this->arg(0)->apply(obj)->template as<Uri>()->v_furi())
                                            .get());
         }) {}
   };*/
} // namespace fhatos

#endif
