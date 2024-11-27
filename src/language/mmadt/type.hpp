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
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <language/type.hpp>
#include FOS_MQTT(mqtt.hpp)

#define TOTAL_INSTRUCTIONS 75
#define MMADT_FURI "/mmadt/"

namespace mmadt {
  using namespace fhatos;

  class mmADT {
  public:
    static Inst_p map(const Obj_p arg) {
      return
          ObjHelper::InstTypeBuilder::build(MMADT_FURI "map")
          ->type_args(arg)
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return args.at(0);
          })
          ->itype_and_seed(IType::ZERO_TO_ONE)
          ->create();
    }

    static ID import() {
      const Str_p ARG_ERROR = str("wrong number of arguments");
      ///////////////////////////////////////// OBJ TYPES ///////////////////////////////////////////////////
      Type::singleton()->start_progress_bar(10);
      TYPE_SAVER(OBJ_FURI, Obj::create(Any(), OType::OBJ, OBJ_FURI));
      TYPE_SAVER(NOOBJ_FURI, Obj::create(Any(), OType::NOOBJ, NOOBJ_FURI));
      TYPE_SAVER(BOOL_FURI, Obj::create(false, OType::BOOL, OBJ_FURI));
      TYPE_SAVER(INT_FURI, Obj::create(0, OType::INT, OBJ_FURI));
      TYPE_SAVER(REAL_FURI, Obj::create(0.0f, OType::REAL, OBJ_FURI));
      TYPE_SAVER(STR_FURI, Obj::create(string("0"), OType::STR, OBJ_FURI));
      TYPE_SAVER(URI_FURI, Obj::create(fURI("mmadt://0"), OType::URI, OBJ_FURI));
      TYPE_SAVER(LST_FURI, Obj::create(make_shared<Obj::LstList>(), OType::LST, OBJ_FURI));
      TYPE_SAVER(REC_FURI, Obj::create(make_shared<Obj::RecMap<>>(), OType::REC, OBJ_FURI));
      //TYPE_SAVER(ERROR_FURI, Obj::create(Pair<Obj_p, Inst_p>(nullptr, nullptr), OType::REC, OBJ_FURI));
      Type::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1 " FURI_WRAP " !ybase types!! loaded \n",MMADT_FURI "+"));
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      Type::singleton()->start_progress_bar(6);
      /* TYPE_SAVER(id_p(MMADT_FURI "inst/start"),
                  ObjHelper::InstTypeBuilder::build(MMADT_FURI "start")
                  ->type_args(x(0, "starts"))
                  ->itype_and_seed(IType::ZERO_TO_MANY, Obj::to_inst([](const Obj_p &obj, const InstArgs &args) {
                    Objs_p objs = Obj::to_objs(make_shared<List<Obj_p>>(args));
                    return objs;
                  }, {x(0)}, id_p(ID(StringHelper::cxx_f_metadata(__FILE__,__LINE__))), nullptr))
                  ->create());*/
      TYPE_SAVER(id_p(MMADT_FURI "inst/as"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "as")
                 ->type_args(x(0, "type"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return lhs->as(id_p(args.at(0)->uri_value()));;
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "inst/map"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "map")
                 ->type_args(x(0, "mapping"))
                 ->inst_f([](const Obj_p &, const InstArgs &args) {
                   return args.at(0);
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "inst/is"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "is")
                 ->type_args(x(0, "rhs"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return args.at(0)->bool_value() ? lhs : _noobj_;
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "inst/from"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "from")
                 ->type_args(x(0, "rhs"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Obj_p result = ROUTER_READ(furi_p(args.at(0)->uri_value()))->at(nullptr);
                   return result->is_noobj() ? args.at(1) : result;
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "inst/block"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "block")
                 ->type_args(x(0, "rhs"))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   return args.at(0);
                 })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "inst/to_inv"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "to_inv")
                 ->type_args(x(0, "value_id"), x(1, "retain", dool(true)))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Obj_p ret = args.at(0);
                   ROUTER_WRITE(furi_p(lhs->uri_value()), ret, args.at(1)->bool_value());
                   return ret;
                 })
                 ->create());
      /////////////////////////// INSPECT INST ///////////////////////////
      TYPE_SAVER(id_p(MMADT_FURI "inst/inspect"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "inspect")
                 ->type_args(x(0, "inspected", ___))
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "bool/inst/inspect"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "inspect")
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(lhs);
                   rec->rec_set("value", dool(lhs->bool_value()));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_FURI "int/inst/inspect"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "inspect")
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(lhs);
                   rec->rec_set("value", jnt(lhs->int_value()));
                   rec->rec_set("encoding", vri(STR(FOS_INT_TYPE)));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_FURI "real/inst/inspect"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "inspect")
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(lhs);
                   rec->rec_set("value", real(lhs->real_value()));
                   rec->rec_set("encoding", vri(STR(FOS_REAL_TYPE)));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_FURI "str/inst/inspect"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "inspect")
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(lhs);
                   rec->rec_set("value", str(lhs->str_value()));
                   rec->rec_set("encoding", vri(string("UTF") + to_string(8 * sizeof(char))));
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_FURI "uri/inst/inspect"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "inspect")
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(lhs);
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
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_FURI "inst/inst/inspect"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "inspect")
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(lhs);
                   rec->rec_set("op", str(lhs->inst_op()));
                   const Lst_p &args_list = lst();
                   for (const Obj_p &o: lhs->inst_args()) {
                     args_list->lst_add(o);
                   }
                   rec->rec_set("args", args_list);
                   //rec->rec_set("f",lhs->inst_f());
                   return rec;
                 })->create());
      TYPE_SAVER(id_p(MMADT_FURI "bcode/inst/inspect"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "inspect")
                 ->type_args(x(0, "inspected", ___))
                 ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
                   const Rec_p rec = build_inspect_rec(lhs);
                   const Lst_p l = lst();
                   for (const Inst_p &i: *lhs->bcode_value()) {
                     l->lst_add(i);
                   }
                   rec->rec_set("value", l);
                   return rec;
                 })->create());
      Type::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1 " FURI_WRAP " !yobj insts!! loaded \n",MMADT_FURI "inst/+"));
      /////////////////////////// PLUS INST ///////////////////////////
      TYPE_SAVER(id_p(MMADT_FURI "inst/plus"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "plus")
                 ->type_args(x(0, "rhs"))
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "int/inst/plus"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "plus")
                 ->type_args(x(0, "rhs"))
                 ->inst_f(
                     [](const Obj_p &lhs, const InstArgs &args) {
                       return jnt(lhs->int_value() + args.at(0)->int_value());
                     })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "real/inst/plus"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "plus")
                 ->type_args(x(0, "rhs"))
                 ->inst_f(
                     [](const Obj_p &lhs, const InstArgs &args) {
                       return real(lhs->real_value() + args.at(0)->real_value());
                     })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "str/inst/plus"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "plus")
                 ->type_args(x(0, "rhs"))
                 ->inst_f(
                     [](const Obj_p &lhs, const InstArgs &args) {
                       return str(lhs->str_value().append(args.at(0)->str_value()));
                     })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "bool/inst/plus"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "plus")
                 ->type_args(x(0, "rhs"))
                 ->inst_f(
                     [](const Obj_p &lhs, const InstArgs &args) {
                       return dool(lhs->bool_value() || args.at(0)->bool_value());
                     })
                 ->create());
      TYPE_SAVER(id_p(MMADT_FURI "uri/inst/plus"),
                 ObjHelper::InstTypeBuilder::build(MMADT_FURI "plus")
                 ->type_args(x(0, "rhs"))
                 ->inst_f(
                     [](const Obj_p &lhs, const InstArgs &args) {
                       return vri(lhs->uri_value().extend(args.at(0)->uri_value()));
                     })
                 ->create());
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
      return ID(MMADT_FURI);
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
      return rec;
    }
  };
} // namespace mmadt
#endif