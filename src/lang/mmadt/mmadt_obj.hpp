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
#ifndef mmadt_obj_hpp
#define mmadt_obj_hpp

#include "../../fhatos.hpp"
#include "../../model/fos/sys/typer/typer.hpp"
#include "../../structure/qtype/q_type.hpp"
#include "../obj.hpp"
#include "mmadt.hpp"

#define MMADT_PREFIX "/mmadt/"
#define MMADT_URI "/mmadt"
#define MMADT_EXT_URI "/mmadt/ext"
#define TOTAL_INSTRUCTIONS 130

namespace mmadt {
  using namespace fhatos;

  static const ID_p CHAR_FURI = id_p(MMADT_EXT_URI "/char");
  static const ID_p HEX_FURI = id_p(MMADT_EXT_URI "/Ox");
  static const ID_p INT8_FURI = id_p(MMADT_EXT_URI "/int8");
  static const ID_p UINT8_FURI = id_p(MMADT_EXT_URI "/uint8");
  static const ID_p INT16_FURI = id_p(MMADT_EXT_URI "/int16");
  static const ID_p INT32_FURI = id_p(MMADT_EXT_URI "/int32");
  static const ID_p NAT_FURI = id_p(MMADT_EXT_URI "/nat");
  static const ID_p CELSIUS_FURI = id_p(MMADT_EXT_URI "/C");
  static const ID_p PERCENT_FURI = id_p(MMADT_EXT_URI "/prnt");
  static const ID_p MILLISECOND_FURI = id_p(MMADT_EXT_URI "/ms");
  static const ID_p SECOND_FURI = id_p(MMADT_EXT_URI "/sec");
  static const ID_p SECRET_FURI = id_p(MMADT_EXT_URI "/secret");

  class mmADT {
  public:

    static Obj_p isa_arg(const ID_p &id) {
      return __().isa(*id);
    }

    static void *import(const std::vector<fURI>& patterns = {});

    static Obj_p delift(const Obj_p &obj) {
      if(obj->is_not_empty_bcode() && obj->bcode_value()->front()->tid->equals(MMADT_PREFIX "lift"))
        return obj->bcode_value()->front()->arg(0);
      if(obj->is_inst() && obj->tid->equals(MMADT_PREFIX "lift"))
        return obj->arg(0);
      return obj;
    }

    static Rec_p build_inspect_rec(const Obj_p &lhs) {
      //const Obj_p type = ROUTER_READ(*lhs->tid);
      const Rec_p rec = Obj::to_rec();
      // rec->rec_set("parent", lhs->parent_ ? lhs->parent_->shared_from_this() : Obj::to_noobj());
      rec->rec_set("type/id", vri(lhs->tid));
      rec->rec_set("type/obj", Obj::to_type(lhs->tid));
      rec->rec_set("type/dom/id", vri(lhs->domain()));
      rec->rec_set("type/dom/coeff", lst({
                       jnt(lhs->domain_coefficient().first),
                       jnt(lhs->domain_coefficient().second)}));
      rec->rec_set("type/rng/id", vri(lhs->range()));
      rec->rec_set("type/rng/coeff", lst({
                       jnt(lhs->range_coefficient().first),
                       jnt(lhs->range_coefficient().second)}));
      if(lhs->vid)
        rec->rec_set("value/id", vri(lhs->vid));
      rec->rec_set("value/obj", Obj::create(lhs->value_, lhs->otype, OTYPE_FURI.at(lhs->otype)));
      if(lhs->vid)
        if(const Obj_p subs = ROUTER_READ(lhs->vid->query("sub")); !subs->is_noobj())
          rec->rec_set("sub", subs);
      return rec;
    }
  };
} // namespace mmadt
#endif
