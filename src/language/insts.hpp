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
#include <language/obj.hpp>
#include <structure/router.hpp>
#include <util/options.hpp>
#include <utility>

namespace fhatos {
  struct Insts {
    explicit Insts() = delete;

    static Obj_p start(const Obj_p &starts) {
      return Obj::to_inst(
          "start", {starts}, [starts](const InstArgs &) { return [starts](const Obj_p &seed) { return starts; }; },
          IType::ZERO_TO_MANY, starts);
    }

    static Obj_p optional(const Obj_p &option) {
      return Obj::to_inst(
          "optional", {option},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              return args.at(0)->is_noobj() ? lhs : args.at(0);
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p lambda(const Function<Obj_p, Obj_p> &function, const Uri_p &location = vri("cpp-impl")) {
      return Obj::to_inst(
          "lambda", {location},
          [function](const InstArgs &) { return [function](const Obj_p &input) { return function(input); }; },
          IType::ONE_TO_ONE);
    }

    static Objs_p dedup(const BCode_p &projection) {
      return Obj::to_inst(
          "dedup", {projection},
          [](const InstArgs &args) {
            return [args](const Objs_p &barrier) {
              List<Obj_p> filtered;
              Set<Obj> feature;
              for (const Obj_p &obj: *barrier->objs_value()) {
                const Obj_p feat = args.at(0);
                if (!feature.count(*feat)) {
                  feature.insert(*feat);
                  filtered.push_back(obj);
                }
              }
              return objs(filtered);
            };
          },
          IType::MANY_TO_MANY, Insts::map(Obj::to_objs()));
    }

    static Objs_p insert(const Objs_p &objs) {
      return Obj::to_inst(
          "insert", {objs},
          [](const InstArgs &args) {
            return [args](const Objs_p &barrier) {
              const Objs_p objs_ = args.at(0);
              if (objs_->is_objs()) {
                for (const Obj_p &obj: *objs_->objs_value()) {
                  barrier->add_obj(obj);
                }
              } else {
                barrier->add_obj(objs_);
              }
              return barrier;
            };
          },
          IType::MANY_TO_MANY, Insts::map(Obj::to_objs()));
    }


    static Obj_p explain() {
      return Obj::to_inst(
          "explain", {}, [](const InstArgs &) { return [](const Objs_p &lhs) { return lhs; }; }, IType::ZERO_TO_ONE);
    }

    static Obj_p plus(const Obj_p &rhs) {
      return Obj::to_inst(
          "plus", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return share(*lhs + *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p mult(const Obj_p &rhs) {
      return Obj::to_inst(
          "mult", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return share(*lhs * *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p div(const Obj_p &rhs) {
      return Obj::to_inst(
          "div", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return share(*lhs / *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p mod(const Obj_p &rhs) {
      return Obj::to_inst(
          "mod", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return make_shared<Obj>(*lhs % *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p map(const BCode_p &bcode) {
      return Obj::to_inst(
          "map", {bcode},
          [](const InstArgs &args) { return [args](const Obj_p &lhs) { return args.at(0); }; },
          IType::ONE_TO_ONE);
    }

    /*static Obj_p flatmap(const BCode_p &bcode) {
      return Obj::to_inst(
              "flatmap", {bcode}, [bcode](const Obj_p &lhs) { return bcode; }, IType::ONE_TO_MANY);
    }*/

    static Obj_p filter(const BCode_p &bcode) {
      return Obj::to_inst(
          "filter", {bcode},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return args.at(0)->is_noobj() ? Obj::to_noobj() : lhs; };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p repeat(const BCode_p &code, const BCode_p &until, const BCode_p &emit) {
      return Obj::to_inst(
          "repeat", {code, until, emit},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              Objs_p r = Options::singleton()->processor<Obj>(lhs, args.at(0));
              return r;
            };
          },
          IType::ONE_TO_MANY);
    }

    static Obj_p rand(const Uri_p &type) {
      return Obj::to_inst(
          "rand", {type},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              const Obj_p applied_arg = args.at(0);
              if (applied_arg->uri_value().has_path("bool"))
                return dool(::rand() & 1);
              if (applied_arg->uri_value().has_path("int"))
                return jnt((FOS_INT_TYPE) ::rand());
              if (applied_arg->uri_value().has_path("real"))
                return real(static_cast<float>(::rand()) / (FOS_REAL_TYPE) (RAND_MAX / 1.0f));
              throw fError("%s can not be randomly generated", OTypes.to_chars(applied_arg->o_type()).c_str());
              return noobj();
            };
          },
          IType::ONE_TO_ONE);
    }

    static Int_p size() {
      return Obj::to_inst(
          "size", {},
          [](const InstArgs &) {
            return [](const Obj_p &lhs) {
              switch (lhs->o_type()) {
                case OType::LST:
                  return Obj::to_int(lhs->lst_value()->size());
                case OType::REC:
                  return Obj::to_int(lhs->rec_value()->size());
                case OType::STR:
                  return Obj::to_int(lhs->str_value().length());
                case OType::BCODE:
                  return Obj::to_int(lhs->bcode_value()->size());
                case OType::NOOBJ:
                  return Obj::to_int(0);
                default:
                  return Obj::to_int(1);
              };
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p side(const BCode_p &bcode) {
      return Obj::to_inst(
          "side", {bcode},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              args.at(0); // ?
              return lhs;
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p get(const Obj_p &key) {
      return Obj::to_inst(
          "get", {key},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return make_shared<Obj>((*lhs)[*args.at(0)]); };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p set(const Obj_p &key, const Obj_p &value) {
      return Obj::to_inst(
          "set", {key, value},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              switch (lhs->o_type()) {
                case OType::LST: {
                  lhs->lst_set(args.at(0), args.at(1));
                  return lhs;
                }
                case OType::REC: {
                  lhs->rec_set(args.at(0), args.at(1));
                  return lhs;
                }
                default:
                  throw fError("Unknown obj type in []: %s", OTypes.to_chars(lhs->o_type()).c_str());
              }
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p is(const Obj_p &xbool) {
      return Obj::to_inst(
          "is", {xbool},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return args.at(0)->bool_value() ? lhs : Obj::to_noobj(); };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p noop() {
      return Obj::to_inst(
          "noop", {}, [](const InstArgs &) { return [](const Obj_p &lhs) { return lhs; }; }, IType::ONE_TO_ONE);
    }

    static NoObj_p end() {
      return Obj::to_inst(
          "end", {}, [](const InstArgs &) { return [](const Obj_p &) { return Obj::to_noobj(); }; },
          IType::MANY_TO_ZERO);
    }

    static Bool_p neq(const Obj_p &rhs) {
      return Obj::to_inst(
          "neq", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return Obj::to_bool(*lhs != *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Bool_p eq(const Obj_p &rhs) {
      return Obj::to_inst(
          "eq", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return Obj::to_bool(*lhs == *(args.at(0))); };
          },
          IType::ONE_TO_ONE);
    }

    static Bool_p gte(const Obj_p &rhs) {
      return Obj::to_inst(
          "gte", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return Obj::to_bool(*lhs >= *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Bool_p gt(const Obj_p &rhs) {
      return Obj::to_inst(
          "gt", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return Obj::to_bool(*lhs > *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Bool_p lte(const Obj_p &rhs) {
      return Obj::to_inst(
          "lte", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return Obj::to_bool(*lhs <= *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Bool_p lt(const Obj_p &rhs) {
      return Obj::to_inst(
          "lt", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return Obj::to_bool(*lhs < *args.at(0)); };
          },
          IType::ONE_TO_ONE);
    }

    static Bool_p match(const Obj_p &rhs) {
      return Obj::to_inst(
          "match", {rhs},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return Obj::to_bool(lhs->match(args.at(0) /**/, false)); };
          },
          IType::ONE_TO_ONE);
    }

    static Rec_p build_inspect_rec(const Obj_p &lhs) {
      Rec_p rec = Obj::to_rec({
          {vri("type_id"), vri(lhs->tid())},
          {vri("type"),
           FURI_OTYPE.count(*lhs->tid())
             ? vri(OTypes.to_chars(FURI_OTYPE.at(*lhs->tid())))
             : ROUTER_READ(lhs->tid())}});
      if (lhs->vid()) {
        rec->rec_set("value_id", vri(lhs->vid()));
        const Obj_p subs = ROUTER_READ(id_p(lhs->vid()->query("sub")));
        if (!subs->is_noobj())
          rec->rec_set("subscription", subs);
      }
      // BCODE
      if (lhs->is_bcode()) {
        const Lst_p l = lst();
        for (const Inst_p &i: *lhs->bcode_value()) {
          l->lst_add(i);
        }
        rec->rec_set("value", l);
      } else if (lhs->is_int()) {
        /// INT
        rec->rec_set("value", jnt(lhs->int_value()));
        rec->rec_set("encoding", vri(STR(FOS_INT_TYPE)));
      } else if (lhs->is_inst()) {
        // INST
        rec->rec_set("op", str(lhs->inst_op()));
        const Lst_p &args_list = lst();
        for (const Obj_p &o: lhs->inst_args()) {
          args_list->lst_add(o);
        }
        rec->rec_set("args", args_list);
        //rec->rec_set("f",lhs->inst_f());
      } else if (lhs->is_real()) {
        /// REAL
        rec->rec_set("value", real(lhs->real_value()));
        rec->rec_set("value", real(lhs->real_value()));
        rec->rec_set("encoding", vri(STR(FOS_REAL_TYPE)));
      } else if (lhs->is_str()) {
        /// STR
        rec->rec_set("value", str(lhs->str_value()));
        rec->rec_set("encoding", vri(string("UTF") + to_string(8 * sizeof(char))));
      } else if (lhs->is_uri()) {
        /// URI
        const fURI furi = lhs->uri_value();
        if (furi.has_scheme())
          rec->rec_set("scheme", vri(furi.scheme()));
        if (furi.has_user())
          rec->rec_set("user", vri(furi.user()));
        if (furi.has_password())
          rec->rec_set("password", vri(furi.password()));
        if (furi.has_host())
          rec->rec_set("host", vri(furi.host()));
        if (furi.has_port())
          rec->rec_set("port", jnt(lhs->uri_value().port()));
        rec->rec_set("relative", dool(furi.is_relative()));
        rec->rec_set("branch", dool(furi.is_branch()));
        rec->rec_set("pattern", dool(furi.is_pattern()));
        if (furi.has_path()) {
          const Lst_p path = Obj::to_lst();
          for (int i = 0; i < furi.path_length(); i++) {
            path->lst_add(vri(furi.path(i)));
          }
          rec->rec_set("path", path);
        }
        if (furi.has_query()) {
          rec->rec_set("query", str(furi.query()));
        }
      }
      return rec;
    }

    static Rec_p inspect() {
      return Obj::to_inst(
          "inspect", {},
          [](const InstArgs &) {
            return [](const Obj_p &lhs) {
              return Insts::build_inspect_rec(lhs);
            };
          },
          IType::ONE_TO_ONE);
    }

    /*
        static Rec_p inspect(const Obj_p &obj) {
              return Obj::to_inst(
                  "inspect", {obj},
                  [](const InstArgs &args) {
                    return [args](const Obj_p &lhs) {
                      return Insts::build_inspect_rec(Obj::inst_arg(0, args, lhs));
                    };
                  },
                  IType::ONE_TO_ONE);
            }
     */
    static Bool_p a(const Uri_p &type_id) {
      return Obj::to_inst(
          "a", {type_id},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              return dool(TYPE_CHECKER(lhs.get(), id_p(args.at(0)->uri_value()), false));
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p as(const Uri_p &type_id) {
      return Obj::to_inst(
          "as", {type_id},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) { return lhs->as(id_p(args.at(0)->uri_value())); };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p to(const Uri_p &uri, const Bool_p &retain = dool(true)) {
      return Obj::to_inst(
          "to", {uri, retain},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              ROUTER_WRITE(furi_p(args.at(0)->uri_value()), lhs, args.at(1)->bool_value());
              return lhs;
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p to_inv(const Obj_p &obj, const Bool_p &retain = dool(true)) {
      return Obj::to_inst(
          "to_inv", {obj, retain},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              const Obj_p ret = args.at(0);
              ROUTER_WRITE(furi_p(lhs->uri_value()), ret, args.at(1)->bool_value());
              return ret;
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p from(const Uri_p &uri, const Obj_p &default_arg = noobj()) {
      return Obj::to_inst(
          "from", {uri, default_arg},
          [](const InstArgs &args) {
            return [args](const Uri_p &lhs) {
              Obj_p result = ROUTER_READ(furi_p(args.at(0)->uri_value()))->at(nullptr);
              return result->is_noobj() ? args.at(1) : result;
            };
          },
          (uri->is_uri() && uri->uri_value().is_pattern()) ? IType::ONE_TO_MANY : IType::ONE_TO_ONE);
    }

    static Uri_p type() {
      return Obj::to_inst(
          "type", {}, [](const InstArgs &) { return [](const Obj_p &lhs) { return Obj::to_uri(*lhs->tid()); }; },
          IType::ONE_TO_ONE);
    }

    static Lst_p cleave(const Str_p &delimiter) {
      return Obj::to_inst(
          "cleave", {delimiter},
          [](const InstArgs &args) {
            const Obj_p &del = args.at(0);
            return [del](const Str_p &lhs) {
              const List_p<Str_p> tokens = make_shared<List<Str_p>>();
              auto ss =
                  stringstream(lhs->is_str()
                                 ? lhs->str_value()
                                 : (lhs->is_uri() ? lhs->uri_value().toString() : lhs->toString(false)));
              string temp;
              const string delim = del->str_value();
              while (!ss.eof()) {
                if (StringHelper::look_ahead(delim, &ss)) {
                  tokens->push_back(Obj::to_str(temp));
                  temp.clear();
                } else
                  temp += ss.get();
              }
              if (!temp.empty())
                tokens->push_back(Obj::to_str(temp));
              return Obj::to_lst(tokens);
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p by(const Obj_p &by_modulator) {
      return Obj::to_inst(
          "by", {by_modulator},
          [](const InstArgs &) {
            return [](const Obj_p &) {
              if (true)
                throw fError("by()-modulations must be rewritten away");
              return noobj();
            };
          },
          IType::ONE_TO_ZERO);
    }

    static Rec_p group(const BCode_p &key_code, const BCode_p &value_code, const BCode_p &reduce_code) {
      return Obj::to_inst(
          "group", {key_code, value_code, reduce_code},
          [](const InstArgs &args) {
            return [args](const Objs_p &barrier) {
              const Obj::RecMap_p<> map = make_shared<Obj::RecMap<>>();
              for (const Obj_p &obj: *barrier->objs_value()) {
                const Obj_p key = args.at(0)->apply(obj, args);
                const Obj_p value = args.at(1)->apply(obj, args);
                if (map->count(key)) {
                  const Lst_p &list = map->at(key);
                  list->lst_value()->push_back(value);
                } else {
                  map->insert({key, Obj::to_lst({value})});
                }
              }
              /*Obj::RecMap<> map2 = Obj::RecMap<>();
              for (const auto &pair: map) {
                map2.insert({pair.first, reduceCode->apply(pair.second)});
              }*/
              return Obj::to_rec(map);
            };
          },
          IType::MANY_TO_ONE, Insts::map(Obj::to_objs()));
    }

    static Obj_p print(const Obj_p &to_print) {
      return Obj::to_inst(
          "print", {to_print},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              printer()->printf("%s\n", args.at(0)->toString().c_str());
              return lhs;
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p flip(const Obj_p &rhs) {
      return Obj::to_inst(
          "flip", {rhs},
          [](const InstArgs &args) { return [args](const Obj_p &lhs) { return lhs->apply(args.at(0), args); }; },
          IType::ONE_TO_ONE);
    }

    static Obj_p lift(const BCode_p &bcode) {
      return Obj::to_inst(
          "lift", {bcode},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              const InstList_p next_insts = make_shared<List<Inst_p>>();
              for (const Inst_p &p: *args.at(0)->bcode_value()) {
                auto next_args = List<Obj_p>();
                for (const Obj_p &obj: p->inst_args()) {
                  next_args.push_back(obj);
                }
                next_insts->push_back(
                    Obj::to_inst(p->inst_op(), next_args, p->inst_f(), p->itype(), p->inst_seed_supplier()));
              }
              return Obj::to_bcode(next_insts);
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p drop(const BCode_p &bcode) {
      return Obj::to_inst(
          "drop", {bcode},
          [](const InstArgs &args) { return [args](const Obj_p &lhs) { return args.at(0); }; },
          IType::ONE_TO_ONE);
    }

  private:
    static InstFunctionSupplier reduce_(const BiFunction<Obj_p, Obj_p, Obj_p> &bi_function) {
      return [bi_function](const InstArgs &) {
        return [bi_function](const Objs_p &lhs) {
          Obj_p current = Obj::to_noobj();
          for (const Obj_p &obj: *lhs->objs_value()) {
            if (current->is_noobj()) {
              current = obj;
            } else {
              current = bi_function(obj, current);
            }
          }
          return current;
        };
      };
    }

  public:
    static Int_p sum() {
      return Obj::to_inst("sum", {}, reduce_([](const Obj_p &a, const Obj_p &b) { return make_shared<Obj>(*b + *a); }),
                          IType::MANY_TO_ONE, Insts::map(Obj::to_objs()));
    }

    static Int_p prod() {
      return Obj::to_inst("prod", {}, reduce_([](const Obj_p &a, const Obj_p &b) { return make_shared<Obj>(*b * *a); }),
                          IType::MANY_TO_ONE, Insts::map(Obj::to_objs()));
    }

    static Obj_p foldr(const BCode_p &bcode) {
      return Obj::to_inst("foldr", {bcode},
                          reduce_([bcode](const Obj_p &a, const Obj_p &b) {
                            return (bcode->apply(b, {})->apply(a, {}));
                          }),
                          IType::MANY_TO_ONE, Insts::map(Obj::to_objs()));
    }

    static Obj_p window(const Obj_p &obj) {
      return Obj::to_inst(
          "window", {obj},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              const Obj_p &obj = args.at(0);
              List<Obj_p> ret;
              if (obj->is_lst() && lhs->is_lst()) {
                for (size_t i = 0; i <= (lhs->lst_value()->size() - obj->lst_value()->size()); i++) {
                  bool match = true;
                  List_p<Obj_p> m = make_shared<List<Obj_p>>();
                  for (size_t j = 0; j < obj->lst_value()->size(); j++) {
                    const Obj_p x = obj->lst_value()->at(j)->apply(lhs->lst_value()->at(i + j), {});
                    match = !x->is_noobj() && match;
                    if (!match)
                      break;
                    m->push_back(x);
                  }
                  if (match) {
                    ret.push_back(Obj::to_lst(m));
                  }
                }
              } else if (obj->is_lst() && lhs->is_str()) {
                for (size_t i = 0; i <= (lhs->str_value().length() - obj->lst_value()->size()); i++) {
                  bool match = true;
                  string m;
                  for (size_t j = 0; j < obj->lst_value()->size(); j++) {
                    const Obj_p x = obj->lst_value()->at(j)->apply(Obj::to_str(string() + lhs->str_value().at(i + j)),
                                                                   {});
                    match = !x->is_noobj() && match;
                    if (!match)
                      break;
                    m += x->str_value();
                  }
                  if (match) {
                    ret.push_back(Obj::to_str(m));
                  }
                }
              }
              return objs(ret);
            };
          },
          IType::ONE_TO_MANY);
    }

    static Objs_p until(const BCode_p &bcode) {
      return Obj::to_inst(
          "until", {bcode},
          [](const InstArgs &args) {
            return [args](const Objs_p &lhs) {
              Objs_p ret = objs();
              auto mini_ret = List<Obj_p>();
              for (const auto &obj: *lhs->objs_value()) {
                const Obj_p mini_obj = args.at(0)->apply(obj, {});
                mini_ret.push_back(obj);
                if (mini_obj->bool_value()) {
                  ret->add_obj(lst(List<Obj_p>(mini_ret)));
                  mini_ret.clear();
                }
              }
              ret->add_obj(lst(mini_ret));
              return ret;
            };
          },
          IType::MANY_TO_MANY, Insts::map(Obj::to_objs()));
    }

    static Bool_p x_or(const Bool_p &a, const Bool_p &b, const Bool_p &c = noobj(), const Bool_p &d = noobj()) {
      return Obj::to_inst(
          "or", {a, b, c, d},
          [](const InstArgs &args) {
            return [args](const Bool_p &lhs) {
              bool result = false; // OR starts false
              for (const Obj_p &arg: args) {
                if (!arg->is_noobj())
                  result = result || arg->bool_value();
              }
              return dool(result);
            };
          },
          IType::ONE_TO_ONE);
    }

    static Bool_p x_and(const Bool_p &a, const Bool_p &b, const Bool_p &c = noobj(), const Bool_p &d = noobj()) {
      return Obj::to_inst(
          "and", {a, b, c, d},
          [](const InstArgs &args) {
            return [args](const Bool_p &lhs) {
              bool result = true; // AND starts true
              for (const Obj_p &arg: args) {
                if (!arg->is_noobj())
                  result = result && arg->bool_value();
              }
              return dool(result);
            };
          },
          IType::ONE_TO_ONE);
    }

    static Bool_p error(const Str_p &message) {
      return Obj::to_inst(
          "error", {message},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              if (true)
                throw fError((args.at(0)->str_value()).c_str());
              return lhs;
            };
          },
          IType::ONE_TO_ZERO);
    }

    static Obj_p within(const BCode_p &bcode) {
      return Obj::to_inst(
          "within", {bcode},
          [](const InstArgs &args) {
            return [args](const Poly_p &lhs) {
              // LST BY ELEMENTS
              if (lhs->is_lst()) {
                return Obj::to_lst(Options::singleton()
                    ->processor<Obj>(Obj::to_objs(lhs->lst_value()), args.at(0))
                    ->objs_value());
              }
              // REC BY PAIRS
              if (lhs->is_rec()) {
                const Objs_p pairs = Obj::to_objs();
                for (const auto &pair: *lhs->rec_value()) {
                  pairs->add_obj(Obj::to_lst({pair.first, pair.second}));
                }
                const Objs_p results = Options::singleton()->processor<Obj>(pairs, args.at(0));
                const Obj::RecMap_p<> rec = make_shared<Obj::RecMap<>>();
                for (const auto &result: *results->objs_value()) {
                  rec->insert({result->lst_value()->at(0), result->lst_value()->at(1)});
                }
                return Obj::to_rec(rec);
              }
              // STR BY CHARS
              if (lhs->is_str()) {
                ptr<List<Str_p>> chars = make_shared<List<Str_p>>();
                const string xstr = lhs->str_value();
                for (uint8_t i = 0; i < xstr.length(); i++) {
                  chars->push_back(str(xstr.substr(i, 1)));
                }
                const Objs_p strs = Options::singleton()->processor<Obj>(Obj::to_objs(chars), args.at(0));
                string ret;
                for (const Str_p &s: *strs->objs_value()) {
                  ret += s->str_value();
                }
                return str(ret);
              }
              return noobj();
            };
          },
          IType::ONE_TO_ONE);
    }

    static Int_p count() {
      return Obj::to_inst(
          "count", {},
          [](const InstArgs &) {
            return [](const Objs_p &lhs) { return !lhs->is_objs() ? jnt(0) : Obj::to_int(lhs->objs_value()->size()); };
          },
          IType::MANY_TO_ONE, Insts::map(Obj::to_objs()));
    }

    static Obj_p delay(const Int_p &milliseconds) {
      return Obj::to_inst(
          "delay", {milliseconds},
          [](const InstArgs &args) {
            return [args](const Objs_p &lhs) {
              Process::current_process()->delay(args.at(0)->int_value());
              return lhs;
            };
          },
          IType::ONE_TO_ONE);
    }

    static Objs_p barrier(const BCode_p &bcode) {
      return Obj::to_inst(
          "barrier", {bcode},
          [](const InstArgs &args) {
            return [args](const Objs_p &lhs) {
              const Obj_p obj = args.at(0);
              return obj->is_objs() ? obj : objs({obj});
            };
          },
          IType::MANY_TO_MANY, Insts::map(Obj::to_objs()));
    }

    static Objs_p block(const Obj_p &rhs) {
      return Obj::to_inst(
          "block", {rhs}, [](const InstArgs &args) { return [args](const Objs_p &lhs) { return args.at(0); }; },
          IType::ZERO_TO_ONE);
    }

    static Poly_p split(const Poly_p &poly) {
      return Obj::to_inst(
          "split", {poly},
          [](const InstArgs &args) { return [args](const Poly_p &lhs) { return args.at(0); }; },
          IType::ONE_TO_ONE);
    }

    static Obj_p merge(const Int_p &amount) {
      return Obj::to_inst(
          "merge", {amount},
          [](const InstArgs &args) {
            return [args](const Poly_p &lhs) {
              const int amnt = args.at(0)->is_noobj() ? 10000 : args.at(0)->int_value();
              Objs_p objs = Obj::to_objs();
              if (lhs->is_lst()) {
                int counter = 0;
                for (const auto &element: *lhs->lst_value()) {
                  if (counter >= amnt)
                    break;
                  if (!element->is_noobj()) {
                    objs->add_obj(element);
                    ++counter;
                  }
                }
              } else if (lhs->is_rec()) {
                int counter = 0;
                for (const auto &[key, value]: *lhs->rec_value()) {
                  if (counter >= amnt)
                    break;
                  if (!value->is_noobj()) {
                    objs->add_obj(value);
                    ++counter;
                  }
                }
              } else {
                if (amnt > 0)
                  objs->add_obj(lhs);
              }
              return objs;
            };
          },
          IType::ONE_TO_MANY);
    }

    static Obj_p subset(const Obj_p &start, const Obj_p &end) {
      return Obj::to_inst(
          "subset", {start, end},
          [](const InstArgs &args) {
            const Obj_p &start1 = args.at(0);
            const Obj_p &end1 = args.at(1);
            return [start1, end1](const Poly_p &lhs) {
              if (lhs->is_lst()) {
                const auto sub = make_shared<List<Obj_p>>();
                const int s = start1->int_value();
                const int e = end1->int_value();
                int counter = 0;
                for (const auto &obj: *lhs->lst_value()) {
                  if (counter >= s && counter < e) {
                    sub->push_back(obj);
                  }
                  if (counter > e)
                    break;
                  ++counter;
                }
                return Obj::to_lst(sub);
              }
              return noobj();
            };
          },
          IType::ONE_TO_ONE);
    }

    static Obj_p at(const Uri_p &uri, const Obj_p &default_arg) {
      return Obj::to_inst(
          "at", {uri, default_arg},
          [](const InstArgs &args) {
            return [args](const Uri_p &lhs) {
              const ID_p at_id = id_p(args.at(0)->uri_value());
              Obj_p result = ROUTER_READ(at_id)->at(at_id);
              return result->is_noobj() ? args.at(1) : result;
            };
          },
          (uri->is_uri() && uri->uri_value().is_pattern()) ? IType::ONE_TO_MANY : IType::ONE_TO_ONE);
    }

    static Obj_p from_get(const Obj_p &key) {
      return Obj::to_inst(
          "from_get", {key},
          [](const InstArgs &args) {
            return [args](const Obj_p &lhs) {
              if (lhs->is_rec())
                return lhs->rec_get(args.at(0));
              if (lhs->is_lst())
                return lhs->lst_get(args.at(0));
              throw fError("from_get doesn't support %s", lhs->tid()->toString().c_str());
            };
          },
          IType::ONE_TO_ONE);
    }

    static Poly_p each(const Poly_p &poly) {
      return Obj::to_inst(
          "each", {poly},
          [](const InstArgs &args) {
            const Obj_p &arg = args.at(0);
            return [arg](const Poly_p &lhs) {
              const Lst_p poly2 = arg->is_lst() ? arg : arg;
              if (lhs->is_lst()) {
                Lst_p ret = Obj::to_lst();
                for (uint8_t i = 0; i < lhs->lst_value()->size(); i++) {
                  if (poly2->lst_value()->size() >= i) {
                    ret->lst_add(poly2->lst_value()->at(i)->apply(lhs->lst_value()->at(i), {}));
                  } else {
                    ret->lst_add(Obj::to_noobj()->apply(lhs->lst_value()->at(i), {}));
                  }
                }
                return ret;
              } else {
                throw fError("each() currently only supports lst poly");
              }
            };
          },
          IType::ONE_TO_ONE);
    }

  public:
    static bool are_initial_args(const Obj_p &obj_a = noobj(), const Obj_p &obj_b = noobj(),
                                 const Obj_p &obj_c = noobj(), const Obj_p &obj_d = noobj()) {
      bool result = /*objA->isUri() ? objA->uri_value().isAbsolute() :*/ !obj_a->is_noop_bcode();
      result = result && /*(objB->isUri() ? objB->uri_value().isAbsolute() :*/ !obj_b->is_noop_bcode();
      result = result && /*(objC->isUri() ? objC->uri_value().isAbsolute() :*/ !obj_c->is_noop_bcode();
      result = result && /*(objD->isUri() ? objD->uri_value().isAbsolute() :*/ !obj_d->is_noop_bcode();
      return result;
    }

    static const List<Obj_p> &arg_check(const ID_p &inst, const List<Obj_p> &args, const uint8_t expected_size) {
      if (args.size() != expected_size)
        throw fError("incorrect number of arguments provided to %s: %i != %i", inst->toString().c_str(), args.size(),
                     expected_size);
      return args;
    }

    static List<Pair<string, string>> unary_sugars() {
      static List<Pair<string, string>> map = {{"-->", "via_inv"}, {"@", "at"}, {"??", "optional"}, {"-<", "split"},
                                               {">-", "merge"}, {"~", "match"}, {"<-", "to"}, {"->", "to_inv"},
                                               {"|", "block"}, {"^", "lift"}, {"V", "drop"}, {"*", "from"},
                                               {"=", "each"}, {";", "end"}, {"\\", "from_get"}};
      return map;
    }


    static Inst_p to_inst(const ID &type_id, const List<Obj_p> &args) {
      LOG(TRACE, "Searching for inst: %s\n", type_id.toString().c_str());
      /// try user defined inst
      const ID_p type_id_resolved = type_id.path_length() > 1 || type_id.has_scheme()
                                      ? id_p(type_id)
                                      : id_p(INST_FURI->resolve(type_id));
      Obj_p base_inst = ROUTER_READ(type_id_resolved);
      if (base_inst->is_noobj()) {
        base_inst = ROUTER_READ(id_p(type_id));
        if (!base_inst->is_code())
          throw fError("unknown instruction: %s", type_id.toString().c_str());
      }
      LOG(TRACE, "located !y%s!! %s: !b%s!!\n", OTypes.to_chars(base_inst->o_type()).c_str(),
          base_inst->toString().c_str(), base_inst->tid()->toString().c_str());
      if (base_inst->is_inst())
        return ObjHelper::replace_from_inst(base_inst, args);
      if (base_inst->is_bcode()) {
        if (base_inst->bcode_value()->size() == 1)
          return ObjHelper::replace_from_inst(base_inst->bcode_value()->at(0), args);
        return Obj::to_inst(
            type_id.name(), args,
            [base_inst,args](const InstArgs &) {
              return [base_inst,args](const Obj_p &lhs) {
                InstArgs args3;
                for (const Obj_p &arg: args) {
                  args3.push_back(arg);
                }
                const Obj_p new_bcode = ObjHelper::replace_from_bcode(base_inst, args3);
                //LOG(INFO, "final bcode: %s\n", new_bcode->toString().c_str());
                return new_bcode;
              };
            },
            base_inst->itype(),
            base_inst->is_inst() ? base_inst->inst_seed_supplier() : _noobj_, // TODO
            type_id_resolved);
      }
      // return replace_from_obj(args, base_inst);
      throw fError("!b%s!! does not resolve to !yinst!! or !ybcode!!: %s", type_id.toString().c_str(),
                   base_inst->toString().c_str());
    }

  };
} // namespace fhatos

#endif