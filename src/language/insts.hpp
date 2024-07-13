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
#pragma once
#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>
#include <process/router/router.hpp>
#include <util/options.hpp>

namespace fhatos {
  struct Insts {
    Insts() = delete;
    static Obj_p start(const Objs_p &starts) {
      return Obj::to_inst(
          "start", {starts}, [](const Objs_p &start) { return start; }, IType::ZERO_TO_MANY,
          (starts->objs_value()->empty() ? Obj::to_noobj() : starts));
    }

    static Obj_p explain() {
      return Obj::to_inst("explain", {}, [](const Obj_p &) { return nullptr; }, IType::MANY_TO_ONE);
    }

    static Obj_p plus(const Obj_p &rhs) {
      return Obj::to_inst(
          "plus", {rhs}, [rhs](const Obj_p &lhs) { return share(*lhs + *rhs->apply(lhs)); }, IType::ONE_TO_ONE);
    }

    static Obj_p mult(const Obj_p &rhs) {
      return Obj::to_inst(
          "mult", {rhs}, [rhs](const Obj_p &lhs) { return share(*lhs * *rhs->apply(lhs)); }, IType::ONE_TO_ONE);
    }

    static Obj_p mod(const Obj_p &rhs) {
      return Obj::to_inst(
          "mod", {rhs}, [rhs](const Obj_p &lhs) { return share(*lhs % *rhs->apply(lhs)); }, IType::ONE_TO_ONE);
    }

    static Obj_p bswitch(const Rec_p &rec) {
      return Obj::to_inst(
          "switch", {rec},
          [rec](const Obj_p &lhs) {
            try {
              for (const auto &[key, value]: *rec->rec_value()) {
                if (!key->apply(lhs)->isNoObj())
                  return value->apply(lhs);
              }
            } catch (const std::bad_any_cast &e) {
              LOG_EXCEPTION(e);
            }
            return Obj::to_noobj();
          },
          IType::ONE_TO_MANY);
    }

    /* static Objs_p bunion(const Rec_p rec) {
   return Obj::to_inst("union", {rec}, [rec](const Obj_p &lhs) {
     for (const auto &[key, value]: *rec->rec_value()) {
       if (!key->apply(lhs)->isNoObj())
         return value->apply(lhs);
     }
     return Obj::to_noobj();
   });
 }*/

    static Obj_p map(const BCode_p &bcode) {
      return Obj::to_inst("map", {bcode}, [bcode](const Obj_p &lhs) { return bcode->apply(lhs); }, IType::ONE_TO_ONE);
    }

    static Obj_p filter(const BCode_p &bcode) {
      return Obj::to_inst(
          "filter", {bcode}, [bcode](const Obj_p &lhs) { return bcode->apply(lhs)->isNoObj() ? Obj::to_noobj() : lhs; },
          IType::ONE_TO_MANY);
    }

    static Obj_p side(const BCode_p &bcode) {
      return Obj::to_inst(
          "side", {bcode},
          [bcode](const Obj_p &lhs) {
            bcode->apply(lhs);
            return lhs;
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p get(const Obj_p &key) {
      return Obj::to_inst("get", {key}, [key](const Obj_p &lhs) { return share((*lhs)[*key]); }, IType::ONE_TO_ONE);
    }

    static Obj_p set(const Obj_p &key, const Obj_p &value) {
      return Obj::to_inst(
          "set", {key, value},
          [key, value](const Obj_p &lhs) {
            switch (lhs->o_type()) {
              case OType::LST: {
                lhs->lst_set(key, value);
                return lhs;
              }
              case OType::REC: {
                lhs->rec_set(key, value);
                return lhs;
              }
              default:
                throw fError("Unknown obj type in []: %s\n", OTypes.toChars(lhs->o_type()));
            }
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p is(const Obj_p &xbool) {
      return Obj::to_inst(
          "is", {xbool}, [xbool](const Obj_p &lhs) { return xbool->apply(lhs)->bool_value() ? lhs : Obj::to_noobj(); },
          IType::ONE_TO_MANY);
    }

    static Obj_p noop() {
      return Obj::to_inst("noop", {}, [](const Obj_p &lhs) { return lhs; }, IType::ONE_TO_ONE);
    }


    static Bool_p neq(const Obj_p &rhs) {
      return Obj::to_inst(
          "neq", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs != *rhs->apply(lhs)); }, IType::ONE_TO_ONE);
    }

    static Bool_p eq(const Obj_p &rhs) {
      return Obj::to_inst(
          "eq", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs == *(rhs->apply(lhs))); }, IType::ONE_TO_ONE);
    }

    static Bool_p gte(const Obj_p &rhs) {
      return Obj::to_inst(
          "gte", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs >= *rhs->apply(lhs)); }, IType::ONE_TO_ONE);
    }

    static Bool_p gt(const Obj_p &rhs) {
      return Obj::to_inst(
          "gt", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs > *rhs->apply(lhs)); }, IType::ONE_TO_ONE);
    }

    static Bool_p lte(const Obj_p &rhs) {
      return Obj::to_inst(
          "gte", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs <= *rhs->apply(lhs)); }, IType::ONE_TO_ONE);
    }

    static Bool_p lt(const Obj_p &rhs) {
      return Obj::to_inst(
          "gt", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs < *rhs->apply(lhs)); }, IType::ONE_TO_ONE);
    }

    static Obj_p define(const Obj_p &typeId, const BCode_p &type) {
      return Obj::to_inst(
          "define", {typeId, type},
          [typeId, type](const Obj_p &lhs) {
            TYPE_WRITER(typeId->uri_value(), type->isNoOpBytecode() ? lhs : type);
            return lhs;
          },
          areInitialArgs(typeId, type) ? IType::ZERO_TO_ONE : IType::ONE_TO_ONE, Obj::to_noobj());
    }

    static Obj_p as(const Uri_p &typeId) {
      return Obj::to_inst(
          "as", {typeId},
          [typeId](const Obj_p &lhs) { return lhs->as(typeId->apply(lhs)->uri_value().toString().c_str()); },
          IType::ONE_TO_ONE);
    }

    static Obj_p to(const Uri_p &uri) {
      return Obj::to_inst(
          "to", {uri},
          [uri](const Obj_p &lhs) {
            RESPONSE_CODE _rc = Router::write(uri->apply(lhs)->uri_value(), lhs);
            if (_rc)
              LOG(ERROR, "%s\n", RESPONSE_CODE_STR(_rc));
            return lhs;
          },
          areInitialArgs(uri) ? IType::ZERO_TO_ONE : IType::ONE_TO_ONE);
    }

    static Obj_p from(const Uri_p &uri) {
      return Obj::to_inst(
          "from", {uri}, [uri](const Obj_p &lhs) { return Router::read(uri->apply(lhs)->uri_value()); },
          areInitialArgs(uri) ? IType::ZERO_TO_ONE : IType::ONE_TO_ONE);
    }

    static Uri_p type() {
      return Obj::to_inst("type", {}, [](const Obj_p &lhs) { return share(Uri(*lhs->id())); }, IType::ONE_TO_ONE);
    }

    static Rec_p group(const BCode_p &keyCode, const BCode_p &valueCode, const BCode_p &reduceCode) {
      return Obj::to_inst(
          "group", {keyCode, valueCode, reduceCode},
          [keyCode, valueCode, reduceCode](const Objs_p &barrier) {
            Obj::RecMap<> map = Obj::RecMap<>();
            for (const Obj_p &obj: *barrier->objs_value()) {
              const Obj_p key = keyCode->apply(obj);
              const Obj_p value = valueCode->apply(obj);
              if (map.count(key)) {
                Lst_p list = map.at(key);
                list->lst_value()->push_back(value);
              } else {
                map.insert({key, Obj::to_lst({value})});
              }
            }
            /*Obj::RecMap<> map2 = Obj::RecMap<>();
            for (const auto &pair: map) {
              map2.insert({pair.first, reduceCode->apply(pair.second)});
            }*/
            return Obj::to_rec(share(map));
          },
          IType::MANY_TO_ONE, Obj::to_objs(List_p<Obj_p>({})));
    }

    static Obj_p print(const Obj_p &toprint) {
      return Obj::to_inst(
          "print", {toprint},
          [toprint](const Obj_p &lhs) {
            const Obj_p done = toprint->apply(lhs);
            GLOBAL_OPTIONS->printer<>()->printf("%s\n", done->toString().c_str());
            return lhs;
          },
          toprint->isBytecode() ? IType::ONE_TO_ONE : IType::ZERO_TO_ONE);
    }

    static Obj_p pub(const Uri_p &target, const Obj_p &payload) {
      return Obj::to_inst(
          "pub", {target, payload},
          [target, payload](const Obj_p &lhs) {
            GLOBAL_OPTIONS->router<Router>()->publish(Message{.source = FOS_DEFAULT_SOURCE_ID,
                                                              .target = target->apply(lhs)->uri_value(),
                                                              .payload = payload->isNoOpBytecode() ? lhs : payload,
                                                              .retain = TRANSIENT_MESSAGE});
            return lhs;
          },
          areInitialArgs(target, payload) ? IType::ZERO_TO_ONE : IType::ONE_TO_ONE);
    }

    static Obj_p sub(const Uri_p &pattern, const BCode_p &onRecv) {
      return Obj::to_inst(
          "sub", {pattern, onRecv},
          [pattern, onRecv](const Obj_p &lhs) {
            if (onRecv->isNoObj()) {
              GLOBAL_OPTIONS->router<Router>()->unsubscribe(FOS_DEFAULT_SOURCE_ID, pattern->apply(lhs)->uri_value());
            } else {
              GLOBAL_OPTIONS->router<Router>()->subscribe(
                  Subscription{.mailbox = nullptr,
                               .source = FOS_DEFAULT_SOURCE_ID,
                               .pattern = pattern->apply(lhs)->uri_value(),
                               .onRecv = [onRecv](const Message_p &message) { onRecv->apply(message->payload); },
                               .onRecvBCode = onRecv});
            }
            return lhs;
          },
          areInitialArgs(pattern) ? IType::ZERO_TO_ONE : IType::ONE_TO_ONE);
    }

    static Int_p sum() {
      return Obj::to_inst(
          "sum", {},
          [](const Objs_p &lhs) {
            Obj_p current = Obj::to_noobj();
            for (const Obj_p &obj: *lhs->objs_value()) {
              if (current->isNoObj()) {
                current = obj;
              } else {
                current = share(*obj + *current);
              }
            }
            return current;
          },
          IType::MANY_TO_ONE, Obj::to_objs(List<Obj_p>{}));
    }

    static Int_p prod() {
      return Obj::to_inst(
          "prod", {},
          [](const Objs_p &lhs) {
            Obj_p current = Obj::to_noobj();
            for (const Obj_p &obj: *lhs->objs_value()) {
              if (current->isNoObj()) {
                current = obj;
              } else {
                current = share(*obj * *current);
              }
            }
            return current;
          },
          IType::MANY_TO_ONE, Obj::to_objs(List<Obj_p>{}));
    }

    static Int_p count() {
      return Obj::to_inst(
          "count", {}, [](const Objs_p &lhs) { return Obj::to_int(lhs->objs_value()->size()); }, IType::MANY_TO_ONE,
          Obj::to_objs(List<Obj_p>{}));
    }

    static Objs_p barrier(const BCode_p &bcode) {
      return Obj::to_inst(
          "barrier", {bcode},
          [bcode](const Objs_p &lhs) {
            const Obj_p obj = bcode->apply(lhs);
            return obj->isObjs() ? obj : Obj::to_objs({bcode->apply(lhs)});
          },
          IType::MANY_TO_MANY);
    }

    ///// HELPER METHODS
    static bool isBarrier(const Inst_p &inst) {
      return inst->itype() == IType::MANY_TO_MANY || inst->itype() == IType::MANY_TO_ONE;
    }
    static bool isInitial(const Inst_p &inst) {
      return inst->itype() == IType::ZERO_TO_ONE || inst->itype() == IType::ZERO_TO_MANY;
    }
    static bool areInitialArgs(const Obj_p &objA, const Obj_p &objB = Obj::to_noobj(),
                               const Obj_p &objC = Obj::to_noobj(), const Obj_p &objD = Obj::to_noobj()) {
      bool result = /*objA->isUri() ? objA->uri_value().isAbsolute() :*/ !objA->isNoOpBytecode();
      result = result && /*(objB->isUri() ? objB->uri_value().isAbsolute() :*/ !objB->isNoOpBytecode();
      result = result && /*(objC->isUri() ? objC->uri_value().isAbsolute() :*/ !objC->isNoOpBytecode();
      result = result && /*(objD->isUri() ? objD->uri_value().isAbsolute() :*/ !objD->isNoOpBytecode();
      return result;
    }

    static constexpr const char *MAP_T = "map";
    static constexpr const char *FILTER_T = "filter";
    static const Inst_p to_inst(const fURI &type, const List<Obj_p> &args) {
      if (type == INST_FURI->resolve("start") || type == INST_FURI->resolve("__"))
        return Insts::start(Objs::to_objs(args));
      if (type == INST_FURI->resolve(MAP_T))
        return Insts::map(args.at(0));
      if (type == INST_FURI->resolve(FILTER_T))
        return Insts::filter(args.at(0));
      if (type == INST_FURI->resolve("side"))
        return Insts::side(args.at(0));
      if (type == INST_FURI->resolve("count"))
        return Insts::count();
      if (type == INST_FURI->resolve("sum"))
        return Insts::sum();
      if (type == INST_FURI->resolve("prod"))
        return Insts::prod();
      if (type == INST_FURI->resolve("group"))
        return Insts::group(args.at(0), args.at(1), args.at(2));
      if (type == INST_FURI->resolve("get"))
        return Insts::get(args.at(0));
      if (type == INST_FURI->resolve("set"))
        return Insts::set(args.at(0), args.at(1));
      if (type == INST_FURI->resolve("noop"))
        return Insts::noop();
      if (type == INST_FURI->resolve("as"))
        return Insts::as(args.at(0));
      if (type == INST_FURI->resolve("define"))
        return Insts::define(args.at(0), args.at(1));
      if (type == INST_FURI->resolve("type"))
        return Insts::type();
      if (type == INST_FURI->resolve("is"))
        return Insts::is(args.at(0));
      if (type == INST_FURI->resolve("plus") || type == INST_FURI->resolve("+"))
        return Insts::plus(args.at(0));
      if (type == INST_FURI->resolve("mult"))
        return Insts::mult(args.at(0));
      if (type == INST_FURI->resolve("mod"))
        return Insts::mod(args.at(0));
      if (type == INST_FURI->resolve("eq"))
        return Insts::eq(args.at(0));
      if (type == INST_FURI->resolve("neq"))
        return Insts::neq(args.at(0));
      if (type == INST_FURI->resolve("gte"))
        return Insts::gte(args.at(0));
      if (type == INST_FURI->resolve("gt"))
        return Insts::gt(args.at(0));
      if (type == INST_FURI->resolve("lte"))
        return Insts::lte(args.at(0));
      if (type == INST_FURI->resolve("lt"))
        return Insts::lt(args.at(0));
      if (type == INST_FURI->resolve("to"))
        return Insts::to(args.at(0));
      if (type == INST_FURI->resolve("from") || type == INST_FURI->resolve("*"))
        return Insts::from(args.at(0));
      if (type == INST_FURI->resolve("pub"))
        return Insts::pub(args.at(0), args.at(1));
      if (type == INST_FURI->resolve("sub"))
        return Insts::sub(args.at(0), args.at(1));
      if (type == INST_FURI->resolve("print"))
        return Insts::print(args.at(0));
      if (type == INST_FURI->resolve("switch"))
        return Insts::bswitch(args.at(0));
      if (type == INST_FURI->resolve("explain"))
        return Insts::explain();
      if (type == INST_FURI->resolve("count"))
        return Insts::count();
      if (type == INST_FURI->resolve("barrier"))
        return Insts::barrier(args.at(0));
      /// try user defined inst
      const Obj_p userInst = Router::read<Obj>(INST_FURI->resolve(type));
      if (!userInst->isNoObj()) {
        return Obj::to_inst(
            type.name(), args,
            [userInst, args](const Obj_p &lhs) {
              int counter = 0;
              for (const Obj_p &arg: args) {
                Router::write(ID((string("_") + to_string(counter++)).c_str()), arg->apply(lhs));
              }
              const Obj_p ret = userInst->lst_value()->at(0)->apply(lhs);
              for (int i = 0; i < counter; i++) {
                Router::destroy(ID((string("_") + to_string(i)).c_str()));
              }
              return ret;
            },
            userInst->lst_value()->at(0)->itype());
      }
      throw fError("Unknown instruction: %s\n", type.toString().c_str());
    }
  };

  /*template<typename ROUTER = Router>
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

  /*template<typename ROUTER = Router>
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
} // namespace fhatos

#endif
