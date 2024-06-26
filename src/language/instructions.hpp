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
    static Obj_p start(const List<Obj_p> &starts) {
      return Obj::to_inst("start", starts, [](const ptr<Obj> &start) { return start; });
    }

    static Obj_p explain() {
      return Obj::to_inst("explain", {}, [](const ptr<Obj>) { return nullptr; });
    }

    static Obj_p plus(const Obj_p rhs) {
      return Obj::to_inst("plus", {rhs}, [rhs](const Obj_p &lhs) { return share(*lhs + *rhs->apply(lhs)); });
    }

    static Obj_p mult(const Obj_p rhs) {
      return Obj::to_inst("mult", {rhs}, [rhs](const Obj_p &lhs) { return share(*lhs * *rhs->apply(lhs)); });
    }

    static Obj_p mod(const Obj_p rhs) {
      return Obj::to_inst("mod", {rhs}, [rhs](const Obj_p &lhs) { return share(*lhs % *rhs->apply(lhs)); });
    }

    static Obj_p bswitch(const Obj_p rec) {
      return Obj::to_inst("switch", {rec}, [rec](const Obj_p &lhs) {
        for (const auto &pair: *rec->rec_value()) {
          if (!pair.first->apply(lhs)->isNoObj())
            return pair.second->apply(lhs);
        }
        return Obj::to_noobj();
      });
    }

    static Obj_p is(const Obj_p xbool) {
      return Obj::to_inst(
          "is", {xbool}, [xbool](const Obj_p &lhs) { return xbool->apply(lhs)->bool_value() ? lhs : Obj::to_noobj(); });
    }


    static Obj_p neq(const Obj_p rhs) {
      return Obj::to_inst("neq", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs != *rhs->apply(lhs)); });
    }

    static Obj_p eq(const Obj_p rhs) {
      return Obj::to_inst("eq", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs == *(rhs->apply(lhs))); });
    }

    static Obj_p gte(const Obj_p rhs) {
      return Obj::to_inst("gte", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs >= *rhs->apply(lhs)); });
    }

    static Obj_p gt(const Obj_p rhs) {
      return Obj::to_inst("gt", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs > *rhs->apply(lhs)); });
    }

    static Obj_p lte(const Obj_p rhs) {
      return Obj::to_inst("gte", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs <= *rhs->apply(lhs)); });
    }

    static Obj_p lt(const Obj_p rhs) {
      return Obj::to_inst("gt", {rhs}, [rhs](const Obj_p &lhs) { return Obj::to_bool(*lhs < *rhs->apply(lhs)); });
    }

    template<typename ROUTER = FOS_DEFAULT_ROUTER>
    static Obj_p define(const Obj_p uri, const Obj_p type) {
      return Obj::to_inst("define", {uri, type}, [uri, type](const Obj_p &lhs) {
        ROUTER::singleton()->publish(
            Message{.source = "123", .target = uri->uri_value(), .payload = type, .retain = RETAIN_MESSAGE});
        return lhs;
      });
    }

    template<typename ROUTER = FOS_DEFAULT_ROUTER>
    static Obj_p as(const Uri_p type) {
      return Obj::to_inst("as", {type}, [type](const Obj_p &lhs) {
        Obj_p t = ROUTER::singleton()->read("123", type->apply(lhs)->uri_value().resolve(*lhs->id()));
        if (t->apply(lhs)->isNoObj()) {
          LOG(ERROR, "[!ytyping!!] %s is not a %s[%s]\n", lhs->toString().c_str(), type->toString().c_str(),
              t->toString().c_str());
          return Obj::to_noobj();
        }
        return lhs->as(share(type->uri_value()));
      });
    }

    template<typename ROUTER = FOS_DEFAULT_ROUTER>
    static Obj_p to(const Uri_p &uri) {
      return Obj::to_inst("to", {uri}, [uri](const Obj_p &lhs) {
        RESPONSE_CODE _rc = ROUTER::singleton()->write(lhs, "123", uri->apply(lhs)->uri_value());
        if (_rc)
          LOG(ERROR, "%s\n", RESPONSE_CODE_STR(_rc));
        return lhs;
      });
    }

    template<typename ROUTER = FOS_DEFAULT_ROUTER>
    static Obj_p from(const Uri_p &uri) {
      return Obj::to_inst("from", {uri}, [uri](const Obj_p &lhs) {
        return ROUTER::singleton()->read("123", uri->apply(lhs)->uri_value());
      });
    }

    static Obj_p type(const Obj_p &rhs) {
      return Obj::to_inst("type", {rhs}, [rhs](const Obj_p &lhs) { return share(Uri(fURI(*rhs->apply(lhs)->id()))); });
    }

    template<typename PRINTER = FOS_DEFAULT_PRINTER>
    static Obj_p print(const Obj_p toprint) {
      return Obj::to_inst("print", {toprint}, [toprint](const Obj_p &lhs) {
        const Obj_p done = toprint->apply(lhs);
        PRINTER::singleton()->printf("%s\n", done->toString().c_str());
        return lhs;
      });
    }

    template<typename ROUTER = FOS_DEFAULT_ROUTER>
    static Obj_p publish(const Obj_p &target, const Obj_p &payload) {
      return Obj::to_inst("publish", {target, payload}, [target, payload](const Obj_p &lhs) {
        ROUTER::singleton()->publish(Message{.source = "123",
                                             .target = fURI(target->apply(lhs)->uri_value()),
                                             .payload = Obj_p(payload) /*->apply(incoming)*/,
                                             .retain = TRANSIENT_MESSAGE});
        return lhs;
      });
    }

    template<typename ROUTER = FOS_DEFAULT_ROUTER>
    static Obj_p subscibe(const Obj_p &pattern, const Obj_p &onRecv) {
      return Obj::to_inst("subscribe", {pattern, onRecv}, [pattern, onRecv](const Obj_p &lhs) {
        ROUTER::singleton()->subscribe(
            Subscription{.mailbox = nullptr,
                         .source = "123",
                         .pattern = fURI(pattern->apply(lhs)->uri_value()),
                         .onRecv = [onRecv](const ptr<Message> &message) {
                           const Obj_p outgoing = onRecv->apply(ptr<Obj>((Obj *) message->payload.get()));
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

    static const Inst_p to_inst(const fURI &type, const List<Obj_p> &args) {
      if (type == INST_FURI->resolve("as"))
        return Insts::as(args.at(0));
      if (type == INST_FURI->resolve("define"))
        return Insts::define(args.at(0), args.at(1));
      if (type == INST_FURI->resolve("type"))
        return Insts::type(args.at(0));
      if (type == INST_FURI->resolve("is"))
        return Insts::is(args.at(0));
      if (type == INST_FURI->resolve("plus"))
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
      if (type == INST_FURI->resolve("from"))
        return Insts::to(args.at(0));
      if (type == INST_FURI->resolve("publish"))
        return Insts::publish(args.at(0), args.at(1));
      if (type == INST_FURI->resolve("subscribe"))
        return Insts::subscibe(args.at(0), args.at(1));
      if (type == INST_FURI->resolve("print"))
        return Insts::print(args.at(0));
      if (type == INST_FURI->resolve("start"))
        return Insts::start(args);
      if (type == INST_FURI->resolve("switch"))
        return Insts::bswitch(args.at(0));
      if (type == INST_FURI->resolve("explain"))
        return Insts::explain();
      throw fError("Unknown instruction: %s\n", type.toString().c_str());
    }
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
} // namespace fhatos

#endif
