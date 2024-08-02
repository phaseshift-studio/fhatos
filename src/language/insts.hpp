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
#include <utility>

namespace fhatos {
  struct Insts {
    explicit Insts() = delete;
    static Obj_p start(const Objs_p &starts) {
      return Obj::to_inst(
          "start", {starts}, [](const Objs_p &start) { return start; }, IType::ZERO_TO_MANY,
          (starts->objs_value()->empty() ? Obj::to_noobj() : starts));
    }

    static Obj_p explain() {
      return Obj::to_inst(
          "explain", {}, [](const Objs_p &lhs) { return lhs->objs_value()->front(); }, IType::MANY_TO_ONE,
          Obj::to_objs(List<Obj_p>({})));
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

    static Obj_p flatmap(const BCode_p &bcode) {
      return Obj::to_inst(
          "flatmap", {bcode}, [bcode](const Obj_p &lhs) { return bcode->apply(lhs); }, IType::ONE_TO_MANY);
    }

    static Obj_p filter(const BCode_p &bcode) {
      return Obj::to_inst(
          "filter", {bcode}, [bcode](const Obj_p &lhs) { return bcode->apply(lhs)->isNoObj() ? Obj::to_noobj() : lhs; },
          IType::ONE_TO_ONE);
    }

    static Int_p size() {
      return Obj::to_inst(
          "size", {},
          [](const Obj_p &lhs) {
            switch (lhs->o_type()) {
              case OType::LST:
                return Obj::to_int(lhs->lst_value()->size());
              case OType::REC:
                return Obj::to_int(lhs->rec_value()->size());
              case OType::STR:
                return Obj::to_int(lhs->str_value().length());
              case OType::BCODE:
                return Obj::to_int(lhs->bcode_value().size());
              case OType::NOOBJ:
                return Obj::to_int(0);
              default:
                return Obj::to_int(1);
            }
          },
          IType::ONE_TO_ONE);
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
      return Obj::to_inst(
          "get", {key}, [key](const Obj_p &lhs) { return share((*lhs)[*key->apply(lhs)]); }, IType::ONE_TO_ONE);
    }

    static Obj_p set(const Obj_p &key, const Obj_p &value) {
      return Obj::to_inst(
          "set", {key, value},
          [key, value](const Obj_p &lhs) {
            switch (lhs->o_type()) {
              case OType::LST: {
                lhs->lst_set(key->apply(lhs), value->apply(lhs));
                return lhs;
              }
              case OType::REC: {
                lhs->rec_set(key->apply(lhs), value->apply(lhs));
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
          IType::ONE_TO_ONE);
    }

    static Obj_p noop() {
      return Obj::to_inst("noop", {}, [](const Obj_p &lhs) { return lhs; }, IType::ONE_TO_ONE);
    }

    static NoObj_p end() {
      return Obj::to_inst("end", {}, [](const Obj_p &) { return Obj::to_noobj(); }, IType::MANY_TO_ZERO);
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
            Router::write(typeId->apply(lhs)->uri_value(), type->apply(lhs));
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
            RESPONSE_CODE _rc = Router::write(uri->apply(lhs)->uri_value(), lhs->apply(uri));
            if (_rc)
              LOG(ERROR, "%s\n", RESPONSE_CODE_STR(_rc));
            return lhs;
          },
          /* areInitialArgs(uri) ? IType::ZERO_TO_ONE :*/ IType::ONE_TO_ONE);
    }

    static Obj_p to_inv(const Obj_p &obj) {
      return Obj::to_inst(
          "to_inv", {obj},
          [obj](const Obj_p &lhs) {
            RESPONSE_CODE _rc = Router::write(lhs->apply(obj)->uri_value(), obj->apply(lhs));
            if (_rc)
              LOG(ERROR, "%s\n", RESPONSE_CODE_STR(_rc));
            return obj;
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p both(const Uri_p &uri) {
      return Obj::to_inst(
          "both", {uri},
          [uri](const Obj_p &lhs) {
            Router::write(uri->apply(lhs)->uri_value(), lhs->apply(uri));
            Router::write(lhs->apply(uri)->uri_value(), uri->apply(lhs));
            return lhs;
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p from(const Uri_p &uri) {
      return Obj::to_inst(
          "from", {uri}, [uri](const Uri_p &lhs) { return Router::read(uri->apply(lhs)->uri_value()); },
          /*areInitialArgs(uri) ? IType::ZERO_TO_ONE :*/ IType::ONE_TO_ONE);
    }

    static Rec_p rfrom(const Uri_p &uri) {
      return Obj::to_inst(
          "rfrom", {uri}, [uri](const Uri_p &lhs) { return Router::readPattern(uri->apply(lhs)->uri_value()); },
          areInitialArgs(uri) ? IType::ZERO_TO_ONE : IType::ONE_TO_ONE);
    }

    static Uri_p type() {
      return Obj::to_inst("type", {}, [](const Obj_p &lhs) { return share(Uri(*lhs->id())); }, IType::ONE_TO_ONE);
    }

    static Obj_p by(const Obj_p &bymod) {
      return Obj::to_inst(
          "by", {bymod},
          [](const Obj_p &THROW_ERROR) {
            if (true)
              throw fError("by()-modulations are to be rewritten away");
            return THROW_ERROR;
          },
          IType::ONE_TO_ONE);
    }

    static Rec_p group(const BCode_p &keyCode, const BCode_p &valueCode, const BCode_p &reduceCode) {
      return Obj::to_inst(
          "group", {keyCode, valueCode, reduceCode},
          [keyCode, valueCode, reduceCode](const Objs_p &barrier) {
            Obj::RecMap<> map = Obj::RecMap<>();
            for (const Obj_p &obj: *barrier->objs_value()) {
              const Obj_p key = keyCode->isNoObj() ? obj : keyCode->apply(obj);
              const Obj_p value = valueCode->isNoObj() ? obj : valueCode->apply(obj);
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
          IType::MANY_TO_ONE, Obj::to_objs());
    }

    static Obj_p print(const Obj_p &toprint) {
      return Obj::to_inst(
          "print", {toprint},
          [toprint](const Obj_p &lhs) {
            const Obj_p done = toprint->apply(lhs);
            Options::singleton()->printer<>()->printf("%s\n", done->toString().c_str());
            return lhs;
          },
          toprint->isBytecode() ? IType::ONE_TO_ONE : IType::ZERO_TO_ONE);
    }

    static Obj_p flip(const Obj_p &rhs) {
      return Obj::to_inst("flip", {rhs}, [rhs](const Obj_p &lhs) { return lhs->apply(rhs); }, IType::ONE_TO_ONE);
    }

    static Obj_p pub(const Uri_p &target, const Obj_p &payload) {
      return Obj::to_inst(
          "pub", {target, payload},
          [target, payload](const Obj_p &lhs) {
            Options::singleton()->router<Router>()->publish(Message{.source = FOS_DEFAULT_SOURCE_ID,
                                                                    .target = target->apply(lhs)->uri_value(),
                                                                    .payload = payload->apply(lhs),
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
              Options::singleton()->router<Router>()->unsubscribe(FOS_DEFAULT_SOURCE_ID, pattern->apply(lhs)->uri_value());
            } else {
               Options::singleton()->router<Router>()->subscribe(
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

    static Obj_p window(const Obj_p &obj) {
      return Obj::to_inst(
          "window", {obj},
          [obj](const Obj_p &lhs) {
            List<Obj_p> ret;
            if (obj->isLst() && lhs->isLst()) {
              for (size_t i = 0; i <= (lhs->lst_value()->size() - obj->lst_value()->size()); i++) {
                bool match = true;
                List<Obj_p> m;
                for (size_t j = 0; j < obj->lst_value()->size(); j++) {
                  const Obj_p x = obj->lst_value()->at(j)->apply(lhs->lst_value()->at(i + j));
                  match = !x->isNoObj() && match;
                  if (!match)
                    break;
                  m.push_back(x);
                }
                if (match) {
                  ret.push_back(Obj::to_lst(share(m)));
                }
              }
            }
            return Obj::to_objs(ret);
          },
          IType::ONE_TO_MANY);
    }

    static Obj_p within(const BCode_p &) {
      return nullptr;
      /*return Obj::to_inst(
         "within", {}, [](const Objs_p &lhs) { return Obj::to_int(lhs->objs_value()->size()); }, IType::MANY_TO_ONE,
         Obj::to_objs(List<Obj_p>{}));*/
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
            return obj->isObjs() ? obj : Obj::to_objs({obj});
          },
          IType::MANY_TO_MANY);
    }

    static Objs_p block(const Obj_p &rhs) {
      return Obj::to_inst("block", {rhs}, [rhs](const Objs_p &) { return rhs; }, IType::ONE_TO_ONE);
    }

  private:
    static Obj_p embed_function(const Uri_p &lhs, const Obj_p &rhs) {
      if (rhs->isLst()) {
        Router::write(lhs->uri_value(), rhs);
        const Lst_p lst2 = rhs->apply(lhs);
        for (size_t i = 0; i < lst2->lst_value()->size(); i++) {
          const Uri_p u = Obj::to_uri(fURI(string("_") + std::to_string(i)))->apply(lhs);
          Router::write(u->uri_value(), lst2->lst_value()->at(i));
        }
        return lst2;
      } else if (rhs->isRec()) {
        const Obj::LstList_p<> links = share(Obj::LstList<>());
        const Obj::RecMap_p<> rec2 = share(Obj::RecMap<>());
        for (const auto &[key, val]: *rhs->rec_value()) {
          const Obj_p key2 = key->apply(lhs);
          const Obj_p val2 = val->apply(lhs);
          links->push_back(key2);
          rec2->insert({key2, val2});
          if (key2->isUri())
            Router::write(key2->uri_value(), val2);
        }
        Router::write(lhs->uri_value(), Obj::to_rec(rec2));
        return Obj::to_rec(rec2);
      } else {
        const Obj_p o = rhs->apply(lhs);
        Router::write(o->isUri() ? o->apply(lhs)->uri_value() : lhs->uri_value(), o);
        return o;
      }
    };

  public:
    static Obj_p embed(const Obj_p &rhs) {
      return Obj::to_inst(
          "embed", {rhs}, [rhs](const Uri_p &lhs) { return embed_function(lhs, rhs); }, IType::ONE_TO_ONE);
    }

    static Obj_p embed_inv(const Obj_p &rhs) {
      return Obj::to_inst(
          "embed_inv", {rhs}, [rhs](const Uri_p &lhs) { return embed_function(rhs, lhs); }, IType::ONE_TO_ONE);
    }

    ///// HELPER METHODS
    static bool isBarrier(const Inst_p &inst) {
      return inst->itype() == IType::MANY_TO_MANY || inst->itype() == IType::MANY_TO_ONE;
    }
    static bool isInitial(const Inst_p &inst) {
      return inst->itype() == IType::ZERO_TO_ONE || inst->itype() == IType::ZERO_TO_MANY;
    }
    static bool isTerminal(const Inst_p &inst) {
      return inst->itype() == IType::ONE_TO_ZERO || inst->itype() == IType::MANY_TO_ZERO;
    }

    static bool areInitialArgs(const Obj_p &objA, const Obj_p &objB = Obj::to_noobj(),
                               const Obj_p &objC = Obj::to_noobj(), const Obj_p &objD = Obj::to_noobj()) {
      bool result = /*objA->isUri() ? objA->uri_value().isAbsolute() :*/ !objA->isNoOpBytecode();
      result = result && /*(objB->isUri() ? objB->uri_value().isAbsolute() :*/ !objB->isNoOpBytecode();
      result = result && /*(objC->isUri() ? objC->uri_value().isAbsolute() :*/ !objC->isNoOpBytecode();
      result = result && /*(objD->isUri() ? objD->uri_value().isAbsolute() :*/ !objD->isNoOpBytecode();
      return result;
    }

    static const List<Obj_p> &argCheck(const ID &opcode, const List<Obj_p> &args, const uint8_t expectedSize) {
      if (args.size() != expectedSize)
        throw fError("Incorrect number of arguments provided to %s: %i != %i\n", opcode.toString().c_str(), args.size(),
                     expectedSize);
      return args;
    }

    static Map<string, string> unarySugars() {
      static Map<string, string> map = {{"*", "from"}, {"~>", "embed"},  {"<~", "embed_inv"}, {"<->", "both"},
                                        {"<-", "to"},  {"->", "to_inv"}, {"|", "block"}};
      return map;
    }

    static Map<ID, Function<List<Obj_p>, Inst_p>> *INSTS_MAP() {
      static Map<ID, Function<List<Obj_p>, Inst_p>> map = Map<ID, Function<List<Obj_p>, Inst_p>>();
      return &map;
    }
    static void register_inst(const ID &typeId, const Function<List<Obj_p>, Inst_p> &func) {
      INSTS_MAP()->insert({typeId, func});
      LOG(INFO, "Instruction registered: %s\n", typeId.toString().c_str());
      ID shortID = INST_FURI->resolve(typeId.name());
      if (!INSTS_MAP()->count(shortID)) {
        INSTS_MAP()->insert({shortID, func});
        LOG(INFO, FOS_TAB_4 "Shorthand registered: !b%s!!\n", shortID.toString().c_str());
      } else {
        LOG(WARN, FOS_TAB_4 "Unable to register shorthand: !b%s!!\n", shortID.toString().c_str());
      }
    }
    static Map<ID, Function<List<Obj_p>, Inst_p>> *CORE_INSTS() {
      static Map<ID, Function<List<Obj_p>, Inst_p>> core_insts = {
          {INST_FURI->resolve("start"), [](const List<Obj_p> &args) { return start(Objs::to_objs(args)); }},
          {INST_FURI->resolve("__"), [](const List<Obj_p> &args) { return start(Objs::to_objs(args)); }},
          {INST_FURI->resolve("end"), [](const List<Obj_p> &) { return end(); }},
          {INST_FURI->resolve("map"), [](const List<Obj_p> &args) { return map(argCheck("map", args, 1).at(0)); }},
          {INST_FURI->resolve("filter"),
           [](const List<Obj_p> &args) { return filter(argCheck("filter", args, 1).at(0)); }},
          {INST_FURI->resolve("side"), [](const List<Obj_p> &args) { return side(argCheck("side", args, 1).at(0)); }},
          {INST_FURI->resolve("count"),
           [](const List<Obj_p> &args) {
             argCheck("count", args, 0);
             return count();
           }},
          {INST_FURI->resolve("sum"),
           [](const List<Obj_p> &args) {
             argCheck("sum", args, 0);
             return sum();
           }},
          {INST_FURI->resolve("plus"), [](const List<Obj_p> &args) { return plus(argCheck("plus", args, 1).at(0)); }}};
      return &core_insts;
    }
    static Inst_p to_inst(const ID &typeId, const List<Obj_p> &args) {
      LOG(TRACE, "Searching for inst: %s\n", typeId.toString().c_str());
      if (typeId == INST_FURI->resolve("start") || typeId == INST_FURI->resolve("__"))
        return Insts::start(Objs::to_objs(args));
      if (typeId == INST_FURI->resolve("end") || typeId == INST_FURI->resolve(";"))
        return Insts::end();
      if (typeId == INST_FURI->resolve("map"))
        return Insts::map(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("filter"))
        return Insts::filter(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("side"))
        return Insts::side(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("count"))
        return Insts::count();
      if (typeId == INST_FURI->resolve("sum"))
        return Insts::sum();
      if (typeId == INST_FURI->resolve("prod"))
        return Insts::prod();
      if (typeId == INST_FURI->resolve("group"))
        return Insts::group(args.empty() ? Obj::to_noobj() : args.at(0), args.size() < 2 ? Obj::to_noobj() : args.at(1),
                            args.size() < 3 ? Obj::to_noobj() : args.at(2));
      if (typeId == INST_FURI->resolve("get"))
        return Insts::get(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("set"))
        return Insts::set(argCheck(typeId, args, 2).at(0), args.at(1));
      if (typeId == INST_FURI->resolve("noop"))
        return Insts::noop();
      if (typeId == INST_FURI->resolve("as"))
        return Insts::as(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("by"))
        return Insts::by(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("define"))
        return Insts::define(argCheck(typeId, args, 2).at(0), args.at(1));
      if (typeId == INST_FURI->resolve("type"))
        return Insts::type();
      if (typeId == INST_FURI->resolve("is"))
        return Insts::is(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("plus") || typeId == INST_FURI->resolve("+"))
        return Insts::plus(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("mult"))
        return Insts::mult(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("mod"))
        return Insts::mod(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("eq"))
        return Insts::eq(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("neq"))
        return Insts::neq(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("gte"))
        return Insts::gte(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("gt"))
        return Insts::gt(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("lte"))
        return Insts::lte(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("lt"))
        return Insts::lt(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("both") || typeId == INST_FURI->resolve("<->"))
        return Insts::both(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("to") || typeId == INST_FURI->resolve("<-"))
        return Insts::to(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("to_inv") || typeId == INST_FURI->resolve("->"))
        return Insts::to_inv(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("from") || typeId == INST_FURI->resolve("*"))
        return Insts::from(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("rfrom") || typeId == INST_FURI->resolve("r*"))
        return Insts::rfrom(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("pub"))
        return Insts::pub(argCheck(typeId, args, 2).at(0), args.at(1));
      if (typeId == INST_FURI->resolve("flip"))
        return Insts::flip(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("sub"))
        return Insts::sub(argCheck(typeId, args, 2).at(0), args.at(1));
      if (typeId == INST_FURI->resolve("within"))
        return Insts::within(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("print"))
        return Insts::print(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("switch"))
        return Insts::bswitch(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("explain"))
        return Insts::explain();
      if (typeId == INST_FURI->resolve("count"))
        return Insts::count();
      if (typeId == INST_FURI->resolve("size"))
        return Insts::size();
      if (typeId == INST_FURI->resolve("barrier"))
        return Insts::barrier(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("block"))
        return Insts::block(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("embed") || typeId == INST_FURI->resolve("~>"))
        return Insts::embed(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("embed_inv") || typeId == INST_FURI->resolve("<~"))
        return Insts::embed_inv(argCheck(typeId, args, 1).at(0));
      if (typeId == INST_FURI->resolve("window"))
        return Insts::window(argCheck(typeId, args, 1).at(0));
      // check registered instructions
      if (INSTS_MAP()->count(typeId))
        return INSTS_MAP()->at(typeId)(args);
      /// try user defined inst
      const Obj_p userInstBCode = Router::read<Obj>(INST_FURI->resolve(typeId));
      if (userInstBCode->isNoObj()) {
        throw fError("Unknown instruction: %s\n", typeId.toString().c_str());
      }
      if (userInstBCode->isBytecode()) {
        return Obj::to_inst(
            typeId.name(), args,
            [userInstBCode, args](const Obj_p &lhs) {
              int counter = 0;
              for (const Obj_p &arg: args) {
                Router::write(ID((string("_") + to_string(counter++)).c_str()), arg->apply(lhs));
              }
              const Obj_p ret = userInstBCode->apply(lhs);
              for (int i = 0; i < counter; i++) {
                Router::destroy(ID((string("_") + to_string(i)).c_str()));
              }
              return ret;
            },
            userInstBCode->bcode_value().front()->itype());
      } else {
        throw fError("!b%s!! does not resolve to bytecode: %s\n", typeId.toString().c_str(),
                     userInstBCode->toString().c_str());
      }
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
