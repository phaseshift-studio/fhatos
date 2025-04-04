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

#include "mmadt_obj.hpp"
#include "../../util/print_helper.hpp"

namespace mmadt {
  using namespace fhatos;

     void mmADT::import_base_types() {
      Typer::singleton()->start_progress_bar(14);
      Typer::singleton()->save_type(*OBJ_FURI, Obj::to_type(OBJ_FURI));
      Typer::singleton()->save_type(*NOOBJ_FURI, Obj::to_type(NOOBJ_FURI));
      Typer::singleton()->save_type(*BOOL_FURI, Obj::to_type(BOOL_FURI));
      Typer::singleton()->save_type(*INT_FURI, Obj::to_type(INT_FURI));
      Typer::singleton()->save_type(*REAL_FURI, Obj::to_type(REAL_FURI));
      Typer::singleton()->save_type(*STR_FURI, Obj::to_type(STR_FURI));
      Typer::singleton()->save_type(*URI_FURI, Obj::to_type(URI_FURI));
      Typer::singleton()->save_type(*LST_FURI, Obj::to_type(LST_FURI));
      Typer::singleton()->save_type(*REC_FURI, Obj::to_type(REC_FURI));
      Typer::singleton()->save_type(*OBJS_FURI, Obj::to_type(OBJS_FURI));
      Typer::singleton()->save_type(*BCODE_FURI, Obj::to_type(BCODE_FURI));
      Typer::singleton()->save_type(*INST_FURI, Obj::to_type(INST_FURI));
      Typer::singleton()->save_type(*ERROR_FURI, Obj::to_type(ERROR_FURI));
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !ybase types!! loaded!g]!! \n",MMADT_SCHEME "/+"));
    }

     void* mmADT::import_ext_types() {
      Typer::singleton()->start_progress_bar(13);
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      Typer::singleton()->save_type(
          *CHAR_FURI,
          __(*CHAR_FURI, *INT_FURI, *STR_FURI).merge(jnt(2)).count().is(__().eq(jnt(1))));
      Typer::singleton()->save_type(*INT8_FURI, __(*UINT8_FURI, *INT_FURI, *INT_FURI)
                                    .is(__().gte(jnt(-127)))
                                    .is(__().lte(jnt(128))));
      Typer::singleton()->save_type(
          *UINT8_FURI,
          __(*UINT8_FURI, *INT_FURI, *INT_FURI).is(__().gte(jnt(0))).is(__().lte(jnt(255))));
      Typer::singleton()->save_type(*INT16_FURI, Obj::to_type(INT16_FURI));
      Typer::singleton()->save_type(*INT32_FURI, Obj::to_type(INT32_FURI));
      Typer::singleton()->save_type(*NAT_FURI, __(*NAT_FURI, *INT_FURI, *INT_FURI).is(__().gte(jnt(0))));
      Typer::singleton()->save_type(
          *CELSIUS_FURI,
          __(*CELSIUS_FURI, *REAL_FURI, *REAL_FURI).is(__().gte(real(-273.15))));
      Typer::singleton()->save_type(
          *PERCENT_FURI,
          __(*PERCENT_FURI, *REAL_FURI, *REAL_FURI).is(__().gte(real(0.0))).is(__().lte(real(100.0))));
      Typer::singleton()->save_type(*HEX_FURI, __(*HEX_FURI, *URI_FURI, *URI_FURI).is(dool(true)));
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      Typer::singleton()->save_type(*MILLISECOND_FURI, Obj::to_type(REAL_FURI));
      InstBuilder::build(MILLISECOND_FURI->add_component(MMADT_PREFIX "as"))
          ->domain_range(MILLISECOND_FURI, {1, 1}, SECOND_FURI, {1, 1})
          ->inst_args(lst({__().is(__().eq(vri(SECOND_FURI)))}))
          ->inst_f([](const Obj_p &millis_obj, const InstArgs &) {
            return Obj::to_real(millis_obj->real_value() * 1000.0, SECOND_FURI);
          })
          ->save();
      Typer::singleton()->save_type(*SECOND_FURI, Obj::to_type(REAL_FURI));
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      Typer::singleton()->save_type(*SECRET_FURI, Obj::to_type(STR_FURI));
      InstBuilder::build(SECRET_FURI->add_component(MMADT_PREFIX "as"))
          ->domain_range(SECRET_FURI, {1, 1}, OBJ_FURI, {1, 1})
          ->type_args(x(0, "type"))
          ->inst_f([](const Obj_p &secret_obj, const InstArgs &args) {
            if(args->arg(0)->is_uri() && ROUTER_RESOLVE(args->arg(0)->uri_value()).equals(*STR_FURI)) {
              const size_t length = secret_obj->str_value().length();
              return Obj::to_str(StringHelper::repeat(length, "*"), STR_FURI);
            }
            return ROUTER_READ(MMADT_PREFIX "as")->apply(secret_obj);
          })
          ->save();
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yextension types!! loaded!g]!! \n",MMADT_PREFIX "ext/+"));
      return nullptr;
    }

     void mmADT::import_base_inst() {
      ROUTER_WRITE("/mmadt/inst/blockers", Obj::to_lst({
                       vri("block"),
                       vri("each"),
                       vri("within"),
                       vri("isa"),
                       vri("split"),
                       vri("choose"),
                       vri("chain")}), true);
      Typer::singleton()->start_progress_bar(TOTAL_INSTRUCTIONS);
      InstBuilder::build(MMADT_PREFIX "embed")
          ->domain_range(OBJ_FURI, {1, 1}, REC_FURI, {1, 1})
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            return obj->embedding();
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "flip")
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {1, 1})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return lhs->apply(args->arg(0));
          })
          ->save();

      /* InstBuilder::build(MMADT_PREFIX "a")
           ->domain_range(OBJ_FURI, {1, 1}, BOOL_FURI, {1, 1})
           ->inst_args(lst({Obj::to_bcode()}))
           ->inst_f([](const Obj_p &obj, const InstArgs &args) {
             return dool(obj->match(args->arg(0)));
             //return dool(Compiler(false, false).type_check(obj, args->arg(0)->uri_value()));
           })
           ->save();*/

      /*InstBuilder::build(MMADT_PREFIX "jump")
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
          ->inst_args(lst({__().isa(*URI_FURI)}))
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            const fURI furi = args->arg(0)->uri_value();
            if(furi == "none")
              return obj;
            else {
              const Inst_p to_inst = ROUTER_READ(args->arg(0)->uri_value());
              const Inst_p from_inst = ROUTER_READ("_inst");
              LOG(INFO, "current inst: %s\n", from_inst->toString().c_str());
              return to_inst->apply(obj, Obj::to_rec({{"_back", from_inst}}));
            }
          })
          ->save();*/

      /*  InstBuilder::build(MMADT_PREFIX "back")
            ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
            // ->inst_args(lst({*__()->isa(*URI_FURI)}))
            ->inst_f([](const Obj_p &obj, const InstArgs &args) {
              const Inst_p inst = ROUTER_READ("_back");
              ROUTER_PUSH_FRAME("#", Obj::to_rec({{"_back", obj}}));
              return inst->apply(obj, Obj::to_rec({{"_back", vri("none")}}));
            })
            ->save();*/

      InstBuilder::build(MMADT_PREFIX "not")
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            return args->arg(0)->apply(obj)->is_noobj() ? obj : Obj::to_noobj();
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "isa")
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            std::stack<string> fail_reason;
            Obj_p result = obj->match(ROUTER_READ(args->arg(0)->uri_value()), &fail_reason) ? obj : Obj::to_noobj();
            if(result->is_noobj())
              LOG_WRITE(DEBUG, obj.get(), L("isa({}) mismatch {}\n", args->arg(0)->toString(),
                                            PrintHelper::print_fail_reason(&fail_reason))                  );
            return result;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "start")
          ->domain_range(NOOBJ_FURI, {0, 0}, OBJS_FURI, {0,INT_MAX})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return args->arg(0)->is_objs() ? args->arg(0) : Obj::to_objs({args->arg(0)});
          })
          ->save();

      /*InstBuilder::build(MMADT_PREFIX "apply")
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0,INT_MAX})
          ->inst_args(rec({{"rhs", Obj::to_noobj()}, {"args", Obj::to_rec()}}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            Obj_p rhs_temp = args->arg("rhs");
            Obj_p args_temp = args->arg("args");
            // if(rhs_temp->is_inst())
            //  return rhs_temp->apply(lhs, args);
            return lhs->apply(rhs_temp, args_temp->is_rec() ? args_temp : Obj::to_inst_args(*args_temp->lst_value()));
          })
          ->save();*/

      InstBuilder::build(MMADT_PREFIX "frame")
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &) {
            return ROUTER_GET_FRAME_DATA();
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "print")
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            string new_line;
            if(args->arg(0)->is_str()) {
              auto line = args->arg(0)->str_value();
              new_line = StringHelper::replace_groups(&line, "{", "}", [lhs](const string &match) {
                const Obj_p result = OBJ_PARSER(match)->apply(lhs);
                const string clean = result->none_one_all()->toString(NO_ANSI_PRINTER);
                return clean;
              });
            } else {
              new_line = args->arg(0)->toString(NO_ANSI_PRINTER);
            }
            if(new_line.length() > 1 &&
               new_line[new_line.length() - 1] == 'n' &&
               new_line[new_line.length() - 2] == '\\')
              printer()->println(new_line.substr(0, new_line.length() - 2).c_str());
            else
              printer()->print(new_line.c_str());
            return lhs;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "as")
          ->inst_args(rec({{"type?uri", Obj::to_bcode()}}))
          //->domain_range()
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            // uri or obj (for type obj)
            const Obj_p type_uri = args->arg("type?uri");
            return type_uri->is_uri()
                     ? lhs->as(id_p(ROUTER_RESOLVE(type_uri->uri_value())))
                     : lhs->as(type_uri->tid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "bool/" MMADT_INST_SCHEME "/as")
          ->domain_range(BOOL_FURI, {1, 1}, OBJ_FURI, {1, 1})
          ->inst_args(lst({isa_arg(URI_FURI)}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            // uri or obj (for type obj)
            if(args->arg(0)->is_uri()) {
              const ID_p resolution = id_p(ROUTER_RESOLVE(args->arg(0)->uri_value()));
              if(resolution->equals(*BOOL_FURI))
                return lhs->as(resolution);
              if(resolution->equals(*INT_FURI))
                return jnt(lhs->bool_value() ? 1 : 0);
              if(resolution->equals(*REAL_FURI))
                return real(lhs->bool_value() ? 1.0 : 0.0);
              if(resolution->equals(*STR_FURI))
                return str(lhs->toString(NO_ANSI_PRINTER));
              if(resolution->equals(*URI_FURI))
                return vri(lhs->toString(NO_ANSI_PRINTER));
              return lhs->as(resolution);
            }
            return lhs->as(args->arg(0)->tid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "int/" MMADT_INST_SCHEME "/as")
          ->domain_range(INT_FURI, {1, 1}, OBJ_FURI, {1, 1})
          ->inst_args(lst({isa_arg(URI_FURI)}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            // uri or obj (for type obj)
            if(args->arg(0)->is_uri()) {
              const ID_p resolution = id_p(ROUTER_RESOLVE(args->arg(0)->uri_value()));
              if(resolution->equals(*BOOL_FURI))
                return dool(lhs->int_value() > 0);
              if(resolution->equals(*INT_FURI))
                return lhs->as(resolution);
              if(resolution->equals(*REAL_FURI))
                return real(static_cast<FOS_REAL_TYPE>(lhs->int_value()));
              if(resolution->equals(*STR_FURI))
                return str(lhs->toString(NO_ANSI_PRINTER));
              if(resolution->equals(*URI_FURI))
                return vri(lhs->toString(NO_ANSI_PRINTER));
              return lhs->as(resolution);
            }
            return lhs->as(args->arg(0)->tid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "real/" MMADT_INST_SCHEME "/as")
          ->domain_range(REAL_FURI, {1, 1}, OBJ_FURI, {1, 1})
          ->inst_args(lst({isa_arg(URI_FURI)}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            // uri or obj (for type obj)
            if(args->arg(0)->is_uri()) {
              const ID_p resolution = id_p(ROUTER_RESOLVE(args->arg(0)->uri_value()));
              if(resolution->equals(*BOOL_FURI))
                return dool(lhs->real_value() > 0.0);
              if(resolution->equals(*INT_FURI))
                return jnt(static_cast<FOS_INT_TYPE>(lhs->real_value()));
              if(resolution->equals(*REAL_FURI))
                return lhs->as(resolution);
              if(resolution->equals(*STR_FURI))
                return str(lhs->toString(NO_ANSI_PRINTER));
              if(resolution->equals(*URI_FURI))
                return vri(lhs->toString(NO_ANSI_PRINTER));
              return lhs->as(resolution);
            }
            return lhs->as(args->arg(0)->tid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "str/" MMADT_INST_SCHEME "/as")
          ->domain_range(STR_FURI, {1, 1}, OBJ_FURI, {1, 1})
          ->inst_args(lst({isa_arg(URI_FURI)}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            // uri or obj (for type obj)
            if(args->arg(0)->is_uri()) {
              const ID_p resolution = id_p(ROUTER_RESOLVE(args->arg(0)->uri_value()));
              if(resolution->equals(*BOOL_FURI))
                return dool(lhs->str_value() == "true");
              if(resolution->equals(*INT_FURI))
                return jnt(static_cast<FOS_INT_TYPE>(atoi(lhs->str_value().c_str())));
              if(resolution->equals(*REAL_FURI))
                return
                    real(static_cast<FOS_REAL_TYPE>(atof(lhs->str_value().c_str())));
              if(resolution->equals(*STR_FURI))
                return lhs->as(resolution);
              if(resolution->equals(*URI_FURI))
                return vri(lhs->str_value());
              return lhs->as(resolution);
            }
            return lhs->as(args->arg(0)->tid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "uri/" MMADT_INST_SCHEME "/as")
          ->domain_range(URI_FURI, {1, 1}, OBJ_FURI, {1, 1})
          ->inst_args(lst({isa_arg(URI_FURI)}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            // uri or obj (for type obj)
            if(args->arg(0)->is_uri()) {
              const ID_p resolution = id_p(ROUTER_RESOLVE(args->arg(0)->uri_value()));
              if(resolution->equals(*BOOL_FURI))
                return dool(lhs->uri_value().toString() == "true");
              if(resolution->equals(*INT_FURI))
                return jnt(static_cast<FOS_INT_TYPE>(atoi(lhs->uri_value().toString().c_str())));
              if(resolution->equals(*REAL_FURI))
                return real(static_cast<FOS_REAL_TYPE>(atof(lhs->uri_value().toString().c_str())));
              if(resolution->equals(*STR_FURI))
                return vri(lhs->str_value());
              if(resolution->equals(*URI_FURI))
                return lhs->as(resolution);
              return lhs->as(resolution);
            }
            return lhs->as(args->arg(0)->tid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "at")
          ->inst_args(lst({isa_arg(URI_FURI)}))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Obj_p new_lhs = lhs->is_noobj() ? ROUTER_READ(args->arg(0)->uri_value()) : lhs;
            return (new_lhs->is_noobj() || new_lhs->vid) ? new_lhs : new_lhs->at(id_p(args->arg(0)->uri_value()));
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "lshift")->save();
      InstBuilder::build(MMADT_PREFIX "rshift")->save();

      InstBuilder::build(MMADT_PREFIX "rec/" MMADT_INST_SCHEME "/lshift")
          ->inst_args(lst({isa_arg(INT_FURI)}))
          ->domain_range(REC_FURI, {1, 1}, REC_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const auto new_rec = make_shared<Obj::RecMap<>>();
            if(args->arg(0)->is_int() || args->arg(0)->is_noobj()) {
              const int steps = args->arg(0)->is_noobj() ? 1 : args->arg(0)->int_value();
              for(const auto &[key, value]: *lhs->rec_value()) {
                if(const fURI new_key = key->uri_value().pretract(steps); !new_key.empty())
                  new_rec->emplace(vri(new_key), value);
              }
            } else {
              const fURI top_level = args->arg(0)->uri_value();
              for(const auto &[key, value]: *lhs->rec_value()) {
                if(const fURI old_key = key->uri_value(); old_key.starts_with(top_level)) {
                  if(const fURI new_key = old_key.pretract(top_level.path_length()); !new_key.empty())
                    new_rec->emplace(vri(new_key), value);
                }
              }
            }
            return Obj::to_rec(new_rec, REC_FURI, lhs->vid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "uri/" MMADT_INST_SCHEME "/lshift")
          ->inst_args(lst({Obj::to_bcode()}))
          ->domain_range(URI_FURI, {1, 1}, URI_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            if(args->arg(0)->is_int() || args->arg(0)->is_noobj()) {
              return Obj::to_uri(lhs->uri_value().pretract(args->arg(0)->is_int() ? args->arg(0)->int_value() : 1),
                                 URI_FURI, lhs->vid);
            } else {
              return Obj::to_uri(lhs->uri_value().pretract(args->arg(0)->uri_value()), URI_FURI, lhs->vid);
            }
          })
          ->save();


      InstBuilder::build(MMADT_PREFIX "uri/" MMADT_INST_SCHEME "/rshift")
          ->inst_args(lst({Obj::to_bcode()}))
          ->domain_range(URI_FURI, {1, 1}, URI_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            if(args->arg(0)->is_int() || args->arg(0)->is_noobj()) {
              return Obj::to_uri(lhs->uri_value().retract(args->arg(0)->is_int() ? args->arg(0)->int_value() : 1),
                                 URI_FURI, lhs->vid);
            } else {
              return Obj::to_uri(lhs->uri_value().retract(args->arg(0)->uri_value()), URI_FURI, lhs->vid);
            }
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "rec/" MMADT_INST_SCHEME "/rshift")
          ->inst_args(lst({isa_arg(URI_FURI)}))
          ->domain_range(REC_FURI, {1, 1}, REC_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const fURI prefix = args->arg(0)->uri_value();
            const Rec_p new_rec = Obj::to_rec();
            for(const Pair<Obj_p, Obj_p> &pair: *lhs->rec_value()) {
              const fURI key = pair.first->uri_value();
              new_rec->rec_set(vri(prefix.extend(key)), pair.second);
            }
            return new_rec;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "block")
          // TODO: currently a "special" instruction (see inst->apply() for logic)
          ->inst_args(lst({Obj::to_bcode()}))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return args->arg(0);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "count")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, INT_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &) {
            return Obj::to_int(lhs->objs_value()->size());
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "drop")
          ->inst_args(lst({isa_arg(OBJ_FURI)}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return args->arg(0)->apply(lhs);
          })
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->save();

      /*InstBuilder::build(MMADT_PREFIX "pc_plus")
          ->type_args(x(0, "incr", Obj::to_bcode()))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            LOG_WRITE(INFO, lhs.get(), L("parent: {}", lhs->parent_ ? lhs->parent()->toString() : "nullptr"));
            return lhs->parent_ ? lhs->parent()->next_inst(lhs) : Obj::to_noobj();
          })->domain_range(INST_FURI, {1, 1}, INST_FURI, {0, 1})
          ->save();*/

      InstBuilder::build(MMADT_PREFIX "lift")
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return Obj::to_rec({{lhs, args->arg(0)}});
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "barrier")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, OBJS_FURI, {0,INT_MAX})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Objs_p &lhs, const InstArgs &args) {
            return args->arg(0)->apply(lhs);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "sum")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, OBJ_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &) {
            Obj_p sum = nullptr;
            for(const auto &obj: *lhs->objs_value()) {
              sum = sum ? sum->inst_apply("plus", {obj}) : obj;
            }
            return sum ? sum : Obj::to_noobj();
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "prod")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, OBJ_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &) {
            Obj_p prod = nullptr;
            for(const auto &obj: *lhs->objs_value()) {
              prod = prod ? prod->inst_apply("mult", std::vector<Obj_p>{obj}) : obj;
            }
            return prod ? prod : Obj::to_noobj();
          })
          ->save();

      /*InstBuilder::build(MMADT_PREFIX "reduce")
          ->domain_range(OBJS_FURI, {0,INT_MAX}, OBJ_FURI, {1, 1})
          ->inst_args(Obj::to_rec({{"reducer", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            Obj_p reduction = nullptr;
            const Obj_p reducer = args->arg("reducer");
            for(const auto &obj: *lhs->objs_value()) {
              reduction = nullptr != reduction
                            ? (reducer->is_inst()
                                 ? reduction->inst_apply(reducer, {obj})
                                 : reducer->apply(reduction, Obj::to_inst_args({obj})))
                            : obj;
            }
            return reduction ? reduction : Obj::to_noobj();
          })
          ->save();*/

      /* InstBuilder::build(MMADT_PREFIX "delay")->type_args(
               x(0, "millis", Obj::to_bcode()))
           ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
             fThread::current_process()->delay(args->arg(0)->int_value());
             return lhs;
           })->save();*/

      InstBuilder::build(MMADT_PREFIX "each")
          ->inst_args(lst({isa_arg(OBJ_FURI)}))
          ->save();

      InstBuilder::build(MMADT_PREFIX "lst/" MMADT_INST_SCHEME "/each")
          ->inst_args(lst({isa_arg(LST_FURI)}))
          ->domain_range(LST_FURI, LST_FURI)
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Lst_p ret = Obj::to_lst();
            const Lst_p rhs = args->arg(0);
            for(uint8_t i = 0; i < rhs->lst_value()->size(); i++) {
              if(i < lhs->lst_value()->size()) {
                ret->lst_add(rhs->lst_value()->at(i)->apply(lhs->lst_value()->at(i)));
              } else {
                ret->lst_add(rhs->lst_value()->at(i)->apply(Obj::to_noobj()));
              }
            }
            return ret;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "rec/" MMADT_INST_SCHEME "/each")
          ->inst_args(lst({isa_arg(REC_FURI)}))
          ->domain_range(REC_FURI, REC_FURI)
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Rec_p ret = Obj::to_rec();
            const Rec_p rhs = args->arg(0);
            for(const auto &[rk,rv]: *rhs->rec_value()) {
              bool found = false;
              for(const auto &[lk,lv]: *lhs->rec_value()) {
                if(lk->match(rk)) {
                  found = true;
                  ret->rec_set(rk->apply(lk), rv->apply(lv));
                }
              }
              if(!found)
                ret->rec_set(rk, rv->apply(Obj::to_noobj()));
            }
            return ret;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "end")
          ->domain_range(OBJ_FURI, {0,INT_MAX}, NOOBJ_FURI, {0, 0})
          ->inst_f([](const Obj_p &, const InstArgs &) {
            return noobj();
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "eq")
          ->domain_range(OBJ_FURI, BOOL_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return Obj::to_bool(lhs->equals(*args->arg(0)));
          })->save();

      InstBuilder::build(MMADT_PREFIX "else")
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {1, 1})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return lhs->is_noobj() ? args->arg(0) : lhs;
          })->save();

      InstBuilder::build(MMADT_PREFIX "explain")
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return lhs;
          })->save();

      InstBuilder::build(MMADT_PREFIX "from")
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_args(lst({Obj::to_bcode(), __().else_(Obj::to_noobj())}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Obj_p result = ROUTER_READ(args->arg(0)->uri_value());
            return result->is_noobj() ? args->arg(1) : result;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "lock")
          ->domain_range(OBJ_FURI, OBJ_FURI)
          ->inst_args(rec({{"user", Obj::to_bcode()}}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const fURI user = args->arg(0)->is_noobj()
                                ? fURI("") //current_thread()->vid)
                                : args->arg(0)->uri_value();
            return lhs->lock().has_value() ? lhs->unlock(user) : lhs->lock(user);
          })->save();

      InstBuilder::build(MMADT_PREFIX "is")
          // TODO: figure out how to get the opcode in obj insts
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return args->arg(0)->bool_value() ? lhs : Obj::to_noobj();
          })

          ->save();

      InstBuilder::build(MMADT_PREFIX "map")
          ->inst_args(lst({Obj::to_bcode()}))
          ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return args->arg(0);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "merge")
          ->domain_range(OBJ_FURI, {1, 1}, OBJ_FURI, {0, 1})
          // ->inst_args(lst({Obj::to_rec({{isa_arg(INT_FURI), Obj::to_bcode()}, {Obj::to_bcode(), jnt(INT32_MAX)}})}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const int max = args->rec_value()->empty() ? INT32_MAX : args->arg(0)->int_value();
            return max > 0 ? lhs : Obj::to_noobj();
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "uri/" MMADT_INST_SCHEME "/merge")
          ->domain_range(URI_FURI, {1, 1}, OBJS_FURI, {0,INT_MAX})
          //->inst_args(lst({Obj::to_rec({{isa_arg(INT_FURI), Obj::to_bcode()}, {Obj::to_bcode(), jnt(INT32_MAX)}})}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const int max = args->rec_value()->empty() ? INT32_MAX : args->arg(0)->int_value();
            const Objs_p objs = Obj::to_objs();
            const Objs_p paths = Obj::to_objs();
            for(int i = 0; i < lhs->uri_value().path_length() && i < max; i++) {
              objs->add_obj(vri(lhs->uri_value().segment(i)));
            }
            return objs;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "str/" MMADT_INST_SCHEME "/merge")
          ->domain_range(STR_FURI, {1, 1}, OBJS_FURI, {0,INT_MAX})
          //->inst_args(lst({Obj::to_rec({{isa_arg(INT_FURI), Obj::to_bcode()}, {Obj::to_bcode(), jnt(INT32_MAX)}})}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const int max = args->rec_value()->empty() ? INT32_MAX : args->arg(0)->int_value();
            const Objs_p objs = Obj::to_objs();
            int counter = 0;
            const string mstring = lhs->str_value();
            const char *chars = mstring.c_str();
            for(int i = 0; i < mstring.length(); i++) {
              if(i >= max)
                break;
              const char x = chars[i];
              objs->add_obj(str(string(1, x)));
            }
            return objs;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "lst/" MMADT_INST_SCHEME "/merge")
          ->domain_range(LST_FURI, {1, 1}, OBJS_FURI, {0,INT_MAX})
          //->inst_args(lst({Obj}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const int max = args->rec_value()->empty() ? INT32_MAX : args->arg(0)->int_value();
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

      InstBuilder::build(MMADT_PREFIX "rec/" MMADT_INST_SCHEME "/merge")
          ->domain_range(REC_FURI, {1, 1}, OBJS_FURI, {0,INT_MAX})
          //->inst_args(lst({Obj::to_rec({{isa_arg(INT_FURI), Obj::to_bcode()}, {Obj::to_bcode(), jnt(INT32_MAX)}})}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const int max = args->rec_value()->empty() ? INT32_MAX : args->arg(0)->int_value();
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

      InstBuilder::build(MMADT_PREFIX "neq")
          ->domain_range(OBJ_FURI, BOOL_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return Obj::to_bool(!lhs->equals(*args->arg(0)));
          })->save();

      InstBuilder::build(MMADT_PREFIX "repeat")
          ->type_args(x(0, "code"), x(1, "until", dool(true)), x(2, "emit", dool(false)))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Obj_p until = args->arg(1);
            Obj_p result = lhs;
            while(!until->apply(result)->bool_value()) {
              result = std::move(args->arg(0)->apply(result));
            }
            return result;
          })
          ->save();

      int op_index = -1;
      for(const auto &op: {"split", "choose", "chain"}) {
        ++op_index;
        InstBuilder::build(string(MMADT_PREFIX).append(op))
            ->inst_args(lst({Obj::to_bcode()}))
            ->inst_f([op_index](const Obj_p &lhs, const InstArgs &args) {
              const Obj_p rhs = args->arg(0);
              switch(op_index) {
                case 0: { // split
                  if(rhs->is_lst()) {
                    Lst_p list = Obj::to_lst();
                    for(const auto &robj: *rhs->lst_value()) {
                      list->lst_add(robj->apply(lhs));
                    }
                    return list;
                  } else if(rhs->is_rec()) {
                    Rec_p record = Obj::to_rec();
                    for(const auto &[rk,rv]: *rhs->rec_value()) {
                      record->rec_set(rk, lhs->match(rk) ? rv->apply(lhs) : Obj::to_noobj());
                    }
                    return record;
                  } else {
                    return rhs->apply(lhs);
                  }
                }
                case 1: { // choose
                  if(rhs->is_lst()) {
                    Lst_p list = Obj::to_lst();
                    bool done = false;
                    for(const auto &robj: *rhs->lst_value()) {
                      if(done)
                        list->lst_add(Obj::to_noobj());
                      else {
                        if(Obj_p result = robj->apply(lhs); !result->is_noobj()) {
                          list->lst_add(result);
                          done = true;
                        }
                      }
                    }
                    return list;
                  } else if(rhs->is_rec()) {
                    Rec_p record = Obj::to_rec();
                    bool done = false;
                    for(const auto &[rk,rv]: *rhs->rec_value()) {
                      if(done)
                        record->rec_set(rk, Obj::to_noobj());
                      else {
                        if(lhs->match(rk)) {
                          record->rec_set(rk, rv->apply(lhs));
                          done = true;
                        }
                      }
                    }
                    return record;
                  } else {
                    return rhs->apply(lhs);
                  }
                }
                case 2: { // chain
                  if(rhs->is_lst()) {
                    if(rhs->lst_value()->empty())
                      return Obj::to_lst();
                    Lst_p list = Obj::to_lst();
                    for(const auto &robj: *rhs->lst_value()) {
                      if(list->lst_value()->empty())
                        list->lst_add(robj->apply(lhs));
                      else
                        list->lst_add(robj->apply(list->lst_value()->back()));
                    }
                    return list;
                  } else if(rhs->is_rec()) {
                    Rec_p record = Obj::to_rec();
                    for(const auto &[rk,rv]: *rhs->rec_value()) {
                      if(record->rec_value()->empty()) {
                        record->rec_set(rk, lhs->match(rk) ? rv->apply(lhs) : Obj::to_noobj());
                      } else {
                        const Obj_p last_lhs = record->rec_value()->back().second;
                        record->rec_set(rk, last_lhs->match(rk) ? rv->apply(last_lhs) : Obj::to_noobj());
                      }
                    }
                    return record;
                  } else {
                    return rhs->apply(lhs);
                  }
                }
                default:
                  throw fError("unknown instruction: %i", op_index);
              }
            })
            ->save();
      }

      /*  InstBuilder::build(MMADT_PREFIX "split")
            ->inst_args(lst({Obj::to_bcode()}))
            ->inst_f([](const Obj_p &, const InstArgs &args) {
              return args->arg(0);
            })
            ->save();*/


      /// TODO: this is for play -- can remove
      InstBuilder::build(MMADT_PREFIX "lst/" MMADT_INST_SCHEME "/split")
          ->inst_args(lst({isa_arg(LST_FURI)}))
          ->inst_f([](const Obj_p &obj, const InstArgs &args) {
            const Obj_p rhs = args->arg(0);
            Lst_p list = Obj::to_lst();
            for(const auto &lhs: *obj->lst_value()) {
              list->lst_add(rhs->apply(lhs));
            }
            return list;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "to")
          ->inst_args(lst({Obj::to_bcode(), dool(true)}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            ROUTER_WRITE(args->arg(0)->uri_value(), lhs, args->arg(1)->bool_value());
            return lhs;
          })->save();

      InstBuilder::build(MMADT_PREFIX "ref")
          ->inst_args(rec({
              {"payload", Obj::to_bcode()},
              {"retain?bool", __().isa(*BOOL_FURI).else_(dool(true))}}))
          ->domain_range(URI_FURI, {1, 1}, OBJ_FURI, {0, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Obj_p payload = args->arg("payload");
            ROUTER_WRITE(lhs->uri_value(), payload, args->arg("retain")->bool_value());
            return payload;
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "type")
          ->domain_range(OBJ_FURI, {0, 1}, URI_FURI, {1, 1})
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return Obj::to_uri(*args->arg(0)->tid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "within")
          ->inst_args(lst({Obj::to_bcode()}))
          ->save();

      InstBuilder::build(MMADT_PREFIX "str/" MMADT_INST_SCHEME "/within")
          ->inst_args(lst({Obj::to_bcode()}))
          ->domain_range(STR_FURI, {1, 1}, STR_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const auto chars = make_shared<List<Str_p>>();
            const string xstr = lhs->str_value();
            for(size_t i = 0; i < xstr.length(); i++) {
              chars->push_back(str(xstr.substr(i, 1)));
            }
            const BCode_p code = args->arg(0)->bcode_starts(Obj::to_objs(chars));
            const Objs_p strs = BCODE_PROCESSOR(code);
            string ret;
            for(const Str_p &s: *strs->objs_value()) {
              ret += s->str_value();
            }
            return Obj::to_str(ret, lhs->tid, lhs->vid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "lst/" MMADT_INST_SCHEME "/within")
          ->inst_args(lst({Obj::to_bcode()}))
          ->domain_range(LST_FURI, {1, 1}, LST_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const BCode_p starts_bcode = args->arg(0)->bcode_starts(Obj::to_objs(lhs->lst_value()));
            return Obj::to_lst(BCODE_PROCESSOR(starts_bcode)->objs_value(), lhs->tid, lhs->vid);
          })
          ->save();

      InstBuilder::build(MMADT_PREFIX "rec/" MMADT_INST_SCHEME "/within")
          ->inst_args(lst({Obj::to_bcode()}))
          ->domain_range(REC_FURI, {1, 1}, REC_FURI, {1, 1})
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            const Objs_p pairs = Obj::to_objs();
            for(const auto &pair: *lhs->rec_value()) {
              pairs->add_obj(Obj::to_lst({pair.first, pair.second}));
            }
            const Objs_p results = args->arg(0)->apply(pairs);
            const Obj::RecMap_p<> rec = make_shared<Obj::RecMap<>>();
            for(const auto &result: *results->objs_value()) {
              rec->insert({result->lst_value()->at(0), result->lst_value()->at(1)});
            }
            return Obj::to_rec(rec, lhs->tid, lhs->vid);
          })
          ->save();

      /////////////////////////// RELATIONAL PREDICATE INSTS ///////////////////////////
      for(const auto &i: {"gt", "gte", "lt", "lte"}) {
        InstBuilder::build(MMADT_ID->extend(i))
            ->domain_range(OBJ_FURI, {0, 1}, BOOL_FURI, {1, 1})
           // ->inst_f([](const Obj_p&, const InstArgs&) { return dool(false); })
            ->save();
        for(const auto &t: {INT_FURI, REAL_FURI, STR_FURI, URI_FURI}) {
          InstBuilder *builder =
              InstBuilder::build(t->resolve(string(MMADT_INST_SCHEME).append("/").append(i)))
              ->domain_range(t, BOOL_FURI)
              ->inst_args(rec({{"arg?" + t->name(),Obj::to_bcode()}}));
          if(strcmp(i, "gt") == 0) {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs > *args->arg(0));
            });
          } else if(strcmp(i, "gte") == 0) {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs >= *args->arg(0));
            });
          } else if(strcmp(i, "lt") == 0) {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs < *args->arg(0));
            });
          } else if(strcmp(i, "lte") == 0) {
            builder->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              return Obj::to_bool(*lhs <= *args->arg(0));
            });
          }
          builder->save();
        }
      }
      /////////////////////////// INSPECT INST ///////////////////////////
      InstBuilder::build(MMADT_PREFIX "inspect")
          ->domain_range(OBJ_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->save();
      InstBuilder::build(MMADT_PREFIX "bool/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(BOOL_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            return rec;
          })->save();

      InstBuilder::build(MMADT_PREFIX "int/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(INT_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("value/encoding", vri(STR(FOS_INT_TYPE)));
            return rec;
          })->save();

      InstBuilder::build(MMADT_PREFIX "real/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(REAL_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("value/encoding", vri(STR(FOS_REAL_TYPE)));
            return rec;
          })->save();

      InstBuilder::build(MMADT_PREFIX "str/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(STR_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("value/length", jnt(args->arg(0)->str_value().size()));
            rec->rec_set("value/encoding", vri(string("UTF") + to_string(8 * sizeof(char))));
            return rec;
          })->save();

      InstBuilder::build(MMADT_PREFIX "uri/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(URI_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            const fURI furi = args->arg(0)->uri_value();
            if(furi.has_scheme())
              rec->rec_set("value/scheme", vri(furi.scheme()));
            if(furi.has_user())
              rec->rec_set("user", vri(furi.user()));
            if(furi.has_password())
              rec->rec_set("value/password", vri(furi.password()));
            if(furi.has_host())
              rec->rec_set("value/host", vri(furi.host()));
            if(furi.has_port())
              rec->rec_set("value/port", jnt(furi.port()));
            rec->rec_set("value/relative", dool(furi.is_relative()));
            rec->rec_set("value/branch", dool(furi.is_branch()));
            rec->rec_set("value/headless", dool(furi.headless()));
            rec->rec_set("value/pattern", dool(furi.is_pattern()));
            if(furi.has_path()) {
              const Lst_p path = Obj::to_lst();
              for(int i = 0; i < furi.path_length(); i++) {
                path->lst_add(vri(furi.segment(i)));
              }
              rec->rec_set("value/path", path);
            }
            if(furi.has_components()) {
              const Lst_p comps = Obj::to_lst();
              for(const auto &comp: furi.components()) {
                comps->lst_add(vri(comp));
              }
              rec->rec_set("value/components", comps);
            }
            if(furi.has_query()) {
              rec->rec_set("value/query", str(furi.query()));
            }
            return rec;
          })->save();

      InstBuilder::build(MMADT_PREFIX "lst/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(LST_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            args->arg(0)->lst_set("value/size", args->arg(0)->lst_size());
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


      InstBuilder::build(MMADT_PREFIX "rec/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(REC_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("value/size", args->arg(0)->rec_size());
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

      InstBuilder::build(MMADT_PREFIX "inst/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(INST_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            const Rec_p rec = build_inspect_rec(args->arg(0));
            rec->rec_set("op", str(args->arg(0)->inst_op().c_str()));
            rec->rec_set("args", args->arg(0)->inst_args());
            // rec->rec_set("f", str(ITypeDescriptions.to_chars(args->arg(0)->itype())));
            rec->rec_set(FOS_DOMAIN, vri(*args->arg(0)->domain()));
            // TODO: coefficients as lsts
            rec->rec_set(FOS_RANGE, vri(*args->arg(0)->range()));
            rec->rec_set("f", std::holds_alternative<Obj_p>(args->arg(0)->inst_f())
                                ? std::get<Obj_p>(args->arg(0)->inst_f())
                                : Obj::to_noobj());
            return rec;
          })->save();

      InstBuilder::build(MMADT_PREFIX "bcode/" MMADT_INST_SCHEME "/inspect")
          ->domain_range(BCODE_FURI, REC_FURI)
          ->inst_args(lst({Obj::to_bcode()}))
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
      InstBuilder::build(MMADT_PREFIX "mod")->type_args(x(0, "rhs"))->save();
      InstBuilder::build(MMADT_PREFIX "int/" MMADT_INST_SCHEME "/mod")
          ->domain_range(INT_FURI, {1, 1}, INT_FURI, {1, 1})
          ->inst_args(lst({isa_arg(INT_FURI)}))
          ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
            return jnt(lhs->int_value() % args->arg(0)->int_value());
          })
          ->save();
      ////////////////////////////// NEGATIVE /////////////////////////////////////
      InstBuilder::build(MMADT_PREFIX "neg")
          ->inst_args(lst({isa_arg(OBJ_FURI)}))
          ->save();

      InstBuilder::build(MMADT_PREFIX "bool/" MMADT_INST_SCHEME "/neg")
          ->inst_args(lst({isa_arg(BOOL_FURI)}))
          ->domain_range(BOOL_FURI, {1, 1}, BOOL_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return dool(!args->arg(0)->bool_value(), args->arg(0)->vid);
          })
          ->save();
      InstBuilder::build(MMADT_PREFIX "int/" MMADT_INST_SCHEME "/neg")
          ->inst_args(lst({isa_arg(INT_FURI)}))
          ->domain_range(INT_FURI, {1, 1}, INT_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return jnt(args->arg(0)->int_value() * -1, args->arg(0)->vid);
          })
          ->save();
      InstBuilder::build(MMADT_PREFIX "real/" MMADT_INST_SCHEME "/neg")
          ->inst_args(lst({isa_arg(REAL_FURI)}))
          ->domain_range(REAL_FURI, {1, 1}, REAL_FURI, {1, 1})
          ->inst_f([](const Obj_p &, const InstArgs &args) {
            return real(args->arg(0)->real_value() * -1.0f, args->arg(0)->vid);
          })
          ->save();
      ////////////////////////////// PLUS/MULT ///////////////////////////////////
      for(const auto &op: {"plus", "minus", "mult", "div"}) {
        const ID MMADT_INST = MMADT_ID->extend(op);
        InstBuilder::build(MMADT_INST)->inst_args(lst({Obj::to_bcode()}))->save();

        InstBuilder::build(string(MMADT_PREFIX "int/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(INT_FURI, {1, 1}, INT_FURI, {1, 1})
            ->inst_args(rec({{"0?int", isa_arg(INT_FURI)}}))
            ->inst_f(
                [op](const Obj_p &lhs, const InstArgs &args) {
                  if(strcmp(op, "plus") == 0)
                    return jnt(lhs->int_value() + args->arg(0)->int_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "minus") == 0)
                    return jnt(lhs->int_value() - args->arg(0)->int_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "mult") == 0)
                    return jnt(lhs->int_value() * args->arg(0)->int_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "div") == 0)
                    return jnt(lhs->int_value() / args->arg(0)->int_value(), lhs->tid, lhs->vid);
                  throw fError("unknown op %s\n", op);
                })
            ->save();

        InstBuilder::build(string(MMADT_PREFIX "real/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(REAL_FURI, {1, 1}, REAL_FURI, {1, 1})
            ->inst_args(lst({__().isa(*REAL_FURI)}))
            ->inst_f(
                [op](const Obj_p &lhs, const InstArgs &args) {
                  if(strcmp(op, "plus") == 0)
                    return real(lhs->real_value() + args->arg(0)->real_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "minus") == 0)
                    return real(lhs->real_value() - args->arg(0)->real_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "mult") == 0)
                    return real(lhs->real_value() * args->arg(0)->real_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "div") == 0)
                    return real(lhs->real_value() / args->arg(0)->real_value(), lhs->tid, lhs->vid);
                  throw fError("unknown op %s\n", op);
                })
            ->save();

        InstBuilder::build(string(MMADT_PREFIX "str/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(STR_FURI, {1, 1}, STR_FURI, {1, 1})
            ->inst_args(lst({__().isa(*STR_FURI)}))
            ->inst_f(
                [op](const Obj_p &lhs, const InstArgs &args) {
                  if(strcmp(op, "plus") == 0)
                    return str(lhs->str_value().append(args->arg(0)->str_value()), lhs->tid, lhs->vid);
                  if(strcmp(op, "minus") == 0) {
                    string temp = lhs->str_value();
                    StringHelper::replace(&temp, args->arg(0)->str_value(), "");
                    return str(temp, lhs->tid, lhs->vid);
                  }
                  if(strcmp(op, "mult") == 0) {
                    string temp;
                    for(const char c: lhs->str_value()) {
                      temp += c;
                      temp.append(args->arg(0)->str_value());
                    }
                    return str(temp, lhs->tid); // , lhs->vid
                  }
                  throw fError("unknown op %s\n", op);
                })
            ->save();

        InstBuilder::build(string(MMADT_PREFIX "bool/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(BOOL_FURI, {1, 1}, BOOL_FURI, {1, 1})
            ->inst_args(lst({__().isa(*BOOL_FURI)}))
            ->inst_f(
                [op](const Obj_p &lhs, const InstArgs &args) {
                  if(strcmp(op, "plus") == 0)
                    return dool(lhs->bool_value() || args->arg(0)->bool_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "minus") == 0)
                    return dool(lhs->bool_value() || !args->arg(0)->bool_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "mult") == 0)
                    return dool(lhs->bool_value() && args->arg(0)->bool_value(), lhs->tid, lhs->vid);
                  if(strcmp(op, "div") == 0)
                    return dool(lhs->bool_value() && !args->arg(0)->bool_value(), lhs->tid, lhs->vid);
                  throw fError("unknown op %s\n", op);
                })
            ->save();

        InstBuilder::build(string(MMADT_PREFIX "uri/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(URI_FURI, {1, 1}, URI_FURI, {1, 1})
            ->inst_args(lst({__().isa(*URI_FURI)}))
            ->inst_f(
                [op](const Obj_p &lhs, const InstArgs &args) {
                  std::vector<std::pair<string, string>> values_a = lhs->uri_value().query_values();
                  if(std::vector<std::pair<string, string>> values_b = args->arg(0)->uri_value().query_values();
                    values_b.empty())
                    values_a.insert(values_a.end(), values_b.begin(), values_b.end());
                  if(0 == strcmp(op, "plus"))
                    return vri(lhs->uri_value().extend(args->arg(0)->uri_value()), lhs->tid, lhs->vid);
                  if(0 == strcmp(op, "mult"))
                    return vri(lhs->uri_value().resolve(args->arg(0)->uri_value()).query(values_a), lhs->tid, lhs->vid);
                  if(0 == strcmp(op, "minus"))
                    return vri(lhs->uri_value().remove_subpath(args->arg(0)->uri_value().toString()), lhs->tid,
                               lhs->vid);
                  throw fError("unknown op %s\n", op);
                })
            ->save();

        InstBuilder::build(string(MMADT_PREFIX "lst/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(LST_FURI, {1, 1}, LST_FURI, {1, 1})
            ->inst_args(lst({__().isa(*LST_FURI)}))
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
                    return Obj::to_lst(new_v, lhs->tid, lhs->vid);
                  }
                  if(strcmp(op, "mult") == 0) {
                    const Obj::LstList_p lhs_v = lhs->lst_value();
                    const Obj::LstList_p rhs_v = args->arg(0)->lst_value();
                    const auto new_v = make_shared<Obj::LstList>();
                    for(int i = 0; i < lhs_v->size(); i++) {
                      for(int j = 0; j < rhs_v->size(); j++) {
                        new_v->push_back(lhs_v->at(i)->inst_apply("mult", std::vector<Obj_p>{rhs_v->at(j)}));
                      }
                    }
                    return Obj::to_lst(new_v, lhs->tid, lhs->vid);
                  }
                  throw fError("unknown op %s\n", op);
                })
            ->save();

        InstBuilder::build(string(MMADT_PREFIX "rec/" MMADT_INST_SCHEME "/").append(op).c_str())
            ->domain_range(REC_FURI, {1, 1}, REC_FURI, {1, 1})
            ->inst_args(lst({__().isa(*REC_FURI)}))
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
                    return Obj::to_rec(new_v, lhs->tid, lhs->vid);
                  }
                  if(strcmp(op, "mult") == 0) {
                    const Obj::RecMap_p<> lhs_v = lhs->rec_value();
                    const Obj::RecMap_p<> rhs_v = args->arg(0)->rec_value();
                    const auto new_v = make_shared<Obj::RecMap<>>();
                    const auto compiler = Compiler(true, false);
                    for(const auto &[k1,v1]: *lhs_v) {
                      for(const auto &[k2,v2]: *rhs_v) {
                        new_v->insert_or_assign(
                            compiler.resolve_inst(k1, Obj::to_inst({x(0, Obj::to_bcode())}, id_p("mult")))->apply(k2),
                            compiler.resolve_inst(v1, Obj::to_inst({x(0, Obj::to_bcode())}, id_p("mult")))->apply(v2));
                      }
                    }
                    return Obj::to_rec(new_v, lhs->tid, lhs->vid);
                  }
                  throw fError("unknown op %s\n", op);
                })
            ->save();
      }
      ///////////////////////////////////////////////////////////////////////////////////////////////////////
      Typer::singleton()->end_progress_bar(
          StringHelper::format("\n\t\t!^u1^ !g[!b%s !yobj insts!! loaded!g]!! \n",
                               MMADT_PREFIX "+/" COMPONENT_SEPARATOR MMADT_PREFIX "+"));
    }

     void *mmADT::import() {
      mmADT::import_base_types();
      mmADT::import_base_inst();
      return nullptr;
    }
} // namespace mmadt
