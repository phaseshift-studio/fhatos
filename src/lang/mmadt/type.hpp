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
#ifndef mmadt_types_hpp
#define mmadt_types_hpp

#include "../../fhatos.hpp"
#include "../obj.hpp"
#include "../type.hpp"
#include "../../structure/stype/mqtt/native/mqtt.hpp"

#define TOTAL_INSTRUCTIONS 100

namespace mmadt {
  using namespace fhatos;

  class mmADT {
  public:
    static void import_base_types() {
      Typer::singleton()->start_progress_bar(14);
      TYPE_SAVER(OBJ_FURI, Obj::to_type(OBJ_FURI));
      TYPE_SAVER(NOOBJ_FURI, Obj::to_type(NOOBJ_FURI));
      TYPE_SAVER(BOOL_FURI, Obj::to_type(BOOL_FURI));
      TYPE_SAVER(INT_FURI, Obj::to_type(INT_FURI));
      TYPE_SAVER(REAL_FURI, Obj::to_type(REAL_FURI));
      TYPE_SAVER(STR_FURI, Obj::to_type(STR_FURI));
      TYPE_SAVER(URI_FURI, Obj::to_type(URI_FURI));
      TYPE_SAVER(LST_FURI, Obj::to_type(LST_FURI));
      TYPE_SAVER(REC_FURI, Obj::to_type(REC_FURI));
      TYPE_SAVER(OBJS_FURI, Obj::to_type(OBJS_FURI));
      TYPE_SAVER(BCODE_FURI, Obj::to_type(BCODE_FURI));
      TYPE_SAVER(INST_FURI, Obj::to_type(INST_FURI));
      TYPE_SAVER(ERROR_FURI, Obj::to_type(ERROR_FURI));
      //      TYPE_SAVER(id_p(INT_FURI->extend("::one")), jnt(1));
      //      TYPE_SAVER(id_p(INT_FURI->extend("::zero")), jnt(0));
      Typer::singleton()->end_progress_bar(
        StringHelper::format("\n\t\t!^u1^ " FURI_WRAP " !ybase types!! loaded \n",MMADT_SCHEME "/+"));
    }

    static void import_base_inst() {
      Typer::singleton()->start_progress_bar(TOTAL_INSTRUCTIONS);
      InstBuilder::build(MMADT_SCHEME "/start")
          ->domain_range(NOOBJ_FURI, {0, 0}, OBJS_FURI, {0,INT_MAX})
          ->type_args(x(0, "starts"))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return args->arg(0)->is_objs() ? args->arg(0) : Obj::to_objs({args->arg(0)});
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/print")
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->type_args(x(0, "to_print", ___))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            printer()->printf("%s\n", args->arg(0)->toString().c_str());
            return args->arg(0);
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/as")
          ->type_args(x(0, "type"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            // uri or obj (for type obj)
            return lhs->as(args->arg(0)->is_uri()
                             ? id_p(*ROUTER_RESOLVE(args->arg(0)->uri_value()))
                             : args->arg(0)->tid());
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/at")
          ->type_args(x(0, "var"))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const ID_p at_id = id_p(args->arg(0)->uri_value());
            const Obj_p new_lhs = lhs->is_noobj() ? ROUTER_READ(at_id) : lhs;
            const Obj_p ret = new_lhs->is_noobj() ? noobj() : new_lhs->at(at_id);
            return ret;
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/block")
          // TODO: currently a "special" instruction (see inst->apply() for logic)
          ->type_args(x(0, "rhs"))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return args->arg(0);
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/count")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, INT_FURI, {1, 1})
          //->type_args(x(0, "obj", ___))
          ->inst_f([](const Obj_p &lhs, const InstArgs &) {
            return Obj::to_int(lhs->objs_value()->size());
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/drop")
          ->type_args(x(0, "obj", ___))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return args->arg(0)->apply(lhs);
          })
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->save();

      /*InstBuilder::build(MMADT_SCHEME "/lift")
          ->type_args(x(0, "obj", ___))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const InstList_p next_insts = make_shared<List<Inst_p>>();
            for(const Inst_p &next_inst: *args->arg(0)->bcode_value()) {
              auto next_args = Obj::to_inst_args();
              for(const auto &[k,v]: next_inst->inst_args()->rec_value()) {
                next_args->rec_value()->insert({k, v});
              }
              next_insts->push_back(
                Obj::to_inst(next_inst->inst_op(), next_args, next_inst->inst_f(), next_inst->itype(), next_inst->inst_seed_supplier()));
            }
            return Obj::to_bcode(next_insts);
          })
          ->save();*/

      InstBuilder::build(MMADT_SCHEME "/barrier")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, OBJS_FURI, {0,INT_MAX})
          ->type_args(x(0, "barrier_op", ___))
          ->inst_f([](const Objs_p &lhs, const InstArgs &args) {
            return args->arg(0)->apply(lhs);
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/sum")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, OBJ_FURI, {1, 1})\
          ->save();

      InstBuilder::build(MMADT_SCHEME "/bool/" MMADT_INST_SCHEME "/sum")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, BOOL_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &) {
            bool sum = false;
            for(const auto &b: *lhs->objs_value()) {
              sum = sum || b->bool_value();
            }
            return dool(sum);
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/int/" MMADT_INST_SCHEME "/sum")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, INT_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &) {
            FOS_INT_TYPE sum = 0;
            for(const auto &i: *lhs->objs_value()) {
              sum += i->int_value();
            }
            return jnt(sum);
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/str/" MMADT_INST_SCHEME "/sum")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, STR_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &) {
            string sum;
            for(const auto &s: *lhs->objs_value()) {
              sum += s->str_value();
            }
            return str(sum);
          })

          ->save();


      InstBuilder::build(MMADT_SCHEME "/delay")
          ->type_args(x(0, "millis", ___))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            Process::current_process()->delay(args->arg(0)->int_value());
            return lhs;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/each")
          ->type_args(x(0, "poly"))
          ->save();

      InstBuilder::build(MMADT_SCHEME "/lst/" MMADT_INST_SCHEME "/each")
          ->type_args(x(0, "lst", ___))
          ->domain_range(LST_FURI, LST_FURI)
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Lst_p ret = Obj::to_lst();
            for(uint8_t i = 0; i < args->arg(0)->lst_value()->size(); i++) {
              if(i < lhs->lst_value()->size()) {
                ret->lst_add(args->arg(0)->lst_value()->at(i)->apply(lhs->lst_value()->at(i)));
              } else {
                ret->lst_add(args->arg(0)->lst_value()->at(i)->apply(Obj::to_noobj()));
              }
            }
            return ret;
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/end")
          ->domain_range(OBJ_FURI, {0,INT_MAX}, NOOBJ_FURI, {0, 0})
          ->inst_f([](const Obj_p &, const InstArgs &) {
            return noobj();
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/eq")
          ->domain_range(OBJ_FURI, BOOL_FURI)
          ->type_args(x(0, "rhs"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return Obj::to_bool(lhs->equals(*args->arg(0)));
          })->save();

      InstBuilder::build(MMADT_SCHEME "/explain")
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return lhs;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/from")
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->type_args(x(0, "rhs", ___), x(1, "default", Obj::to_noobj()))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Obj_p result = ROUTER_READ(args->arg(0)->uri_p_value<fURI>());
            return result->is_noobj() ? args->arg(1) : result;
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/is") // TODO: figure out how to get the opcode in obj insts
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
          ->type_args(x(0, "rhs"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return args->arg(0)->bool_value() ? lhs : Obj::to_noobj();
          })

          ->save();

      InstBuilder::build(MMADT_SCHEME "/map")
          ->type_args(x(0, "mapping"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return args->arg(0);
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/merge")
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
          ->type_args(x(0, "count", jnt(INT32_MAX)))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return args->arg(0)->int_value() > 0 ? lhs : Obj::to_noobj();
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/lst" MMADT_INST_SCHEME "/merge")
          ->domain_range(LST_FURI, {1, 1}, OBJS_FURI, {0,INT_MAX})
          ->type_args(x(0, "count", jnt(INT32_MAX)))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const int max = args->arg(0)->int_value();
            const Objs_p objs = Obj::to_objs();
            int counter = 0;
            for(const auto &element: *lhs->lst_value()) {
              if(counter >= max)
                break;
              if(!element->is_noobj()) {
                objs->add_obj(element);
                ++counter;
              }
            }
            return objs;
          })

          ->save();

      InstBuilder::build(MMADT_SCHEME "/rec/" MMADT_INST_SCHEME "/merge")
          ->domain_range(REC_FURI, {1, 1}, OBJS_FURI, {0,INT_MAX})
          ->type_args(x(0, "count", jnt(INT32_MAX)))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const int max = args->arg(0)->int_value();
            const Objs_p objs = Obj::to_objs();
            int counter = 0;
            for(const auto &[key, value]: *lhs->rec_value()) {
              if(counter >= max)
                break;
              if(!value->is_noobj()) {
                objs->add_obj(value->apply(key));
                ++counter;
              }
            }
            return objs;
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/goto")
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {1, 1})
          ->type_args(x(0, "inst_id"))
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            const Inst_p inst = ROUTER_READ(args->arg(0)->uri_p_value<fURI>());
            if(!inst->is_inst())
              throw fError("!bgoto!! must resolve to an !binst!!: %s !m=>!! %s",
                           args->arg(0)->toString().c_str(),
                           inst->toString().c_str());
            ROUTER_PUSH_FRAME("^", args);
            const Obj_p result = ROUTER_READ(id_p("v"));
            ROUTER_POP_FRAME();
            return result;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/neq")
          ->domain_range(OBJ_FURI, BOOL_FURI)
          ->type_args(x(0, "rhs"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return Obj::to_bool(!lhs->equals(*args->arg(0)));
          })->save();
      InstBuilder::build(MMADT_SCHEME "/repeat")
          ->type_args(x(0, "code"), x(1, "until", dool(true)), x(2, "emit", dool(false)))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            Objs_p r = BCODE_PROCESSOR(args->arg(0)->bcode_starts({lhs}));
            return r;
          })
          ->save();
      InstBuilder::build(MMADT_SCHEME "/split")
          ->type_args(x(0, "poly"))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return args->arg(0);
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/to")
          ->type_args(x(0, "uri"), x(1, "retain", dool(true)))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            ROUTER_WRITE(furi_p(args->arg(0)->uri_value()), lhs, args->arg(1)->bool_value());
            return lhs;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/to_inv")
          ->type_args(x(0, "value_id"), x(1, "retain", dool(true)))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Obj_p ret = args->arg(0);
            ROUTER_WRITE(furi_p(lhs->uri_value()), ret, args->arg(1)->bool_value());
            return ret;
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/type")
          ->domain_range(OBJ_FURI, {0, 1}, URI_FURI, {1, 1})
          ->type_args(x(0, "obj", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return Obj::to_uri(*args->arg(0)->tid());
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/within")
          ->type_args(x(0, "code"))
          ->save();

      InstBuilder::build(MMADT_SCHEME "/str/" MMADT_INST_SCHEME "/within")
          ->type_args(x(0, "code"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const auto chars = make_shared<List<Str_p>>();
            const string xstr = lhs->str_value();
            for(uint8_t i = 0; i < xstr.length(); i++) {
              chars->push_back(str(xstr.substr(i, 1)));
            }
            const Objs_p strs = BCODE_PROCESSOR(/*Obj::to_objs(chars), */args->arg(0));
            string ret;
            for(const Str_p &s: *strs->objs_value()) {
              ret += s->str_value();
            }
            return Obj::to_str(ret, lhs->tid(), lhs->vid());
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/lst/" MMADT_INST_SCHEME "/within")
          ->type_args(x(0, "code"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const BCode_p starts_bcode = args->arg(0)->bcode_starts(Obj::to_objs(lhs->lst_value()));
            return Obj::to_lst(BCODE_PROCESSOR(starts_bcode)->objs_value(), lhs->tid(), lhs->vid());
          })
          ->save();

      InstBuilder::build(MMADT_SCHEME "/rec/" MMADT_INST_SCHEME "/within")
          ->type_args(x(0, "code"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Objs_p pairs = Obj::to_objs();
            for(const auto &pair: *lhs->rec_value()) {
              pairs->add_obj(Obj::to_lst({pair.first, pair.second}));
            }
            const Objs_p results = Obj::to_lst(
              BCODE_PROCESSOR(args->arg(0)->bcode_starts({pairs}))->objs_value());
            const Obj::RecMap_p<> rec = make_shared<Obj::RecMap<>>();
            for(const auto &result: *results->objs_value()) {
              rec->insert({result->lst_value()->at(0), result->lst_value()->at(1)});
            }
            return Obj::to_rec(rec, lhs->tid(), lhs->vid());
          })
          ->save();

      /////////////////////////// RELATIONAL PREDICATE INSTS ///////////////////////////
      for(const auto &i: {"gt", "gte", "lt", "lte"}) {
        InstBuilder::build(MMADT_ID->extend(i))
            ->domain_range(OBJ_FURI, BOOL_FURI)
            ->type_args(x(0, "rhs"))
            ->save();
        for(const auto &t: {INT_FURI, REAL_FURI, STR_FURI, URI_FURI}) {
          InstBuilder *builder =
              InstBuilder::build(t->resolve(string(MMADT_INST_SCHEME).append("/").append(i)))
              ->domain_range(t, BOOL_FURI)
              ->type_args(x(0, "rhs"));
          if(i == "gt") {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs > *args->arg(0));
            });
          } else if(i == "gte") {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs >= *args->arg(0));
            });
          } else if(i == "lt") {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs < *args->arg(0));
            });
          } else if(i == "lte") {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs <= *args->arg(0));
            });
          }
          builder->save();
        }
      }
      /////////////////////////// INSPECT INST ///////////////////////////
      InstBuilder::build(MMADT_SCHEME "/inspect")
          ->domain_range(OBJ_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->save();
      InstBuilder::build(MMADT_SCHEME "/bool/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(BOOL_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("value", dool(args->arg(0)->bool_value()));
            return rec;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/int/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(INT_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("value/obj", jnt(args->arg(0)->int_value()));
            rec->rec_set("value/encoding", vri(STR(FOS_INT_TYPE)));
            return rec;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/real/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(REAL_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("value", real(args->arg(0)->real_value()));
            rec->rec_set("encoding", vri(STR(FOS_REAL_TYPE)));
            return rec;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/str/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(STR_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("value", str(args->arg(0)->str_value()));
            rec->rec_set("length", jnt(args->arg(0)->str_value().size()));
            rec->rec_set("encoding", vri(string("UTF") + to_string(8 * sizeof(char))));
            return rec;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/uri/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(URI_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            const fURI furi = args->arg(0)->uri_value();
            if(furi.has_scheme())
              rec->rec_set("scheme", vri(furi.scheme()));
            if(furi.has_user())
              rec->rec_set("user", vri(furi.user()));
            if(furi.has_password())
              rec->rec_set("password", vri(furi.password()));
            if(furi.has_host())
              rec->rec_set("host", vri(furi.host()));
            if(furi.has_port())
              rec->rec_set("port", jnt(furi.port()));
            rec->rec_set("relative", dool(furi.is_relative()));
            rec->rec_set("branch", dool(furi.is_branch()));
            rec->rec_set("headless", dool(furi.headless()));
            rec->rec_set("pattern", dool(furi.is_pattern()));
            if(furi.has_path()) {
              const Lst_p path = Obj::to_lst();
              for(int i = 0; i < furi.path_length(); i++) {
                path->lst_add(vri(furi.segment(i)));
              }
              rec->rec_set("path", path);
            }
            if(furi.has_components()) {
              const Lst_p comps = Obj::to_lst();
              for(const auto &comp: furi.components()) {
                comps->lst_add(vri(comp));
              }
              rec->rec_set("components", comps);
            }
            if(furi.has_query()) {
              rec->rec_set("query", str(furi.query()));
            }
            return rec;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/lst/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(LST_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            args->arg(0)->lst_set("size", args->arg(0)->lst_size());
            bool embeddable = true;
         //   for(const auto &element: *args->arg(0)->lst_value()) {
              /* if(i->is_rec() && i->is_indexed_args()) {
                 embeddable = false;
                 break;
               }*/ // TODO: walk data structure in search of non-uri keyed recs (if any)
         //   }
            rec->rec_set("embeddable", dool(embeddable));
            return rec;
          })->save();


      InstBuilder::build(MMADT_SCHEME "/rec/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(REC_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("size", args->arg(0)->rec_size());
            bool embeddable = true;
            for(const auto &[k,v]: *args->arg(0)->rec_value()) {
              if(!k->is_uri()) {
                embeddable = false;
                break;
              }
            }
            rec->rec_set("embeddable", dool(embeddable));
            return rec;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/inst/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(INST_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("op", str(args->arg(0)->inst_op().c_str()));
            rec->rec_set("args", args->arg(0)->inst_args());
            // rec->rec_set("f", str(ITypeDescriptions.to_chars(args->arg(0)->itype())));
            rec->rec_set(FOS_DOMAIN, vri(*args->arg(0)->domain()));
            // TODO: coefficients as lsts
            rec->rec_set(FOS_RANGE, vri(*args->arg(0)->range()));
            //if(args->arg(0)->inst_f()->pure)
            //  rec->rec_set("body", args->arg(0)->inst_f()->obj_f());
            return rec;
          })->save();

      InstBuilder::build(MMADT_SCHEME "/bcode/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(BCODE_FURI, REC_FURI)
          ->type_args(x(0, "inspected", ___))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            const Lst_p l = lst();
            for(const Inst_p &i: *args->arg(0)->bcode_value()) {
              l->lst_add(i);
            }
            rec->rec_set("value", l);
            return rec;
          })->save();

      //////////////////////////////// MODULO ////////////////////////////////////
      InstBuilder::build(MMADT_SCHEME "/mod")
          ->type_args(x(0, "rhs"))
          ->save();
      InstBuilder::build(MMADT_SCHEME "/int/" MMADT_INST_SCHEME "/mod")
          ->type_args(x(0, "rhs"))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return jnt(lhs->int_value() % args->arg(0)->int_value());
          })
          ->save();
      ////////////////////////////// NEGATIVE /////////////////////////////////////
      InstBuilder::build(MMADT_SCHEME "/neg")
          ->type_args(x(0, "self", ___))
          ->save();

      InstBuilder::build(MMADT_SCHEME "/bool/" MMADT_INST_SCHEME "/neg")
          ->type_args(x(0, "self", ___))
          ->domain_range(BOOL_FURI, {1, 1}, BOOL_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return dool(!args->arg(0)->bool_value(), args->arg(0)->vid());
          })
          ->save();
      InstBuilder::build(MMADT_SCHEME "/int/" MMADT_INST_SCHEME "/neg")
          ->type_args(x(0, "self", ___))
          ->domain_range(INT_FURI, {1, 1}, INT_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return jnt(args->arg(0)->int_value() * -1, args->arg(0)->vid());
          })
          ->save();
      InstBuilder::build(MMADT_SCHEME "/real/" MMADT_INST_SCHEME "/neg")
          ->type_args(x(0, "self", ___))
          ->domain_range(REAL_FURI, {1, 1}, REAL_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return real(args->arg(0)->real_value() * -1.0f, args->arg(0)->vid());
          })
          ->save();
      ////////////////////////////// PLUS/MULT ///////////////////////////////////
      for(const auto &op: {"plus", "mult"}) {
        const ID MMADT_INST = MMADT_ID->extend(op);
        InstBuilder::build(MMADT_INST)
            ->type_args(x(0, "rhs"))
            ->save();

        InstBuilder::build(string(MMADT_SCHEME "/int/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(INT_FURI, {1, 1}, INT_FURI, {1, 1})
            ->type_args(x(0, "rhs")) //
            ->inst_f(
              [op](const Obj_p &lhs, const InstArgs &args) {
                if(strcmp(op, "plus") == 0)
                  return jnt(lhs->int_value() + args->arg(0)->int_value(), lhs->tid(), lhs->vid());
                if(strcmp(op, "mult") == 0)
                  return jnt(lhs->int_value() * args->arg(0)->int_value(), lhs->tid(), lhs->vid());
                throw fError("unknown op %s\n", op);
              })
            ->save();

        InstBuilder::build(string(MMADT_SCHEME "/real/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(REAL_FURI, {1, 1}, REAL_FURI, {1, 1})
            ->type_args(x(0, "rhs"))
            ->inst_f(
              [op](const Obj_p &lhs, const InstArgs &args) {
                if(strcmp(op, "plus") == 0)
                  return jnt(lhs->real_value() + args->arg(0)->real_value(), lhs->tid(), lhs->vid());
                if(strcmp(op, "mult") == 0)
                  return jnt(lhs->real_value() * args->arg(0)->real_value(), lhs->tid(), lhs->vid());
                throw fError("unknown op %s\n", op);
              })
            ->save();

        InstBuilder::build(string(MMADT_SCHEME "/str/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(STR_FURI, {1, 1}, STR_FURI, {1, 1})
            ->type_args(x(0, "rhs"))
            ->inst_f(
              [op](const Obj_p &lhs, const InstArgs &args) {
                if(strcmp(op, "plus") == 0)
                  return str(lhs->str_value().append(args->arg(0)->str_value()), lhs->tid()); // , lhs->vid()
                if(strcmp(op, "mult") == 0) {
                  string temp;
                  for(const char c: lhs->str_value()) {
                    temp += c;
                    temp.append(args->arg(0)->str_value());
                  }
                  return str(temp, lhs->tid()); // , lhs->vid()
                }
                throw fError("unknown op %s\n", op);
              })
            ->save();

        InstBuilder::build(string(MMADT_SCHEME "/bool/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(BOOL_FURI, {1, 1}, BOOL_FURI, {1, 1})
            ->type_args(x(0, "rhs"))
            ->inst_f(
              [op](const Obj_p &lhs, const InstArgs &args) {
                if(strcmp(op, "plus") == 0)
                  return dool(lhs->bool_value() || args->arg(0)->bool_value(), lhs->tid(), lhs->vid());
                if(strcmp(op, "mult") == 0)
                  return dool(lhs->bool_value() && args->arg(0)->bool_value(), lhs->tid(), lhs->vid());
                throw fError("unknown op %s\n", op);
              })
            ->save();

        InstBuilder::build(string(MMADT_SCHEME "/uri/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(URI_FURI, {1, 1}, URI_FURI, {1, 1})
            ->type_args(x(0, "rhs"))
            ->inst_f(
              [op](const Obj_p &lhs, const InstArgs &args) {
                if(strcmp(op, "plus") == 0)
                  return vri(lhs->uri_value().extend(args->arg(0)->uri_value()), lhs->tid()); // , lhs->vid()
                if(strcmp(op, "mult") == 0)
                  return vri(lhs->uri_value().resolve(args->arg(0)->uri_value()), lhs->tid()); // , lhs->vid()
                throw fError("unknown op %s\n", op);
              })
            ->save();

        InstBuilder::build(string(MMADT_SCHEME "/lst/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(LST_FURI, {1, 1}, LST_FURI, {1, 1})
            ->type_args(x(0, "rhs"))
            ->inst_f(
              [op](const Obj_p &lhs, const InstArgs &args) -> Obj_p {
                if(strcmp(op, "plus") == 0) {
                  const auto new_v = make_shared<Obj::LstList>();
                  for(const auto &v: *lhs->lst_value()) {
                    new_v->push_back(v);
                  }
                  for(const auto &v: *args->arg(0)->lst_value()) {
                    new_v->push_back(v);
                  }
                  return Obj::to_lst(new_v, lhs->tid(), lhs->vid());
                }
                if(strcmp(op, "mult") == 0) {
                  const Obj::LstList_p lhs_v = lhs->lst_value();
                  const Obj::LstList_p rhs_v = args->arg(0)->lst_value();
                  const auto new_v = make_shared<Obj::LstList>();
                  for(int i = 0; i < lhs_v->size(); i++) {
                    for(int j = 0; j < rhs_v->size(); j++) {
                      new_v->push_back(
                        TYPE_INST_RESOLVER(
                          lhs_v->at(i),
                          Obj::to_inst({x(0, ___)}, id_p("mult")))
                        ->apply(rhs_v->at(j)));
                    }
                  }
                  return Obj::to_lst(new_v, lhs->tid(), lhs->vid());
                }
                throw fError("unknown op %s\n", op);
              })
            ->save();

        InstBuilder::build(string(MMADT_SCHEME "/rec/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(REC_FURI, {1, 1}, REC_FURI, {1, 1})
            ->type_args(x(0, "rhs"))
            ->inst_f(
              [op](const Obj_p &lhs, const InstArgs &args) -> Obj_p {
                if(strcmp(op, "plus") == 0) {
                  const auto new_v = make_shared<Obj::RecMap<>>();
                  for(const auto &[k1,v1]: *lhs->rec_value()) {
                    new_v->insert_or_assign(k1, v1);
                  }
                  for(const auto &[k2,v2]: *args->arg(0)->rec_value()) {
                    new_v->insert_or_assign(k2, v2);
                  }
                  return Obj::to_rec(new_v, lhs->tid(), lhs->vid());
                }
                if(strcmp(op, "mult") == 0) {
                  const Obj::RecMap_p<> lhs_v = lhs->rec_value();
                  const Obj::RecMap_p<> rhs_v = args->arg(0)->rec_value();
                  const auto new_v = make_shared<Obj::RecMap<>>();
                  for(const auto &[k1,v1]: *lhs_v) {
                    for(const auto &[k2,v2]: *rhs_v) {
                      new_v->insert_or_assign(
                        TYPE_INST_RESOLVER(k1, Obj::to_inst({x(0, ___)}, id_p("mult")))->apply(k2),
                        TYPE_INST_RESOLVER(v1, Obj::to_inst({x(0, ___)}, id_p("mult")))->apply(v2));
                    }
                  }
                  return Obj::to_rec(new_v, lhs->tid(), lhs->vid());
                }
                throw fError("unknown op %s\n", op);
              })
            ->save();
      }
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      Typer::singleton()->end_progress_bar(
        StringHelper::format("\n\t\t!^u1^ " FURI_WRAP " !yobj insts!! loaded \n",
                             MMADT_SCHEME "/+/" C_INST_C MMADT_SCHEME "/+"));
    }

    static void *import() {
      const Str_p ARG_ERROR = str("wrong number of arguments");
      ///////////////////////////////////////// OBJ TYPES ///////////////////////////////////////////////////
      mmADT::import_base_types();
      mmADT::import_base_inst();
      return nullptr;
      ///////////////////////////////////////////////////////////////////////////////////////////////////////

      /*TYPE_SAVER(id_p(MMADT_FURI "rec/inst/plus"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "plus")
                 ->type_args(x(0, "rhs"))
                 ->inst_f(
                     [](const Obj_p &lhs, const InstArgs &args) {
                       return lhs->rec_merge(args->arg(0)->rec_value());
                     })
                 ->save();*/
      /////////////////////////// PLUS INST ///////////////////////////

      // this->saveType(id_p(fURI(FOS_TYPE_PREFIX).extend("uri/url")), bcode());
      /*Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "a"), Insts::a(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "optional"), Insts::optional(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "inspect"), Insts::inspect());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "plus"), Insts::plus(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "mult"), Insts::mult(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "div"), Insts::div(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "mod"), Insts::mod(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "eq"), Insts::eq(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "neq"), Insts::neq(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "gte"), Insts::gte(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "lte"), Insts::lte(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "lt"), Insts::lt(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "gt"), Insts::gt(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "to"), Insts::to(x(0), x(1, dool(true))));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "to_inv"), Insts::to_inv(x(0), x(1, dool(true))));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "via_inv"), Insts::to_inv(x(0), dool(false)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "start"), Insts::start(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "merge"), Insts::merge(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "map"), Insts::map(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "filter"), Insts::filter(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "count"), Insts::count());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "subset"), Insts::subset(x(0), x(1)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "sum"), Insts::sum());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "prod"), Insts::prod());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "group"),
                                   Insts::group(x(0, bcode()), x(1, bcode()), x(2, bcode())));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "get"), Insts::get(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "set"), Insts::set(x(0), x(1)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "noop"), Insts::noop());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "as"), Insts::as(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "by"), Insts::by(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "type"), Insts::type());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "is"), Insts::is(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "from"), Insts::from(x(0, Insts::error(ARG_ERROR)), x(1)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "at"), Insts::at(x(0, Insts::error(ARG_ERROR)), x(1)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "within"), Insts::within(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "print"), Insts::print(x(0, bcode())));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "explain"), Insts::explain());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "drop"), Insts::drop(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "lift"), Insts::lift(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "size"), Insts::size());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "foldr"), Insts::foldr(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "barrier"), Insts::barrier(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "block"), Insts::block(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "cleave"), Insts::cleave(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "split"), Insts::split(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "each"), Insts::each(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "window"), Insts::window(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "match"), Insts::match(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "end"), Insts::end());
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "until"), Insts::until(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "dedup"), Insts::dedup(x(0, bcode())));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "insert"), Insts::insert(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "delay"), Insts::delay(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "from_get"), Insts::from_get(x(0)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "and"),
                                   Insts::x_and(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "or"),
                                   Insts::x_or(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "rand"), Insts::rand(x(0, vri(BOOL_FURI))));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "error"), Insts::error(x(0, str("an error occurred"))));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "repeat"), Insts::repeat(x(0), x(1, bcode()), x(2)));
      Type::singleton()->save_type(id_p(FOS_TYPE_INST_URI "side"), Insts::side(x(0)));*/
      //  return ID(MMADT_FURI);
    }

    static Rec_p build_inspect_rec(const Obj_p &lhs) {
      const Obj_p type = ROUTER_READ(lhs->tid());
      const Rec_p rec = Obj::to_rec();
      rec->rec_set("type/type_id", vri(lhs->tid()));
      rec->rec_set("type/obj", Obj::to_type(lhs->tid()));
      rec->rec_set("type/dom/id", vri(lhs->domain()));
      rec->rec_set("type/dom/coeff", lst({
                     jnt(lhs->domain_coefficient().first),
                     jnt(lhs->domain_coefficient().second)}));
      rec->rec_set("type/rng/id", vri(lhs->range()));
      rec->rec_set("type/rng/coeff", lst({
                     jnt(lhs->range_coefficient().first),
                     jnt(lhs->range_coefficient().second)}));
      if(lhs->vid()) {
        rec->rec_set("value/value_id", vri(lhs->vid()));
        if(const Obj_p subs = ROUTER_READ(id_p(lhs->vid()->query("sub"))); !subs->is_noobj())
          rec->rec_set("subscription", subs);
      }
      return rec;
    }
  };
} // namespace mmadt
#endif
