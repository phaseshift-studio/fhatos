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

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <language/type.hpp>
#include FOS_MQTT(mqtt.hpp)

#define TOTAL_INSTRUCTIONS 75

namespace mmadt {
  using namespace fhatos;

  class mmADT {
  public:
    static Inst_p map(const Obj_p arg) {
      return
          InstBuilder::build(MMADT_SCHEME "/map")
          ->type_args(arg)
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return args.at(0);
          })
          ->itype_and_seed(IType::ZERO_TO_ONE)
          ->create();
    }

    static void import_base_types() {
      Type::singleton()->start_progress_bar(16);
      const Obj_p OBJ_TYPE = Obj::create(Any(), OType::OBJ, OBJ_FURI);
      TYPE_SAVER(OBJ_FURI, Obj::create(Any(), OType::OBJ, OBJ_FURI));
      TYPE_SAVER(NOOBJ_FURI, Obj::create(Any(), OType::OBJ, NOOBJ_FURI));
      TYPE_SAVER(BOOL_FURI, Obj::create(Any(), OType::OBJ, BOOL_FURI));
      TYPE_SAVER(INT_FURI, Obj::create(Any(), OType::OBJ, INT_FURI));
      TYPE_SAVER(id_p(INT_FURI->extend("::one")), jnt(1));
      TYPE_SAVER(id_p(INT_FURI->extend("::zero")), jnt(0));
      TYPE_SAVER(REAL_FURI, Obj::create(Any(), OType::OBJ, REAL_FURI));
      TYPE_SAVER(STR_FURI, Obj::create(Any(), OType::OBJ, STR_FURI));
      TYPE_SAVER(URI_FURI, Obj::create(Any(), OType::OBJ, URI_FURI));
      TYPE_SAVER(LST_FURI, Obj::create(Any(), OType::OBJ, LST_FURI));
      TYPE_SAVER(REC_FURI, Obj::create(Any(), OType::OBJ, REC_FURI));
      TYPE_SAVER(OBJS_FURI, Obj::create(Any(), OType::OBJ, OBJS_FURI));
      TYPE_SAVER(BCODE_FURI, Obj::create(Any(), OType::OBJ, BCODE_FURI));
      TYPE_SAVER(INST_FURI, Obj::create(Any(), OType::OBJ, INST_FURI));
      TYPE_SAVER(ERROR_FURI, Obj::create(Any(), OType::OBJ, ERROR_FURI));
      Type::singleton()->end_progress_bar(
        StringHelper::format("\n\t\t!^u1 " FURI_WRAP " !ybase types!! loaded \n",MMADT_SCHEME "/+"));
    }

    static void import_base_inst() {
      Type::singleton()->start_progress_bar(TOTAL_INSTRUCTIONS);
      /* TYPE_SAVER(id_p(MMADT_FURI "inst/start"),
                  ObjHelper::InstTypeBuilder::build(MMADT_FURI "start")
                  ->type_args(x(0, "starts"))
                  ->itype_and_seed(IType::ZERO_TO_MANY, Obj::to_inst([](const Obj_p &obj, const InstArgs &args) {
                    Objs_p objs = Obj::to_objs(make_shared<List<Obj_p>>(args));
                    return objs;
                  }, {x(0)}, id_p(ID(StringHelper::cxx_f_metadata(__FILE__,__LINE__))), nullptr))
                  ->create());*/
      TYPE_SAVER(id_p(MMADT_SCHEME "/as"),
                 InstBuilder::build(MMADT_SCHEME "/as")
                 ->type_args(x(0, "type"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return lhs->as(id_p(args.at(0)->uri_value()));;
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/at"),
                 InstBuilder::build(MMADT_SCHEME "/at")
                 ->type_args(x(0, "var"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const ID_p at_id = id_p(args.at(0)->uri_value());
                   Obj_p result = lhs->is_noobj() ? ROUTER_READ(at_id)->at(at_id) : lhs->at(at_id);
                   return result;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/block"),
                 InstBuilder::build(MMADT_SCHEME "/block")
                 // TODO: currently a "special" instruction (see inst->apply() for logic)
                 ->type_args(x(0, "rhs"))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   return args.at(0);
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/count"),
                 InstBuilder::build(MMADT_SCHEME "/count")
                 ->domain_range(OBJS_FURI, INT_FURI)
                 ->type_args(x(0, "obj", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   return !args.at(0)->is_objs() ? jnt(1) : Obj::to_int(args.at(0)->objs_value()->size());
                 })
                 ->itype_and_seed(IType::MANY_TO_ONE,
                                  Obj::to_bcode([](const Obj_p &lhs, const InstArgs &args) { return objs(); },
                                                InstArgs(),
                                                StringHelper::cxx_f_metadata(__FILE__,__LINE__).c_str()))
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/delay"),
                 InstBuilder::build(MMADT_SCHEME "/delay")
                 ->type_args(x(0, "millis", ___))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   Process::current_process()->delay(args.at(0)->int_value());
                   return lhs;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/each"),
                 InstBuilder::build(MMADT_SCHEME "/each")
                 ->type_args(x(0, "poly"))
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/lst/" MMADT_INST_SCHEME "/each"),
                 InstBuilder::build("each")
                 ->type_args(x(0, "lst", ___))
                 ->domain_range(LST_FURI, LST_FURI)
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Lst_p ret = Obj::to_lst();
                   for(uint8_t i = 0; i < args.at(0)->lst_value()->size(); i++) {
                     if(i < lhs->lst_value()->size()) {
                       ret->lst_add(args.at(0)->lst_value()->at(i)->apply(lhs->lst_value()->at(i)));
                     } else {
                       ret->lst_add(args.at(0)->lst_value()->at(i)->apply(Obj::to_noobj()));
                     }
                   }
                   return ret;
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/eq"),
                 InstBuilder::build(MMADT_SCHEME "/eq")
                 ->domain_range(OBJ_FURI, BOOL_FURI)
                 ->type_args(x(0, "rhs"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return Obj::to_bool(lhs->equals(*args.at(0)));
                 })->create());
      static Obj_p FROM_INST;
      TYPE_SAVER(id_p(MMADT_SCHEME "/from"),
                 FROM_INST = InstBuilder::build(MMADT_SCHEME "/from")
                 ->domain_range(URI_FURI, OBJ_FURI)
                 ->type_args(x(0, "rhs", ___), x(1, "default", _noobj_))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Obj_p result = ROUTER_READ(furi_p(args.at(0)->uri_value()));
                   return result->is_noobj() ? args.at(1) : result;
                 })
                 ->itype_and_seed(IType::ZERO_TO_MANY)
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/is"),
                 InstBuilder::build(MMADT_SCHEME "/is") // TODO: figure out how to get the opcode in obj insts
                 //->domain_range(OBJ_FURI, OBJ_FURI)
                 ->type_args(x(0, "rhs"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return args.at(0)->bool_value() ? lhs : _noobj_;
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/map"),
                 InstBuilder::build(MMADT_SCHEME "/map")
                 ->type_args(x(0, "mapping"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return args.at(0);
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/merge"),
                 InstBuilder::build(MMADT_SCHEME "/merge")
                 ->domain_range(LST_FURI, OBJS_FURI)
                 ->type_args(x(0, "count", jnt(INT32_MAX)))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const int max = args.at(0)->int_value();
                   const Objs_p objs = Obj::to_objs();
                   if(lhs->is_lst()) {
                     int counter = 0;
                     for(const auto &element: *lhs->lst_value()) {
                       if(counter >= max)
                         break;
                       if(!element->is_noobj()) {
                         objs->add_obj(element);
                         ++counter;
                       }
                     }
                   } else if(lhs->is_rec()) {
                     int counter = 0;
                     for(const auto &[key, value]: *lhs->rec_value()) {
                       if(counter >= max)
                         break;
                       if(!value->is_noobj()) {
                         objs->add_obj(value);
                         ++counter;
                       }
                     }
                   } else {
                     if(max > 0)
                       objs->add_obj(lhs);
                   }
                   return objs;
                 })
                 ->itype_and_seed(IType::ONE_TO_MANY)
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/neq"),
                 InstBuilder::build(MMADT_SCHEME "/neq")
                 ->domain_range(OBJ_FURI, BOOL_FURI)
                 ->type_args(x(0, "rhs"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return Obj::to_bool(!lhs->equals(*args.at(0)));
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/to"),
                 InstBuilder::build(MMADT_SCHEME "/to")
                 ->type_args(x(0, "uri"), x(1, "retain", dool(true)))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   ROUTER_WRITE(furi_p(args.at(0)->uri_value()), lhs, args.at(1)->bool_value());
                   return lhs;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/to_inv"),
                 InstBuilder::build(MMADT_SCHEME "/to_inv")
                 ->type_args(x(0, "value_id"), x(1, "retain", dool(true)))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Obj_p ret = args.at(0);
                   ROUTER_WRITE(furi_p(lhs->uri_value()), ret, args.at(1)->bool_value());
                   return ret;
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/type"),
                 InstBuilder::build(MMADT_SCHEME "/type")
                 ->domain_range(OBJ_FURI, URI_FURI)
                 ->type_args(x(0, "obj", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   return Obj::to_uri(*args.at(0)->tid());
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/within"),
                 InstBuilder::build(MMADT_SCHEME "/within")
                 ->type_args(x(0, "code"))
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/str/" MMADT_INST_SCHEME "/within"),
                 InstBuilder::build(MMADT_SCHEME "/within")
                 ->type_args(x(0, "code"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   ptr<List<Str_p>> chars = make_shared<List<Str_p>>();
                   const string xstr = lhs->str_value();
                   for(uint8_t i = 0; i < xstr.length(); i++) {
                     chars->push_back(str(xstr.substr(i, 1)));
                   }
                   const Objs_p strs = BCODE_PROCESSOR(/*Obj::to_objs(chars), */args.at(0));
                   string ret;
                   for(const Str_p &s: *strs->objs_value()) {
                     ret += s->str_value();
                   }
                   return str(ret);
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/lst/" MMADT_INST_SCHEME "/within"),
                 InstBuilder::build(MMADT_SCHEME "/within")
                 ->type_args(x(0, "code"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return Obj::to_lst(BCODE_PROCESSOR(/*Obj::to_objs(chars), */args.at(0))
                     ->objs_value());
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/rec/" MMADT_INST_SCHEME "/within"),
                 InstBuilder::build(MMADT_SCHEME "/within")
                 ->type_args(x(0, "code"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Objs_p pairs = Obj::to_objs();
                   for(const auto &pair: *lhs->rec_value()) {
                     pairs->add_obj(Obj::to_lst({pair.first, pair.second}));
                   }
                   const Objs_p results =BCODE_PROCESSOR(/*pairs, */args.at(0));
                   const Obj::RecMap_p<> rec = make_shared<Obj::RecMap<>>();
                   for(const auto &result: *results->objs_value()) {
                     rec->insert({result->lst_value()->at(0), result->lst_value()->at(1)});
                   }
                   return Obj::to_rec(rec);
                 })
                 ->create());
      /////////////////////////// RELATIONAL PREDICATE INSTS ///////////////////////////
      for(const auto &i: {"gt", "gte", "lt", "lte"}) {
        TYPE_SAVER(id_p(MMADT_ID->extend(i)),
                   InstBuilder::build(MMADT_ID->extend(i))
                   ->domain_range(OBJ_FURI, BOOL_FURI)
                   ->type_args(x(0, "rhs"))
                   ->create());
        for(const auto &t: {INT_FURI, REAL_FURI, STR_FURI, URI_FURI}) {
          InstBuilder *builder =
              InstBuilder::build(MMADT_ID->extend(i))
              ->domain_range(t, BOOL_FURI)
              ->type_args(x(0, "rhs"));
          if(i == "gt") {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs > *args.at(0));
            });
          } else if(i == "gte") {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs >= *args.at(0));
            });
          } else if(i == "lt") {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs < *args.at(0));
            });
          } else if(i == "lte") {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs <= *args.at(0));
            });
          }
          TYPE_SAVER(id_p(t->resolve(string(MMADT_INST_SCHEME).append("/").append(i))), builder->create());
        }
      }
      /////////////////////////// INSPECT INST ///////////////////////////
      TYPE_SAVER(id_p(MMADT_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(OBJ_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/bool/" MMADT_INST_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(BOOL_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(args.at(0));
                   rec->rec_set("value", dool(args.at(0)->bool_value()));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/int/" MMADT_INST_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(INT_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(args.at(0));
                   rec->rec_set("value/obj", jnt(args.at(0)->int_value()));
                   rec->rec_set("value/encoding", vri(STR(FOS_INT_TYPE)));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/real/" MMADT_INST_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(REAL_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(args.at(0));
                   rec->rec_set("value", real(args.at(0)->real_value()));
                   rec->rec_set("encoding", vri(STR(FOS_REAL_TYPE)));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/str/" MMADT_INST_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(STR_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(args.at(0));
                   rec->rec_set("value", str(args.at(0)->str_value()));
                   rec->rec_set("length", jnt(args.at(0)->str_value().size()));
                   rec->rec_set("encoding", vri(string("UTF") + to_string(8 * sizeof(char))));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/uri/" MMADT_INST_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(URI_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(args.at(0));
                   const fURI furi = args.at(0)->uri_value();
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
                       path->lst_add(vri(furi.path(i)));
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
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/rec/" MMADT_INST_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(REC_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(args.at(0));
                   rec->rec_set("size", args.at(0)->rec_size());
                   bool embeddable = true;
                   for(const auto &[k,v]: *args.at(0)->rec_value()) {
                     if(!k->is_uri()) {
                       embeddable = false;
                       break;
                     }
                   }
                   rec->rec_set("embeddable", dool(embeddable));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/inst/" MMADT_INST_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(INST_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(args.at(0));
                   rec->rec_set("op", str(args.at(0)->inst_op()));
                   const Lst_p &args_list = lst();
                   for(const Obj_p &o: args.at(0)->inst_args()) {
                     args_list->lst_add(o);
                   }
                   rec->rec_set("args", args_list);
                   //rec->rec_set("f",lhs->inst_f());
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_SCHEME "/bcode/" MMADT_INST_SCHEME "/inspect"),
                 InstBuilder::build(MMADT_SCHEME "/inspect")
                 ->domain_range(BCODE_FURI, REC_FURI)
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(args.at(0));
                   const Lst_p l = lst();
                   for(const Inst_p &i: *args.at(0)->bcode_value()) {
                     l->lst_add(i);
                   }
                   rec->rec_set("value", l);
                   return rec;
                 })->create());
      /////////////////////////// PLUS,MUT INST ///////////////////////////
      for(const auto &op: {"plus", "mult"}) {
        const ID MMADT_INST = MMADT_ID->extend(op);
        TYPE_SAVER(id_p(MMADT_INST),
                   InstBuilder::build(MMADT_INST)
                   ->type_args(x(0, "rhs"))
                   ->create());
        TYPE_SAVER(id_p(string(MMADT_SCHEME "/int/" MMADT_INST_SCHEME "/").append(op).c_str()),
                   InstBuilder::build(MMADT_INST)
                   ->domain_range(INT_FURI)
                   ->type_args(x(0, "rhs")) //
                   ->inst_f(
                     [op](const Obj_p &lhs, const InstArgs &args) {
                       if(strcmp(op, "plus") == 0)
                         return jnt(lhs->int_value() + args.at(0)->int_value(), lhs->tid(), lhs->vid());
                       else if(strcmp(op, "mult") == 0)
                         return jnt(lhs->int_value() * args.at(0)->int_value(), lhs->tid(), lhs->vid());
                       else
                         throw fError("unknown op %s\n", op);
                     })
                   ->create());
        TYPE_SAVER(id_p(string(MMADT_SCHEME "/real/" MMADT_INST_SCHEME "/").append(op).c_str()),
                   InstBuilder::build(MMADT_INST)
                   ->domain_range(REAL_FURI)
                   ->type_args(x(0, "rhs"))
                   ->inst_f(
                     [op](const Obj_p &lhs, const InstArgs &args) {
                       if(strcmp(op, "plus") == 0)
                         return jnt(lhs->real_value() + args.at(0)->int_value(), lhs->tid(), lhs->vid());
                       else if(strcmp(op, "mult") == 0)
                         return jnt(lhs->real_value() * args.at(0)->int_value(), lhs->tid(), lhs->vid());
                       else
                         throw fError("unknown op %s\n", op);
                     })
                   ->create());
        TYPE_SAVER(id_p(string(MMADT_SCHEME "/str/" MMADT_INST_SCHEME "/").append(op).c_str()),
                   InstBuilder::build(MMADT_INST)
                   ->domain_range(STR_FURI)
                   ->type_args(x(0, "rhs"))
                   ->inst_f(
                     [op](const Obj_p &lhs, const InstArgs &args) {
                       if(strcmp(op, "plus") == 0)
                         return str(lhs->str_value().append(args.at(0)->str_value()), lhs->tid()); // , lhs->vid()
                       else if(strcmp(op, "mult") == 0) {
                         string temp;
                         for(const char c: lhs->str_value()) {
                           temp += c;
                           temp.append(args.at(0)->str_value());
                         }
                         return str(temp, lhs->tid()); // , lhs->vid()
                       } else
                         throw fError("unknown op %s\n", op);
                     })
                   ->create());
        TYPE_SAVER(id_p(string(MMADT_SCHEME "/bool/" MMADT_INST_SCHEME "/").append(op).c_str()),
                   InstBuilder::build(MMADT_INST)
                   ->domain_range(BOOL_FURI)
                   ->type_args(x(0, "rhs"))
                   ->inst_f(
                     [op](const Obj_p &lhs, const InstArgs &args) {
                       if(strcmp(op, "plus") == 0)
                         return dool(lhs->bool_value() || args.at(0)->bool_value(), lhs->tid(), lhs->vid());
                       else if(strcmp(op, "mult") == 0)
                         return dool(lhs->bool_value() && args.at(0)->bool_value(), lhs->tid(), lhs->vid());
                       else
                         throw fError("unknown op %s\n", op);
                     })
                   ->create());
        TYPE_SAVER(id_p(string(MMADT_SCHEME "/uri/" MMADT_INST_SCHEME "/").append(op).c_str()),
                   InstBuilder::build(MMADT_INST)
                   ->domain_range(URI_FURI)
                   ->type_args(x(0, "rhs"))
                   ->inst_f(
                     [op](const Obj_p &lhs, const InstArgs &args) {
                       if(strcmp(op, "plus") == 0)
                         return vri(lhs->uri_value().extend(args.at(0)->uri_value()), lhs->tid()); // , lhs->vid()
                       else if(strcmp(op, "mult") == 0)
                         return vri(lhs->uri_value().resolve(args.at(0)->uri_value()), lhs->tid()); // , lhs->vid()
                       else
                         throw fError("unknown op %s\n", op);
                     })
                   ->create());
      }
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      Type::singleton()->end_progress_bar(
        StringHelper::format("\n\t\t!^u1 " FURI_WRAP " !yobj insts!! loaded \n",
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
                       return lhs->rec_merge(args.at(0)->rec_value());
                     })
                 ->create());*/
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
      rec->rec_set("type/domain", vri(lhs->domain()));
      rec->rec_set("type/range", vri(lhs->range()));
      rec->rec_set("type/obj", FURI_OTYPE.count(*lhs->tid())
                                 ? vri(OTypes.to_chars(FURI_OTYPE.at(*lhs->tid())))
                                 : type);
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
