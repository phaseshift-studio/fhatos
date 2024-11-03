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
#ifndef fhatos_obj_helper_hpp
#define fhatos_obj_helper_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

namespace fhatos {
  class ObjHelper final {
  public:
    ObjHelper() = delete;

    static Inst_p replace_from_inst(const InstArgs &args, const Inst_p &old_inst) {
      if (old_inst->inst_op() == "from" && old_inst->inst_arg(0)->is_uri() &&
          old_inst->inst_arg(0)->uri_value().toString()[0] == '_' &&
          StringHelper::is_integer(old_inst->inst_arg(0)->uri_value().toString().substr(1))) {
        const uint8_t index = stoi(old_inst->inst_arg(0)->uri_value().toString().substr(1));
        if (index < args.size())
          return args.at(index);
        if (old_inst->inst_args().size() == 2)
          return old_inst->inst_args().at(1); // default argument
        throw fError("%s requires !y%i!! arguments and !y%i!! were provided", old_inst->toString().c_str(),
                     old_inst->inst_args().size(), args.size());
      } else {
        InstArgs new_args;
        for (const Obj_p &old_arg: old_inst->inst_args()) {
          new_args.push_back(replace_from_obj(args, old_arg));
        }
        return Obj::to_inst(old_inst->inst_op(), new_args, old_inst->inst_f(), old_inst->itype(),
                            old_inst->inst_seed_supplier());
      }
    }

    static Obj_p replace_from_obj(const InstArgs &args, const Obj_p &old_obj) {
      if (old_obj->is_inst())
        return replace_from_inst(args, old_obj);
      else if (old_obj->is_bcode())
        return replace_from_bcode(args, old_obj);
      else if (old_obj->is_rec())
        return replace_from_rec(args, old_obj);
      else if (old_obj->is_lst())
        return replace_from_lst(args, old_obj);
      else
        return old_obj;
    }

    static BCode_p replace_from_bcode(const InstArgs &args, const BCode_p &old_bcode) {
      BCode_p new_bcode = bcode();
      LOG(TRACE, "old bcode: %s\n", old_bcode->toString().c_str());
      for (const Inst_p &old_inst: *old_bcode->bcode_value()) {
        LOG(TRACE, "replacing old bcode inst: %s\n", old_inst->toString().c_str());
        const Inst_p new_inst = replace_from_inst(args, old_inst);
        new_bcode->add_inst(new_inst);
      }
      LOG(TRACE, "new bcode: %s\n", new_bcode->toString().c_str());
      return new_bcode;
    }

    static Rec_p replace_from_rec(const InstArgs &args, const Rec_p &old_rec) {
      Rec_p new_rec = rec();
      for (const auto &[key, value]: *old_rec->rec_value()) {
        new_rec->rec_set(replace_from_obj(args, key), replace_from_obj(args, value));
      }
      return new_rec;
    }

    static Lst_p replace_from_lst(const InstArgs &args, const Lst_p &old_lst) {
      Lst_p new_lst = lst();
      for (const auto &element: *old_lst->lst_value()) {
        new_lst->lst_add(replace_from_obj(args, element));
      }
      return new_lst;
    }

    ////////////////////////////////////////////////

    static string objAnalysis(const Obj &obj) {
      char a[250];
      sprintf(a,
              "!b%s!! structure:\n"
              "\t!gtype!!            : %s\n"
              "\t!grange<=domain!! : %s<=%s\n"
              "\t!gsize!!  (bytes) : %i\n"
              "\t!gbcode!!         : %s\n"
              "\t!gvalue!!         : %s",
              obj.type()->name().c_str(), obj.type()->toString().c_str(), OTypes.to_chars(obj.o_type()).c_str(),
              OTypes.to_chars(obj.o_type()).c_str(), obj.serialize()->first, FOS_BOOL_STR(obj.is_bcode()),
              obj.is_bcode() ? obj.toString().c_str() : obj.toString(false).c_str());
      return string(a);
    }

    static Rec_p encode_lst(const fURI &base_furi, const List<Obj_p> &list) {
      const Rec_p rec = Obj::to_rec();
      for (size_t i = 0; i < list.size(); i++) {
        rec->rec_set(vri(base_furi.resolve(string("./") + to_string(i))), list.at(i));
      }
      return rec;
    }
  };
} // namespace fhatos
#endif
