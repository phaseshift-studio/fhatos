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
#include <process/obj_process.hpp>
#include <structure/router.hpp>
#include FOS_MQTT(mqtt.hpp)

#define TOTAL_INSTRUCTIONS 75

using namespace fhatos;

namespace mmadt {
  class mmADT {
    static ID_p inst_id(const string &opcode) { return id_p(INST_FURI->resolve(opcode)); }

  public:
    static void load() {
      const Str_p ARG_ERROR = str("wrong number of arguments");
      // this->saveType(id_p(fURI(FOS_TYPE_PREFIX).extend("uri/url")), bcode());
      Type::singleton()->save_type(MESSAGE_FURI,
                                   Obj::to_rec({
                                       {vri(":target"), Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                                       {vri(":payload"), Obj::to_bcode()},
                                       {vri(":retain"), Obj::to_bcode({Insts::as(vri(BOOL_FURI))})}
                                   }));
      Type::singleton()->save_type(SUBSCRIPTION_FURI, Obj::to_rec({
                                       {vri(":source"), Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                                       {vri(":pattern"), Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                                       {vri(":on_recv"), Obj::to_bcode()}}));
      Type::singleton()->save_type(THREAD_FURI, Obj::to_rec({
                                       {vri(":setup"), Obj::to_bcode()},
                                       {vri(":loop"), Obj::to_bcode()},
                                       {vri(":stop"), Obj::to_bcode()},
                                       {vri(":delay"), Obj::to_bcode()},
                                       {vri(":yield"), Obj::to_bcode()}}));
      Type::singleton()->start_progress_bar(TOTAL_INSTRUCTIONS);
      Type::singleton()->save_type(inst_id("a"), Insts::a(x(0)));
      Type::singleton()->save_type(inst_id("optional"), Insts::optional(x(0)), false);
      // this->save_type(inst_id("*"), Insts::from(vri(inst_id("from"))));
      // this->save_type(inst_id("V"), Insts::from(vri(inst_id("drop"))));
      // this->save_type(inst_id("^"), Insts::from(vri(inst_id("lift"))));
      // this->save_type(inst_id("|"), Insts::from(vri(inst_id("block"))));
      // this->save_type(inst_id("??"), Insts::from(vri(inst_id("optional"))));
      Type::singleton()->save_type(inst_id("inspect"), Insts::inspect());
      Type::singleton()->save_type(inst_id("plus"), Insts::plus(x(0)));
      Type::singleton()->save_type(inst_id("mult"), Insts::mult(x(0)));
      Type::singleton()->save_type(inst_id("div"), Insts::div(x(0)));
      Type::singleton()->save_type(inst_id("mod"), Insts::mod(x(0)));
      Type::singleton()->save_type(inst_id("eq"), Insts::eq(x(0)));
      Type::singleton()->save_type(inst_id("neq"), Insts::neq(x(0)));
      Type::singleton()->save_type(inst_id("gte"), Insts::gte(x(0)));
      Type::singleton()->save_type(inst_id("lte"), Insts::lte(x(0)));
      Type::singleton()->save_type(inst_id("lt"), Insts::lt(x(0)));
      Type::singleton()->save_type(inst_id("gt"), Insts::gt(x(0)));
      Type::singleton()->save_type(inst_id("to"), Insts::to(x(0), x(1, dool(true))));
      Type::singleton()->save_type(inst_id("to_inv"), Insts::to_inv(x(0), x(1, dool(true))));
      // this->save_type(inst_id("->"), Insts::from(vri(inst_id("to_inv"))));
      Type::singleton()->save_type(inst_id("via_inv"), Insts::to_inv(x(0), dool(false)));
      // this->save_type(inst_id("-->"), Insts::from(vri(inst_id("via_inv"))));
      Type::singleton()->save_type(inst_id("start"), Insts::start(x(0)));
      Type::singleton()->save_type(inst_id("merge"), Insts::merge(x(0)));
      // this->save_type(inst_id("-<"), Insts::from(vri(inst_id("split"))));
      // this->save_type(inst_id(">-"), Insts::from(vri(inst_id("merge"))));
      // this->save_type(inst_id("_/"), Insts::from(vri(inst_id("within"))));
      // this->save_type(inst_id("\_"), Insts::from(vri(inst_id("within"))));
      Type::singleton()->save_type(inst_id("map"), Insts::map(x(0)));
      Type::singleton()->save_type(inst_id("filter"), Insts::filter(x(0)));
      Type::singleton()->save_type(inst_id("count"), Insts::count());
      Type::singleton()->save_type(inst_id("subset"), Insts::subset(x(0), x(1)));
      Type::singleton()->save_type(inst_id("sum"), Insts::sum());
      Type::singleton()->save_type(inst_id("prod"), Insts::prod());
      Type::singleton()->save_type(inst_id("group"), Insts::group(x(0, bcode()), x(1, bcode()), x(2, bcode())));
      Type::singleton()->save_type(inst_id("get"), Insts::get(x(0)));
      Type::singleton()->save_type(inst_id("set"), Insts::set(x(0), x(1)));
      Type::singleton()->save_type(inst_id("noop"), Insts::noop());
      Type::singleton()->save_type(inst_id("as"), Insts::as(x(0)));
      Type::singleton()->save_type(inst_id("by"), Insts::by(x(0)));
      Type::singleton()->save_type(inst_id("type"), Insts::type());
      Type::singleton()->save_type(inst_id("is"), Insts::is(x(0)));
      Type::singleton()->save_type(inst_id("from"), Insts::from(x(0, Insts::error(ARG_ERROR)), x(1)));
      Type::singleton()->save_type(inst_id("at"), Insts::at(x(0, Insts::error(ARG_ERROR)), x(1)));
      Type::singleton()->save_type(inst_id("within"), Insts::within(x(0)));
      Type::singleton()->save_type(inst_id("print"), Insts::print(x(0, bcode())));
      Type::singleton()->save_type(inst_id("explain"), Insts::explain());
      Type::singleton()->save_type(inst_id("drop"), Insts::drop(x(0)));
      Type::singleton()->save_type(inst_id("lift"), Insts::lift(x(0)));
      Type::singleton()->save_type(inst_id("size"), Insts::size());
      Type::singleton()->save_type(inst_id("foldr"), Insts::foldr(x(0)));
      Type::singleton()->save_type(inst_id("barrier"), Insts::barrier(x(0)));
      Type::singleton()->save_type(inst_id("block"), Insts::block(x(0)));
      Type::singleton()->save_type(inst_id("cleave"), Insts::cleave(x(0)));
      Type::singleton()->save_type(inst_id("split"), Insts::split(x(0)));
      Type::singleton()->save_type(inst_id("each"), Insts::each(x(0)));
      Type::singleton()->save_type(inst_id("window"), Insts::window(x(0)));
      Type::singleton()->save_type(inst_id("match"), Insts::match(x(0)));
      Type::singleton()->save_type(inst_id("end"), Insts::end());
      Type::singleton()->save_type(inst_id("until"), Insts::until(x(0)));
      Type::singleton()->save_type(inst_id("dedup"), Insts::dedup(x(0, bcode())));
      Type::singleton()->save_type(inst_id("insert"), Insts::insert(x(0)));
      Type::singleton()->save_type(inst_id("delay"), Insts::delay(x(0)));
      Type::singleton()->save_type(inst_id("and"), Insts::x_and(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3)));
      Type::singleton()->save_type(inst_id("or"), Insts::x_or(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3)));
      Type::singleton()->save_type(inst_id("rand"), Insts::rand(x(0, vri(BOOL_FURI))));
      Type::singleton()->save_type(inst_id("error"), Insts::error(x(0, str("an error occurred"))));
      Type::singleton()->save_type(inst_id("repeat"), Insts::repeat(x(0), x(1, bcode()), x(2)));
      Type::singleton()->save_type(inst_id("side"), Insts::side(x(0)));
      Type::singleton()->end_progress_bar("!bmm-adt !yobjs!! loaded\n");
    }
  };
} // namespace mmadt
#endif