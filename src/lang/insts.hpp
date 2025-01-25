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

#include "../fhatos.hpp"
#include "obj.hpp"
#include "../structure/router.hpp"
#include "../util/options.hpp"
#include <utility>

#include "mmadt/type.hpp"

namespace fhatos {
  struct Insts {
    explicit Insts() = delete;

  /*
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





        static Obj_p div(const Obj_p &rhs) {
          return Obj::to_inst(
              "div", {rhs},
              [](const InstArgs &args) {
                return [args](const Obj_p &lhs) { return share(*lhs / *args.at(0)); };
              },
              IType::ONE_TO_ONE);
        }



        /*static Obj_p flatmap(const BCode_p &bcode) {
          return Obj::to_inst(
                  "flatmap", {bcode}, [bcode](const Obj_p &lhs) { return bcode; }, IType::ONE_TO_MANY);
        }*/
    /*
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
                  throw fError("{} can not be randomly generated", OTypes.to_chars(applied_arg->o_type()).c_str());
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
                      throw fError("Unknown obj type in []: {}", OTypes.to_chars(lhs->o_type()).c_str());
                  }
                };
              },
              IType::ONE_TO_ONE);
        }



        static Obj_p noop() {
          return Obj::to_inst(
              "noop", {}, [](const InstArgs &) { return [](const Obj_p &lhs) { return lhs; }; }, IType::ONE_TO_ONE);
        }





        static Bool_p match(const Obj_p &rhs) {
          return Obj::to_inst(
              "match", {rhs},
              [](const InstArgs &args) {
                return [args](const Obj_p &lhs) { return Obj::to_bool(lhs->match(args.at(0)  false)); };
              },
              IType::ONE_TO_ONE);
        }




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
                  return Obj::to_rec(map);
                };
              },
              IType::MANY_TO_ONE, Insts::map(Obj::to_objs()));
        }


        static Obj_p flip(const Obj_p &rhs) {
          return Obj::to_inst(
              "flip", {rhs},
              [](const Obj_p &lhs, const InstArgs &args) { return lhs->apply(args.at(0), args); },
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



        static Obj_p from_get(const Obj_p &key) {
          return Obj::to_inst(
              "from_get", {key},
              [](const InstArgs &args) {
                return [args](const Obj_p &lhs) {
                  if (lhs->is_rec())
                    return lhs->rec_get(args.at(0));
                  if (lhs->is_lst())
                    return lhs->lst_get(args.at(0));
                  throw fError("from_get doesn't support {}", lhs->tid_->toString().c_str());
                };
              },
              IType::ONE_TO_ONE);
        }


    */
  };
} // namespace fhatos

#endif
