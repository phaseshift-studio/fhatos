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
  using std::make_pair;
  using LHSArgs = Pair<Obj_p, List_p<Obj_p>>;
  using LHSArgs_p = ptr<LHSArgs>;

  class ObjHelper final {
  public:
    ObjHelper() = delete;

    static Lst_p make_lhs_args(const Obj_p &lhs, const List<Obj_p> &args) {
      return lst({lhs, lst(make_shared<List<Obj_p>>(args))});
    }

    static LHSArgs_p parse_lhs_args(const Obj_p &maybe_lhs_pairs) {
      if (maybe_lhs_pairs->is_noobj())
        return make_shared<LHSArgs>(make_pair<Obj_p, List_p<Obj_p>>(noobj(), make_shared<List<Obj_p>>()));
      if (!maybe_lhs_pairs->is_lst())
        return make_shared<LHSArgs>(Pair<Obj_p, List_p<Obj_p>>(maybe_lhs_pairs, make_shared<List<Obj_p>>()));
      return make_shared<LHSArgs>(
          make_pair<Obj_p, List_p<Obj_p>>(maybe_lhs_pairs->lst_get(0), maybe_lhs_pairs->lst_get(1)->lst_value()));
    }

    static Obj_p apply_lhs_args(const Obj_p &old_obj, const Lst_p &lhs_args, const Obj_p &lhs = noobj()) {
      if (old_obj->is_noobj())
        throw fError("id doesn't not reference !ybcode!! or !yinst!!: !b%s!!", old_obj->toString().c_str());
      LOG(DEBUG, "apply_lhs_args: %s => %s\n", old_obj->toString().c_str(), lhs_args->toString().c_str());
      if (!lhs_args->is_lst())
        return old_obj->apply(lhs_args);
      if (lhs_args->is_lst() && lhs_args->lst_size()->int_value() == 1)
        return old_obj->apply(lhs_args->lst_get(0));
      if (lhs_args->is_lst() && lhs_args->lst_size()->int_value() > 1) {
        const Obj_p new_obj = ObjHelper::replace_from_obj(old_obj, *lhs_args->lst_get(1)->lst_value(), lhs);
        LOG(DEBUG, "structure read() transformed bcode: %s => %s\n", old_obj->toString().c_str(),
            new_obj->toString().c_str());
        return new_obj->apply(lhs_args->lst_get(0));
      }
      return old_obj;
    }

    static Inst_p replace_from_inst(const Obj_p &old_inst, const InstArgs &args, const Obj_p &lhs = noobj()) {
      const bool is_from = old_inst->inst_op() == "from";
      if (is_from && old_inst->inst_arg(0)->is_uri() && old_inst->inst_arg(0)->uri_value().toString()[0] == '_') {
        const uint8_t index = stoi(old_inst->inst_arg(0)->uri_value().name().substr(1));
        if (index < args.size())
          return args.at(index);
        if (old_inst->inst_args().size() == 2)
          return old_inst->inst_args().at(1); // default argument
        throw fError("%s requires !y%i!! arguments and !y%i!! were provided", old_inst->toString().c_str(),
                     old_inst->inst_args().size(), args.size());

      } else if (is_from && old_inst->inst_arg(0)->toString() == "_") {
        return lhs;
      } else {
        InstArgs new_args;
        for (const Obj_p &old_arg: old_inst->inst_args()) {
          new_args.push_back(replace_from_obj(old_arg, args, lhs));
        }
        return Obj::to_inst(old_inst->inst_op(), new_args, old_inst->inst_f(), old_inst->itype(),
                            old_inst->inst_seed_supplier());
      }
    }

    static Obj_p replace_from_obj(const Obj_p &old_obj, const InstArgs &args, const Obj_p &lhs = noobj()) {
      if (old_obj->is_inst())
        return replace_from_inst(old_obj, args, lhs);
      else if (old_obj->is_bcode())
        return replace_from_bcode(old_obj, args, lhs);
      else if (old_obj->is_rec())
        return replace_from_rec(old_obj, args, lhs);
      else if (old_obj->is_lst())
        return replace_from_lst(old_obj, args, lhs);
      else
        return old_obj;
    }

    /*static Function<InstArgs, BCode_p> proto_bcode(const Obj_p &old_bcode) {
      return [old_bcode](const InstArgs &args) {
        return ObjHelper::replace_from_bcode(old_bcode, args);
      };
    }*/

    static BCode_p replace_from_bcode(const Obj_p &old_bcode, const InstArgs &args, const Obj_p &lhs = noobj()) {
      BCode_p new_bcode = bcode();
      LOG(TRACE, "old bcode: %s\n", old_bcode->toString().c_str());
      for (const Inst_p &old_inst: *old_bcode->bcode_value()) {
        LOG(TRACE, "replacing old bcode inst: %s\n", old_inst->toString().c_str());
        const Inst_p new_inst = replace_from_inst(old_inst, args, lhs);
        new_bcode = new_bcode->add_inst(new_inst);
      }
      LOG(TRACE, "new bcode: %s\n", new_bcode->toString().c_str());
      return new_bcode;
    }

    static Rec_p replace_from_rec(const Obj_p &old_rec, const InstArgs &args, const Obj_p &lhs = noobj()) {
      Rec_p new_rec = rec();
      for (const auto &[key, value]: *old_rec->rec_value()) {
        new_rec->rec_set(replace_from_obj(key, args, lhs), replace_from_obj(value, args, lhs));
      }
      return new_rec;
    }

    static Lst_p replace_from_lst(const Obj_p &old_lst, const InstArgs &args, const Obj_p &lhs = noobj()) {
      Lst_p new_lst = lst();
      for (const auto &element: *old_lst->lst_value()) {
        new_lst->lst_add(replace_from_obj(element, args, lhs));
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
              obj.tid()->name().c_str(), obj.tid()->toString().c_str(), OTypes.to_chars(obj.o_type()).c_str(),
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

    class InstTypeBuilder {
      explicit InstTypeBuilder(const TypeO_p &type) : type_(type) {}

    protected:
      TypeO_p type_;
      InstArgs args_{};
      InstFunctionSupplier function_supplier_{};
      IType itype_{IType::ONE_TO_ONE};

    public:
      static InstTypeBuilder *build(const TypeO &type) { return new InstTypeBuilder(id_p(type)); }

      InstTypeBuilder *type_args(const Obj_p &arg0, const Obj_p &arg1 = nullptr, const Obj_p &arg2 = nullptr,
                                 const Obj_p &arg3 = nullptr) {
        this->args_.push_back(arg0);
        if (arg1)
          this->args_.push_back(arg1);
        if (arg2)
          this->args_.push_back(arg2);
        if (arg3)
          this->args_.push_back(arg3);
        return this;
      }

      InstTypeBuilder *instance_f(const BiFunction<Obj_p, InstArgs, Obj_p> &inst_f) {
        this->function_supplier_ = [inst_f](const InstArgs &args) {
          return [args, inst_f](const Obj_p &lhs) {
            InstArgs args_applied;
            for (const Obj_p &arg: args) {
              args_applied.push_back(arg->apply(lhs));
            }
            return inst_f(lhs, args_applied);
          };
        };
        return this;
      }

      Inst_p create(const ValueO_p &value_id = nullptr) const {
        const Inst_p p = Obj::to_inst(this->type_->name(), // opcode
                                      this->args_, // args
                                      this->function_supplier_, itype_, Obj::noobj_seed(), this->type_);

        delete this;
        return value_id ? p->at(value_id) : p;
      }
    };
  };
} // namespace fhatos
#endif