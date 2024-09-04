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
#include <structure/router.hpp>
#include <util/options.hpp>
#include <utility>

namespace fhatos {
  struct Insts {
    explicit Insts() = delete;

    [[maybe_unused]] static Inst_p x(const uint8_t arg_num, const Obj_p &defaultArg = noobj()) {
      return Insts::from(Obj::to_uri(string("_") + to_string(arg_num)), defaultArg);
    }

    static Obj_p start(const Obj_p &starts) {
      return Obj::to_inst(
        "start", {starts}, [](const InstArgs &) {
          return [](const Obj_p &seed) {
            return seed;
          };
        }, IType::ZERO_TO_MANY,
        [starts](const Obj_p &) { return starts; });
    }

    static Objs_p dedup(const BCode_p &projection) {
      return Obj::to_inst("dedup", {projection}, [](const InstArgs &args) {
        return [args](const Objs_p &barrier) {
          List<Obj_p> filtered;
          Set<Obj> feature;
          for (const Obj_p obj: *barrier->objs_value()) {
            const Obj_p feat = args.at(0)->apply(obj);
            if (!feature.contains(*feat)) {
              feature.insert(*feat);
              filtered.push_back(obj);
            }
          }
          return objs(filtered);
        };
      }, IType::MANY_TO_MANY, Obj::objs_seed());
    }

    static Objs_p insert(const Objs_p &objs) {
      return Obj::to_inst("insert", {objs}, [](const InstArgs &args) {
        return [args](const Objs_p &barrier) {
          const Objs_p objs = args.at(0)->apply(barrier);
          if (objs->is_objs()) {
            for (const Obj_p &obj: *objs->objs_value()) {
              barrier->add_obj(obj);
            }
          } else {
            barrier->add_obj(objs);
          }
          return barrier;
        };
      }, IType::MANY_TO_MANY, Objs::objs_seed());
    }


    static Obj_p explain() {
      return Obj::to_inst("explain", {}, [](const InstArgs &) { return [](const Objs_p &lhs) { return lhs; }; },
                          IType::ZERO_TO_ONE);
    }

    static Obj_p plus(const Obj_p &rhs) {
      return Obj::to_inst(
        "plus", {rhs},
        [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return share(*lhs + *args.at(0)->apply(lhs));
          };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p mult(const Obj_p &rhs) {
      return Obj::to_inst(
        "mult", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return share(*lhs * *args.at(0)->apply(lhs));
          };
        }, IType::ONE_TO_ONE);
    }

    static Obj_p mod(const Obj_p &rhs) {
      return Obj::to_inst(
        "mod", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return share(*lhs % *args.at(0)->apply(lhs));
          };
        }, IType::ONE_TO_ONE);
    }

    static Obj_p bswitch(const Rec_p &rec) {
      return Obj::to_inst(
        "switch", {rec},
        [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            try {
              for (const auto &[key, value]: *args.at(0)->rec_value()) {
                if (!key->apply(lhs)->is_noobj())
                  return value->apply(lhs);
              }
            } catch (const std::bad_any_cast &e) {
              LOG_EXCEPTION(e);
            }
            return Obj::to_noobj();
          };
        },
        IType::ONE_TO_MANY);
    }

    /* static Objs_p bunion(const Rec_p rec) {
   return Obj::to_inst("union", {rec}, [rec](const Obj_p &lhs) {
     for (const auto &[key, value]: *rec->rec_value()) {
       if (!key->apply(lhs)->is_noobj())
         return value->apply(lhs);
     }
     return Obj::to_noobj();
   });
 }*/

    static Obj_p map(const BCode_p &bcode) {
      return Obj::to_inst("map", {bcode}, [](const InstArgs &args) {
        return [args](const Obj_p &lhs) {
          return args.at(0)->apply(lhs);
        };
      }, IType::ONE_TO_ONE);
    }

    /*static Obj_p flatmap(const BCode_p &bcode) {
      return Obj::to_inst(
              "flatmap", {bcode}, [bcode](const Obj_p &lhs) { return bcode->apply(lhs); }, IType::ONE_TO_MANY);
    }*/

    static Obj_p filter(const BCode_p &bcode) {
      return Obj::to_inst(
        "filter", {bcode},
        [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return args.at(0)->apply(lhs)->is_noobj() ? Obj::to_noobj() : lhs;
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
            args.at(0)->apply(lhs);
            return lhs;
          };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p get(const Obj_p &key) {
      return Obj::to_inst(
        "get", {key},
        [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return share((*lhs)[*args.at(0)->apply(lhs)]);
          };
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
                lhs->lst_set(args.at(0)->apply(lhs), args.at(1)->apply(lhs));
                return lhs;
              }
              case OType::REC: {
                lhs->rec_set(args.at(0)->apply(lhs), args.at(1)->apply(lhs));
                return lhs;
              }
              default:
                throw fError("Unknown obj type in []: %s\n", OTypes.toChars(lhs->o_type()).c_str());
            }
          };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p is(const Obj_p &xbool) {
      return Obj::to_inst(
        "is", {xbool},
        [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return args.at(0)->apply(lhs)->bool_value() ? lhs : Obj::to_noobj();
          };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p noop() {
      return Obj::to_inst("noop", {}, [](const InstArgs &) { return [](const Obj_p &lhs) { return lhs; }; },
                          IType::ONE_TO_ONE);
    }

    static NoObj_p end() {
      return Obj::to_inst("end", {}, [](const InstArgs &) {
                            return [](const Obj_p &) { return Obj::to_noobj(); };
                          },
                          IType::MANY_TO_ZERO);
    }

    static Bool_p neq(const Obj_p &rhs) {
      return Obj::to_inst(
        "neq", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return Obj::to_bool(*lhs != *args.at(0)->apply(lhs));
          };
        },
        IType::ONE_TO_ONE);
    }

    static Bool_p eq(const Obj_p &rhs) {
      return Obj::to_inst(
        "eq", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return Obj::to_bool(*lhs == *(args.at(0)->apply(lhs)));
          };
        },
        IType::ONE_TO_ONE);
    }

    static Bool_p gte(const Obj_p &rhs) {
      return Obj::to_inst(
        "gte", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return Obj::to_bool(*lhs >= *args.at(0)->apply(lhs));
          };
        },
        IType::ONE_TO_ONE);
    }

    static Bool_p gt(const Obj_p &rhs) {
      return Obj::to_inst(
        "gt", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return Obj::to_bool(*lhs > *args.at(0)->apply(lhs));
          };
        },
        IType::ONE_TO_ONE);
    }

    static Bool_p lte(const Obj_p &rhs) {
      return Obj::to_inst(
        "gte", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return Obj::to_bool(*lhs <= *args.at(0)->apply(lhs));
          };
        },
        IType::ONE_TO_ONE);
    }

    static Bool_p lt(const Obj_p &rhs) {
      return Obj::to_inst(
        "gt", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return Obj::to_bool(*lhs < *args.at(0)->apply(lhs));
          };
        },
        IType::ONE_TO_ONE);
    }

    static Bool_p match(const Obj_p &rhs) {
      return Obj::to_inst(
        "match", {rhs}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return Obj::to_bool(lhs->match(args.at(0)->apply(lhs)));
          };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p as(const Uri_p &typeId) {
      return Obj::to_inst(
        "as", {typeId},
        [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return lhs->as(args.at(0)->apply(lhs)->uri_value().toString().c_str());
          };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p to(const Uri_p &uri) {
      return Obj::to_inst(
        "to", {uri},
        [](const InstArgs &args) {
          const Obj_p &uri = args.at(0);
          return [uri](const Obj_p &lhs) {
            RESPONSE_CODE _rc = OK;
            const Obj_p main_apply = lhs->apply(uri);
            const Uri_p uri_apply = uri->apply(lhs);
            if (uri_apply->uri_value().is_pattern()) {
              if (main_apply->is_rec()) {
                Obj::RecMap_p<> remaining = share(Obj::RecMap<>());
                for (const auto &[k, v]: *main_apply->rec_value()) {
                  if (k->is_uri())
                    Insts::to(k)->inst_f()({k})(v); // recursive embedding relative to base uri
                  else
                    remaining->insert(
                      {PtrHelper::clone(k), PtrHelper::clone(v)});
                  // non-uri key/values written to base uri
                }
                if (!remaining->empty()) {
                  const Rec_p sub_rec = Obj::to_rec(remaining);
                  _rc = router()->write(id_p(uri_apply->uri_value().retract_pattern()), sub_rec);
                }
              } else {
                _rc = router()->write(id_p(uri_apply->uri_value().retract_pattern()), main_apply);
              }
            } else {
              _rc = router()->write(id_p(uri_apply->uri_value()), main_apply);
            }
            if (_rc)
              LOG(ERROR, "%s\n", ResponseCodes.toChars(_rc).c_str());
            return main_apply;
          };
        },
        /* areInitialArgs(uri) ? IType::ZERO_TO_ONE :*/ IType::ONE_TO_ONE);
    }

    static Obj_p to_inv(const Obj_p &obj) {
      return Obj::to_inst(
        "to_inv", {obj},
        [](const InstArgs &args) {
          return
              [args](const Obj_p &lhs) { return Insts::to(lhs)->inst_f()({lhs})(args.at(0)); };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p from(const Uri_p &uri, const Obj_p &defaultArg = noobj()) {
      return Obj::to_inst(
        "from", {uri, defaultArg},
        [](const InstArgs &args) {
          return [args](const Uri_p &lhs) {
            Obj_p result = router()->read(furi_p(args.at(0)->apply(lhs)->uri_value()));
            return result->is_noobj() ? args.at(1)->apply(lhs) : result;
          };
        },
        (uri->is_uri() && uri->uri_value().is_pattern()) ? IType::ONE_TO_MANY : IType::ONE_TO_ONE);
    }

    static Uri_p type() {
      return Obj::to_inst("type", {},
                          [](const InstArgs &) {
                            return [](const Obj_p &lhs) { return share(Uri(*lhs->id())); };
                          },
                          IType::ONE_TO_ONE);
    }

    static Lst_p cleave(const Str_p &delimiter) {
      return Obj::to_inst(
        "cleave", {delimiter},
        [](const InstArgs &args) {
          const Obj_p &delimiter = args.at(0);
          return [delimiter](const Str_p &lhs) {
            List_p<Str_p> tokens = share(List<Str_p>());
            stringstream ss = stringstream(
              lhs->is_str()
                ? lhs->str_value()
                : (lhs->is_uri()
                     ? lhs->uri_value().toString()
                     : lhs->toString(false)));
            string temp;
            string delim = delimiter->str_value();
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
          return [](const Obj_p &THROW_ERROR) {
            if (true)
              throw fError("by()-modulations must be rewritten away");
            return THROW_ERROR;
          };
        },
        IType::ONE_TO_ONE);
    }

    static Rec_p group(
      const BCode_p &keyCode,
      const BCode_p &valueCode,
      const BCode_p &reduceCode) {
      return Obj::to_inst(
        "group", {keyCode, valueCode, reduceCode},
        [](const InstArgs &args) {
          return [args](const Objs_p &barrier) {
            Obj::RecMap<> map = Obj::RecMap<>();
            for (const Obj_p &obj: *barrier->objs_value()) {
              const Obj_p key = args.at(0)->apply(obj);
              const Obj_p value = args.at(1)->apply(obj);
              if (map.count(key)) {
                const Lst_p &list = map.at(key);
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
          };
        },
        IType::MANY_TO_ONE, Obj::objs_seed());
    }

    static Obj_p print(const Obj_p &to_print) {
      return Obj::to_inst(
        "print", {to_print},
        [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            printer()->printf("%s\n", args.at(0)->apply(lhs)->toString().c_str());
            return lhs;
          };
        },
        to_print->is_bcode() ? IType::ONE_TO_ONE : IType::ZERO_TO_ONE);
    }

    static Obj_p flip(const Obj_p &rhs) {
      return Obj::to_inst("flip", {rhs}, [](const InstArgs &args) {
        return [args](const Obj_p &lhs) {
          return lhs->apply(args.at(0));
        };
      }, IType::ONE_TO_ONE);
    }

    static Obj_p pub(const Uri_p &target, const Obj_p &payload, const Bool_p &transient) {
      return Obj::to_inst(
        "pub", {target, payload, transient},
        [](const InstArgs &args) {
          const Uri_p &target = args.at(0);
          const Obj_p &payload = args.at(1);
          const Bool_p &transient = args.at(2);
          return [target, payload, transient](const Obj_p &lhs) {
            router()->route_message(share(Message{
              .source = FOS_DEFAULT_SOURCE_ID,
              .target = target->apply(lhs)->uri_value(),
              .payload = payload->apply(lhs),
              .retain = transient->apply(lhs)->bool_value()
            }));
            return lhs;
          };
        },
        areInitialArgs(target, payload, transient) ? IType::ZERO_TO_ONE : IType::ONE_TO_ONE);
    }

    static Obj_p lift(const BCode_p &bcode) {
      return Obj::to_inst(
        "lift", {bcode},
        [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            InstList_p nextInsts = share(List<Inst_p>());
            for (const Inst_p &p: *args.at(0)->bcode_value()) {
              List<Obj_p> nextArgs = List<Obj_p>();
              for (const Obj_p &obj: p->inst_args()) {
                nextArgs.push_back(obj->apply(lhs));
              }
              nextInsts->push_back(Obj::to_inst(p->inst_op(), nextArgs, p->inst_f(), p->itype(),
                                                p->inst_seed_supplier()));
            }
            return Obj::to_bcode(nextInsts);
          };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p drop(const BCode_p &bcode) {
      return Obj::to_inst(
        "drop", {bcode}, [](const InstArgs &args) {
          return [args](const Obj_p &lhs) {
            return args.at(0)->apply(lhs)->apply(lhs);
          };
        },
        IType::ONE_TO_ONE);
    }

    static Obj_p sub(const Uri_p &pattern, const BCode_p &on_recv) {
      return Obj::to_inst(
        "sub", {pattern, on_recv},
        [](const InstArgs &args) {
          const Uri_p &pattern = args.at(0);
          const BCode_p &on_recv = args.at(1);
          return [pattern, on_recv](const Obj_p &lhs) {
            const Uri_p pattern_A = pattern->apply(lhs);
            const BCode_p on_recv_A = on_recv->apply(lhs);
            if (on_recv_A->is_noobj()) {
              router()->route_unsubscribe(id_p(FOS_DEFAULT_SOURCE_ID), p_p(pattern_A->uri_value()));
            } else {
              router()->route_subscription(
                share(Subscription{
                  .source = FOS_DEFAULT_SOURCE_ID,
                  .pattern = pattern_A->uri_value(),
                  .onRecv = [on_recv_A](const Message_p &message) {
                    // if (!message->source.equals(FOS_DEFAULT_SOURCE_ID))
                    on_recv_A->apply(message->payload);
                  },
                  .onRecvBCode = on_recv_A
                }));
            }
            return lhs;
          };
        },
        areInitialArgs(pattern) ? IType::ZERO_TO_ONE : IType::ONE_TO_ONE);
    }

  private:
    static InstFunctionSupplier reduce_(const BiFunction<Obj_p, Obj_p, Obj_p> &biFunction) {
      return [biFunction](const InstArgs &) {
        return [biFunction](const Objs_p &lhs) {
          Obj_p current = Obj::to_noobj();
          for (const Obj_p &obj: *lhs->objs_value()) {
            if (current->is_noobj()) {
              current = obj;
            } else {
              current = biFunction(obj, current);
            }
          }
          return current;
        };
      };
    }

  public:
    static Int_p sum() {
      return Obj::to_inst(
        "sum", {},
        reduce_([](const Obj_p &a, const Obj_p &b) { return share(*b + *a); }),
        IType::MANY_TO_ONE, Obj::objs_seed());
    }

    static Int_p prod() {
      return Obj::to_inst(
        "prod", {},
        reduce_([](const Obj_p &a, const Obj_p &b) { return share(*b * *a); }),
        IType::MANY_TO_ONE, Obj::objs_seed());
    }

    static Obj_p foldr(const BCode_p &bcode) {
      return Obj::to_inst(
        "foldr", {bcode},
        reduce_([bcode](const Obj_p &a, const Obj_p &b) { return (bcode->apply(b))->apply(a); }),
        IType::MANY_TO_ONE, Obj::objs_seed());
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
                List<Obj_p> m;
                for (size_t j = 0; j < obj->lst_value()->size(); j++) {
                  const Obj_p x = obj->lst_value()->at(j)->apply(lhs->lst_value()->at(i + j));
                  match = !x->is_noobj() && match;
                  if (!match)
                    break;
                  m.push_back(x);
                }
                if (match) {
                  ret.push_back(Obj::to_lst(share(m)));
                }
              }
            } else if (obj->is_lst() && lhs->is_str()) {
              for (size_t i = 0; i <= (lhs->str_value().length() - obj->lst_value()->size()); i++) {
                bool match = true;
                string m;
                for (size_t j = 0; j < obj->lst_value()->size(); j++) {
                  const Obj_p x = obj->lst_value()->at(j)->apply(
                    Obj::to_str(string() + lhs->str_value().at(i + j)));
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
      return Obj::to_inst("until", {bcode}, [](const InstArgs &args) {
        return [args](const Objs_p &lhs) {
          Objs_p ret = objs();
          List<Obj_p> mini_ret = List<Obj_p>();
          for (const auto &obj: *lhs->objs_value()) {
            Obj_p mini_obj = args.at(0)->apply(obj);
            mini_ret.push_back(obj);
            if (mini_obj->bool_value()) {
              ret->add_obj(lst(List<Obj_p>(mini_ret)));
              mini_ret.clear();
            }
          }
          ret->add_obj(lst(mini_ret));
          return ret;
        };
      }, IType::MANY_TO_MANY, Obj::objs_seed());
    }

    static Obj_p within(const BCode_p &bcode) {
      return Obj::to_inst(
        "within", {bcode},
        [](const InstArgs &args) {
          return [args](const Poly_p &lhs) {
            // LST BY ELEMENTS
            if (lhs->is_lst()) {
              return Obj::to_lst(Options::singleton()
                ->processor<Obj, BCode, Obj>(Obj::to_objs(lhs->lst_value()), args.at(0))
                ->objs_value());
            }
            // REC BY PAIRS
            if (lhs->is_rec()) {
              Objs_p pairs = Obj::to_objs();
              for (const auto &pair: *lhs->rec_value()) {
                pairs->add_obj(Obj::to_lst({pair.first, pair.second}));
              }
              const Objs_p results = Options::singleton()->processor<Obj, BCode, Obj>(pairs, args.at(0));
              Obj::RecMap_p<> rec = share(Obj::RecMap<>());
              for (const auto &result: *results->objs_value()) {
                rec->insert({result->lst_value()->at(0), result->lst_value()->at(1)});
              }
              return Obj::to_rec(rec);
            }
            // STR BY CHARS
            if (lhs->is_str()) {
              List<Str_p> chars = List<Str_p>();
              string xstr = lhs->str_value();
              for (uint8_t i = 0; i < xstr.length(); i++) {
                chars.push_back(str(xstr.substr(i, 1)));
              }
              Objs_p strs = Options::singleton()->processor<Objs, BCode, Objs>(Obj::to_objs(share(chars)),
                                                                               args.at(0));
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
          return [](const Objs_p &lhs) {
            return !lhs->is_objs() ? jnt(0) : Obj::to_int(lhs->objs_value()->size());
          };
        },
        IType::MANY_TO_ONE,
        Obj::objs_seed());
    }

    static Objs_p barrier(const BCode_p &bcode) {
      return Obj::to_inst(
        "barrier", {bcode},
        [](const InstArgs &args) {
          return [args](const Objs_p &lhs) {
            const Obj_p obj = args.at(0)->apply(lhs);
            return obj->is_objs() ? obj : objs({obj});
          };
        },
        IType::MANY_TO_MANY, Obj::objs_seed());
    }

    static Objs_p block(const Obj_p &rhs) {
      return Obj::to_inst("block", {rhs},
                          [](const InstArgs &args) { return [args](const Objs_p &) { return args.at(0); }; },
                          IType::ONE_TO_ONE);
    }

    static Poly_p split(const Poly_p &poly) {
      return Obj::to_inst("split", {poly}, [](const InstArgs &args) {
        return [args](const Poly_p &lhs) { return args.at(0)->apply(lhs); };
      }, IType::ONE_TO_ONE);
    }

    static Obj_p merge() {
      return Obj::to_inst(
        "merge", {},
        [](const InstArgs &) {
          return [](const Poly_p &lhs) {
            Objs_p objs = Obj::to_objs();
            if (lhs->is_lst()) {
              for (const auto &obj: *lhs->lst_value()) {
                objs->objs_value()->push_back(obj);
              }
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
          const Obj_p &start = args.at(0);
          const Obj_p &end = args.at(1);
          return [start, end](const Poly_p &lhs) {
            if (lhs->is_lst()) {
              Obj::LstList_p<Obj_p> sub = share(List<Obj_p>());
              int s = start->apply(lhs)->int_value();
              int e = end->apply(lhs)->int_value();
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

    static Poly_p each(const Poly_p &poly) {
      return Obj::to_inst(
        "each", {poly},
        [](const InstArgs &args) {
          const Obj_p &arg = args.at(0);
          return [arg](const Poly_p &lhs) {
            Lst_p poly = arg->is_lst() ? arg : arg->apply(lhs);
            if (lhs->is_lst()) {
              Lst_p ret = Obj::to_lst();
              for (uint8_t i = 0; i < lhs->lst_value()->size(); i++) {
                if (poly->lst_value()->size() >= i) {
                  ret->lst_add(poly->lst_value()->at(i)->apply(lhs->lst_value()->at(i)));
                } else {
                  ret->lst_add(Obj::to_noobj()->apply(lhs->lst_value()->at(i)));
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
    ///// HELPER METHODS
    static bool isBarrier(const Inst_p &inst) {
      return inst->itype() == IType::MANY_TO_MANY || inst->itype() == IType::MANY_TO_ONE ||
             inst->itype() == IType::MANY_TO_ZERO;
    }

    static bool isInitial(const Inst_p &inst) {
      return inst->itype() == IType::ZERO_TO_ONE || inst->itype() == IType::ZERO_TO_MANY ||
             inst->itype() == IType::ZERO_TO_ZERO;
    }

    static bool isTerminal(const Inst_p &inst) {
      return inst->itype() == IType::ONE_TO_ZERO || inst->itype() == IType::MANY_TO_ZERO ||
             inst->itype() == IType::ZERO_TO_ZERO;
    }

    static bool areInitialArgs(
      const Obj_p &objA, const Obj_p &objB = Obj::to_noobj(),
      const Obj_p &objC = Obj::to_noobj(), const Obj_p &objD = Obj::to_noobj()) {
      bool result = /*objA->isUri() ? objA->uri_value().isAbsolute() :*/ !objA->is_noop_bcode();
      result = result && /*(objB->isUri() ? objB->uri_value().isAbsolute() :*/ !objB->is_noop_bcode();
      result = result && /*(objC->isUri() ? objC->uri_value().isAbsolute() :*/ !objC->is_noop_bcode();
      result = result && /*(objD->isUri() ? objD->uri_value().isAbsolute() :*/ !objD->is_noop_bcode();
      return result;
    }

    static const List<Obj_p> &arg_check(const ID_p &inst, const List<Obj_p> &args, const uint8_t expectedSize) {
      if (args.size() != expectedSize)
        throw fError("Incorrect number of arguments provided to %s: %i != %i\n", inst->toString().c_str(),
                     args.size(),
                     expectedSize);
      return args;
    }

    static Map<string, string> unary_sugars() {
      static Map<string, string> map = {
        {"-<", "split"},
        {">-", "merge"},
        {"~", "match"},
        {"<-", "to"},
        {"->", "to_inv"},
        {"|", "block"},
        {"^", "lift"},
        {"V", "drop"},
        {"*", "from"},
        {"=", "each"}
      };
      return map;
    }

    static Inst_p to_inst(const ID &typeId, const List<Obj_p> &args) {
      LOG(TRACE, "Searching for inst: %s\n", typeId.toString().c_str());
      /// try user defined inst
      const ID_p typeIdResolved = id_p(INST_FURI->resolve(typeId));
      const Obj_p baseInst = router()->read(typeIdResolved);
      if (baseInst->is_noobj())
        throw fError("Unknown instruction: %s\n", typeIdResolved->toString().c_str());
      if (baseInst->is_inst()) {
        return replace_from_inst(args, baseInst);
      } else if (baseInst->is_bcode()) {
        return Obj::to_inst(
          typeId.name(), args,
          [baseInst](const InstArgs &args) {
            const Obj_p new_bcode = replace_from_bcode(args, baseInst);
            return [new_bcode](const Obj_p &lhs) {
              return new_bcode->apply(lhs);
            };
          },
          baseInst->itype(),
          baseInst->is_inst() ? baseInst->inst_seed_supplier() : Obj::noobj_seed(), // TODO
          id_p(typeId));
      } else {
        throw fError("!b%s!! does not resolve to an inst or bytecode: %s\n", typeIdResolved->toString().c_str(),
                     baseInst->toString().c_str());
      }
    }

  private:
    static Inst_p replace_from_inst(const InstArgs &args, const Inst_p &old_inst) {
      InstArgs new_args;
      for (const Obj_p &old_arg: old_inst->inst_args()) {
        if (old_arg->is_bcode()) {
          new_args.push_back(replace_from_bcode(args, old_arg));
        } else if (old_arg->is_inst() &&
                   old_arg->inst_op() == "from" &&
                   old_arg->inst_arg(0)->is_uri() &&
                   old_arg->inst_arg(0)->uri_value().toString()[0] == '_' &&
                   StringHelper::is_integer(old_arg->inst_arg(0)->uri_value().toString().substr(1))) {
          const uint8_t index = stoi(old_arg->inst_arg(0)->uri_value().toString().substr(1));
          if (index < args.size())
            new_args.push_back(args.at(index));
          else if (old_arg->inst_args().size() == 2)
            new_args.push_back(old_arg->inst_args().at(1)); // default argument
          else
            throw fError("%s requires !y%i!! arguments and !y%i!! were provided\n",
                         old_inst->toString().c_str(), old_arg->inst_args().size(), args.size());
        } else {
          new_args.push_back(old_arg);
        }
      }
      return Obj::to_inst(old_inst->inst_op(), new_args, old_inst->inst_f(), old_inst->itype(),
                          old_inst->inst_seed_supplier());
    }

    static BCode_p replace_from_bcode(const InstArgs &args, const BCode_p &old_bcode) {
      BCode_p new_bcode = bcode({});
      LOG(TRACE, "old bcode type: %s\n", old_bcode->toString().c_str());
      for (const Inst_p &old_inst: *old_bcode->bcode_value()) {
        const Inst_p new_inst = replace_from_inst(args, old_inst);
        new_bcode->add_inst(new_inst);
      }
      LOG(TRACE, "new bcode type: %s\n", new_bcode->toString().c_str());
      return new_bcode;
    }
  };

  [[maybe_unused]] static Inst_p x(const uint8_t arg_num, const Obj_p &defaultArg = noobj()) {
    return Insts::from(Obj::to_uri(string("_") + to_string(arg_num)), defaultArg);
  }
} // namespace fhatos

#endif
