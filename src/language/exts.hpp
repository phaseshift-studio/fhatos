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
#ifndef fhatos_exts_hpp
#define fhatos_exts_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <language/obj.hpp>
#include <language/insts.hpp>
#include <language/type.hpp>

namespace fhatos {
  static ID inst_id(const string &opcode) { return INST_FURI->resolve(opcode); }
  const Str_p ARG_ERROR = str("wrong number of arguments");
  static auto MODELS =
      std::make_shared<Map<ID, List<Pair<ID, Obj_p>>>>(Map<ID, List<Pair<ID, Obj_p>>>{
      /*  {"/model/mmadt/", {
          {inst_id("a"), Insts::a(x(0))},
          {inst_id("optional"), Insts::optional(x(0))},
          //{inst_id("??"), Insts::from(vri(inst_id("optional")))},
          {inst_id("inspect"), Insts::inspect()},
          {inst_id("plus"), Insts::plus(x(0))},
          {inst_id("mult"), Insts::mult(x(0))},
          {inst_id("div"), Insts::div(x(0))},
          {inst_id("mod"), Insts::mod(x(0))},
          {inst_id("eq"), Insts::eq(x(0))},
          {inst_id("neq"), Insts::neq(x(0))},
          {inst_id("gte"), Insts::gte(x(0))},
          {inst_id("lte"), Insts::lte(x(0))},
          {inst_id("lt"), Insts::lt(x(0))},
          {inst_id("gt"), Insts::gt(x(0))},
          {inst_id("to"), Insts::to(x(0), x(1, dool(true)))},
          {inst_id("to_inv"), Insts::to_inv(x(0), x(1, dool(true)))},
          //{inst_id("->"), Insts::from(vri(inst_id("to_inv")))},
          {inst_id("via_inv"), Insts::to_inv(x(0), dool(false))},
          //{inst_id("-->"), Insts::from(vri(inst_id("via_inv")))},
          {inst_id("start"), Insts::start(x(0))},
          {inst_id("merge"), Insts::merge(x(0))},
          //{inst_id(">-"), Insts::from(vri(inst_id("merge")))},
          {inst_id("map"), Insts::map(x(0))},
          {inst_id("filter"), Insts::filter(x(0))},
          {inst_id("count"), Insts::count()},
          {inst_id("subset"), Insts::subset(x(0), x(1))},
          {inst_id("sum"), Insts::sum()},
          {inst_id("prod"), Insts::prod()},
          {inst_id("group"), Insts::group(x(0, bcode()), x(1, bcode()), x(2, bcode()))},
          {inst_id("get"), Insts::get(x(0))},
          {inst_id("set"), Insts::set(x(0), x(1))},
          {inst_id("noop"), Insts::noop()},
          {inst_id("as"), Insts::as(x(0))},
          {inst_id("by"), Insts::by(x(0))},
          {inst_id("type"), Insts::type()},
          {inst_id("is"), Insts::is(x(0))},
          {inst_id("from"), Insts::from(x(0, Insts::error(ARG_ERROR)), x(1))},
          //{inst_id("*"), Insts::from(x(0, Insts::error(ARG_ERROR)), x(1))},
          {inst_id("at"), Insts::at(x(0, Insts::error(ARG_ERROR)), x(1))},
          //{inst_id("@"), Insts::at(x(0, Insts::error(ARG_ERROR)), x(1))},
          {inst_id("within"), Insts::within(x(0))},
          {inst_id("print"), Insts::print(x(0, bcode()))},
          // {inst_id("switch"), Insts::bswitch(x(0))},
          {inst_id("explain"), Insts::explain()},
          {inst_id("drop"), Insts::drop(x(0))},
          //{inst_id("V"), Insts::from(vri(inst_id("drop")))},
          {inst_id("lift"), Insts::lift(x(0))},
          //{inst_id("^"), Insts::from(vri(inst_id("lift")))},
          {inst_id("size"), Insts::size()},
          {inst_id("foldr"), Insts::foldr(x(0))},
          {inst_id("barrier"), Insts::barrier(x(0))},
          {inst_id("block"), Insts::block(x(0))},
          //{inst_id("|"), Insts::from(vri(inst_id("block")))},
          {inst_id("cleave"), Insts::cleave(x(0))},
          {inst_id("split"), Insts::split(x(0))},
          //{inst_id("-<"), Insts::from(vri(inst_id("split")))},
          {inst_id("each"), Insts::each(x(0))},
          //{inst_id("="), Insts::from(vri(inst_id("each")))},
          {inst_id("window"), Insts::window(x(0))},
          {inst_id("match"), Insts::match(x(0))},
          //{inst_id("~"), Insts::from(vri(inst_id("match")))},
          {inst_id("end"), Insts::end()},
          {inst_id("until"), Insts::until(x(0))},
          {inst_id("dedup"), Insts::dedup(x(0, bcode()))},
          {inst_id("insert"), Insts::insert(x(0))},
          {inst_id("and"), Insts::x_and(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3))},
          {inst_id("or"), Insts::x_or(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3))},
          {inst_id("rand"), Insts::rand(x(0, vri(BOOL_FURI)))},
          {inst_id("error"), Insts::error(x(0, str("an error occurred")))},
          {inst_id("repeat"), Insts::repeat(x(0), x(1, bcode()), x(2))}
        }},*/
        {
          "/model/sys/", {
            {"/type/rec/thread", Obj::to_rec({
              {vri(":setup"), Obj::to_bcode()},
              {vri(":loop"), Obj::to_bcode()},
              {vri(":stop"), Obj::to_bcode()}})},
            {"/type/rec/fiber", Obj::to_rec({
              {vri(":setup"), Obj::to_bcode()},
              {vri(":loop"), Obj::to_bcode()},
              {vri(":stop"), Obj::to_bcode()}})},
            {"/type/rec/coroutine", Obj::to_rec({
              {vri(":setup"), Obj::to_bcode()},
              {vri(":loop"), Obj::to_bcode()},
              {vri(":stop"), Obj::to_bcode()}})},
            {"/type/rec/heap", Obj::to_rec({
              {vri(":setup"), Obj::to_bcode()},
              {vri(":loop"), Obj::to_bcode()},
              {vri(":stop"), Obj::to_bcode()}})},
            {"/type/rec/computed", Obj::to_rec({
              {vri(":setup"), Obj::to_bcode()},
              {vri(":loop"), Obj::to_bcode()},
              {vri(":stop"), Obj::to_bcode()}})},
            {"/type/rec/mqtt", Obj::to_rec({
              {vri(":setup"), Obj::to_bcode()},
              {vri(":loop"), Obj::to_bcode()},
              {vri(":stop"), Obj::to_bcode()}})},
            {"/type/rec/sub",
              Obj::to_rec({
                {vri(":source"), Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                {vri(":pattern"), Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                {vri(":on_recv"), Obj::to_bcode()}})},
            {"/type/rec/msg",
              Obj::to_rec({
                {vri(":target"), Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                {vri(":payload"), Obj::to_bcode()},
                {vri(":retain"), Obj::to_bcode({Insts::as(vri(BOOL_FURI))})}})}
          }}});

  class Exts {
  public:
    Exts() = delete;

    static void load_extension(const ID &ext_id) {
      const List<Pair<ID, Obj_p>>& pairs = MODELS->at(ext_id);
      Type::singleton()->progress_bar_ = ProgressBar::start(Options::singleton()->printer<Ansi<>>().get(),
                                                             pairs.size());
      for (const auto &[key, value]: pairs) {
        const auto type_id = id_p(key);
        const auto value_clone = value->clone();
        Type::singleton()->save_type(type_id, value_clone);
      }
      Type::singleton()->progress_bar_->
          end(StringHelper::format("!b%s !yobjs!! loaded\n", ext_id.toString().c_str()));
      Type::singleton()->progress_bar_ = nullptr;
    }
  };
} // namespace fhatos

#endif
