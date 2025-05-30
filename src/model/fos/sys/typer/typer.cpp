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

#include "typer.hpp"
#include "../../../../lang/mmadt/mmadt.hpp"
#include "../../../../lang/obj.hpp"
#include "../router/memory/memory.hpp"
namespace fhatos {
  using namespace mmadt;

  Typer::Typer(const ID &value_id) : Obj(rmap({{"module", Obj::to_rec()}}), OType::REC, REC_FURI, id_p(value_id)) {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    TYPER_SAVE_TYPE = [this](const ID &type_id, const Obj_p &type_def) { this->save_type(type_id, type_def); };
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    TYPE_INST_RESOLVER = [](const Obj_p &lhs, const Inst_p &inst) -> Inst_p {
      using DerivationTree = List<Trip<ID_p, ID_p, Obj_p>>;
      LOG_WRITE(DEBUG, Typer::singleton().get(), L(" !yresolving!! !yinst!! %s [!gSTART!!]\n", inst->toString()));
      // const auto compiler = Compiler(true, false);
      if(inst->is_noobj())
        return inst;
      // if(!lhs->is_noobj())
      //   compiler.coefficient_check<IntCoefficient>(lhs->range_coefficient(), inst->domain_coefficient());
      const static auto TEMP = [](const Obj_p &lhs, const Inst_p &inst, DerivationTree *dt) {
        Obj_p current_obj = lhs;
        const ID inst_type_id = ROUTER_RESOLVE(static_cast<fURI>(*inst->tid));
        while(true) {
          Inst_p maybe;
          /////////////////////////////// INST VIA ID RESOLVE ///////////////////////////////
          /////////////////////////////// INST VIA VALUE ///////////////////////////////
          if(current_obj->vid) {
            LOG_WRITE(DEBUG, Typer::singleton().get(),
                      L("!m==>!!searching for !yinst!! !b{}!!\n", inst_type_id.toString()));
            const ID next_inst_type_id = current_obj->vid->add_component(inst_type_id);
            maybe = ROUTER_READ(next_inst_type_id);
            if(dt)
              dt->emplace_back(current_obj->vid, id_p(next_inst_type_id), maybe);
            if(!maybe->is_noobj() && maybe->is_inst() && maybe->has_inst_f())
              return maybe;
          }
          /////////////////////////////// INST VIA TYPE ///////////////////////////////
          // check for inst on obj type (if not, walk up the obj type tree till root)
          LOG_WRITE(DEBUG, Typer::singleton().get(),
                    L("!m==>!!searching for !yinst!! !b{}!!\n", inst_type_id.toString()));
          const ID next_inst_type_id = current_obj->tid->no_query().equals(*OBJ_FURI)
                                           ? inst_type_id // drop back to flat namespace
                                           : ID(current_obj->tid->no_query().add_component(inst_type_id));
          maybe = ROUTER_READ(next_inst_type_id);
          if(dt)
            dt->emplace_back(id_p(current_obj->tid->no_query()), id_p(next_inst_type_id), maybe);
          if(!maybe->is_noobj() && maybe->is_inst() && maybe->has_inst_f())
            return maybe;
          /////////////////////////////////////////////////////////////////////////////
          if(current_obj->tid->no_query().equals(
                 (current_obj = ROUTER_READ(current_obj->tid->no_query()))->tid->no_query())) {
            // infinite loop (i.e. base type)
            return noobj();
          }
        }
      };
      //////////////////////////////////////
      //////////////////////////////////////
      /////////////////////////////////////
      DerivationTree *dt = nullptr; // make_unique<DerivationTree>();
      if(dt)
        dt->emplace_back(id_p(""), id_p(""), Obj::to_noobj());
      const ID inst_type_id = ROUTER_RESOLVE(static_cast<fURI>(*inst->tid));
      Inst_p final_inst = ROUTER_READ(inst_type_id);
      if(dt)
        dt->emplace_back(id_p(""), id_p(inst_type_id), final_inst);
      if(final_inst->is_noobj() || !final_inst->is_inst() || !final_inst->has_inst_f()) {
        if(dt)
          dt->emplace_back(id_p(""), id_p(""), Obj::to_noobj());
        final_inst = TEMP(lhs, inst, dt);
        if(final_inst->is_noobj() || !final_inst->is_inst() || !final_inst->has_inst_f()) {
          const Obj_p next_lhs = ROUTER_READ(*lhs->tid);
          const ID next_id = next_lhs->range()->no_query();
          const Obj_p next_obj = ROUTER_READ(next_id);
          if(dt)
            dt->emplace_back(id_p(""), id_p(""), Obj::to_noobj());
          final_inst = TEMP(next_obj, inst, dt);
          if(final_inst->is_noobj() || !final_inst->is_inst() || !final_inst->has_inst_f()) {
            if(inst->has_inst_f())
              final_inst = inst;
          }
          //////////////////// generated printable derivation tree ////////////////////
          string derivation_string;
          if(final_inst->is_noobj() || !final_inst->is_inst() || !final_inst->has_inst_f()) {
            if(dt) {
              int counter = 0;
              for(const auto &oir: *dt) {
                counter = std::get<1>(oir)->empty() ? 0 : counter + 1;
                if(counter != 0) {
                  string indent = StringHelper::repeat(counter, "-").append("!g>!!");
                  derivation_string.append(StringHelper::format("\n\t!m%-8s!g[!b%-15s!g] !b%-30s!! !m=>!m !b%-35s!!",
                                                                indent.c_str(), std::get<0>(oir)->toString().c_str(),
                                                                std::get<1>(oir)->toString().c_str(),
                                                                std::get<2>(oir)->toString().c_str()));
                }
              }
            }
            derivation_string = dt ? derivation_string : "<no derivation>";
            throw fError(FURI_WRAP_C(m) " " FURI_WRAP " !yno inst!! resolution %s", lhs->tid->toString().c_str(),
                         inst->tid->toString().c_str(), derivation_string.c_str());
          }
        }
        ////////////////////////////////////////////////////////////////////////////////
      }
      if(final_inst->is_inst()) {
        LOG(TRACE, "merging resolved inst into provide inst\n\t\t%s => %s [!m%s!!]\n", final_inst->toString().c_str(),
            inst->toString().c_str(), "SIGNATURE HERE");
        const auto merged_args = Obj::to_inst_args();
        int counter = 0;
        for(const auto &[k, v]: *final_inst->inst_args()->rec_value()) {
          if(inst->has_arg(k))
            merged_args->rec_value()->insert({k, inst->arg(k)});
          else if(inst->is_indexed_args() && counter < inst->inst_args()->rec_value()->size())
            merged_args->rec_value()->insert({k, inst->arg(counter)});
          else
            merged_args->rec_value()->insert({k, v->is_inst() ? v->arg(1) : v});
          // TODO: hack to get the default from from();
          ++counter;
        }
        // TODO: recurse off inst for all inst_arg getter/setters
        final_inst = Obj::to_inst(final_inst->inst_op(), merged_args, final_inst->inst_f(),
                                  final_inst->inst_seed_supplier(), final_inst->tid, final_inst->vid);
        /// TODO ^--- inst->vid);
      } else {
        final_inst = Obj::to_inst(
            inst->inst_op(), inst->inst_args(),
            InstF(make_shared<Cpp>(
                [x = final_inst->clone()](const Obj_p &lhs, const InstArgs &args) -> Obj_p { return x->apply(lhs); })),
            inst->inst_seed_supplier(), inst->tid, inst->vid);
      }
      LOG_WRITE(DEBUG, lhs.get(), L(" !gresolved!! !yinst!! {} [!gEND!!]\n", final_inst->toString()));
      return final_inst;
    };
    LOG_WRITE(INFO, this, L("!gtyper!! started\n"));
  }
  ptr<Typer> &Typer::singleton(const ID &id) {
    static auto typer = make_shared<Typer>(id);
    if(BOOTING && !typer->vid->equals(id) && typer->vid->path().find("boot") != std::string::npos) {
      typer->vid = id_p(id);
      TYPER_ID = typer->vid;
      typer->save();
      LOG_WRITE(INFO, typer.get(), L("!gtyper!! !bid!! reassigned\n"));
    }
    return typer;
  }
  void Typer::set_filters(std::vector<fURI> *filters) { this->filters = filters; }
  void Typer::clear_filters() { this->filters = nullptr; }

  void Typer::start_progress_bar(const uint16_t size) {
    type_progress_bar_ = ProgressBar::start(Ansi<>::singleton().get(), size);
  }
  void Typer::end_progress_bar(const string &message) {
    if(type_progress_bar_) {
      type_progress_bar_->end(message);
      type_progress_bar_ = nullptr;
    }
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void Typer::register_module(const ID &module_id, const Inst_p &module) const {
    if(module->is_code()) {
      this->obj_set_component(module_id, module);
      LOG_WRITE(INFO, this,
                L("!b{} !g{} !ymodule!! registered\n", module_id.toString(), OTypes.to_chars(module->otype)));
    } else {
      LOG_WRITE(WARN, this,
                L("!b{} !r{} !ymodule !rnot!! registered!!\n", module_id.toString(), OTypes.to_chars(module->otype)));
    }
  }

  void Typer::import_module(const Pattern &module_furi) {
    const Objs_p x = ROUTER_READ(this->vid->add_component(module_furi));
    for(const Inst_p &module: x->is_objs() ? *x->objs_value() : std::vector<Obj_p>({x})) {
      if(const Rec_p mod = module->apply(Obj::to_noobj()); mod->is_rec()) {
        const size_t size = mod->rec_value()->size();
        const bool do_prog_bar = size > 5;
        if(do_prog_bar)
          this->start_progress_bar(size);
        for(const auto &[id, type_def]: *mod->rec_value()) {
          this->save_type(id->uri_value(), type_def);
        }
        const fURI module_id = module->vid_or_tid()->no_query();
        const string pretracted_module_id = module_id.pretract(this->vid->extend("::")).toString();
        if(do_prog_bar)
          this->end_progress_bar(format("\n\t\t!^u1^ !g[!b{} !ytypes!! imported!g]!!\n", pretracted_module_id));
        else
          LOG_WRITE(INFO, this, L("!b{} !ytypes!! imported!!\n", pretracted_module_id));
      } else {
        LOG_WRITE(ERROR, module.get(),
                  L("!b{} !ymodule !rnot a rec!!: {}\n", module_furi.toString(), module->toString()));
      }
    }
  }

  void Typer::import() {
    // const Rec_p modules = Typer::singleton()->rec_get("::/#/")->or_else(Rec::to_rec());
    // LOG(INFO, "HERE: %s\n", modules->toString().c_str());
    Typer::singleton()->obj_set_component(
        "register", InstBuilder::build(Typer::singleton()->vid->add_component("register"))
                        ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
                        ->inst_args("module_id?uri", __(), "module?inst", __())
                        ->inst_f([](const Obj_p &obj, const InstArgs &args) {
                          Typer::singleton()->register_module(args->arg("module_id")->uri_value(), args->arg("module"));
                          return obj;
                        })
                        ->create());
    Typer::singleton()->obj_set_component("import",
                                          InstBuilder::build(Typer::singleton()->vid->add_component("import"))
                                              ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 1})
                                              ->inst_args("modules", __())
                                              ->inst_f([](const Obj_p &obj, const InstArgs &args) {
                                                Typer::singleton()->import_module(args->arg("modules")->uri_value());
                                                return obj;
                                              })
                                              ->create());
    if(!ROUTER_READ(ROUTER_ID->extend("memory"))->is_noobj()) {
      Memory::register_module();
      Typer::singleton()->import_module(*MEMORY_FURI);
      const ptr<Memory> mem = Memory::singleton(ROUTER_ID->extend("memory"));
      mem->save();
      // Router::singleton()->obj_set("memory",mem);
    }
  }
  void Typer::save_type(const ID &type_id, const Obj_p &type_def) const {
   /* bool allow = nullptr == this->filters || this->filters->empty();
    if(!allow) {
      for(const fURI &furi: *this->filters) {
        if(type_id.matches(furi)) {
          allow = true;
          break;
        }
      }
    }*/
    if(true) {
      try {
        const Obj_p current = ROUTER_READ(type_id);
        ROUTER_WRITE(type_id, type_def, true);
        if(type_progress_bar_) {
          type_progress_bar_->incr_count(type_id.toString());
        } else {
          if(current->is_noobj()) {
            LOG_WRITE(INFO, this,
                      L("!b{} !y{} type!! defined\n", type_id.toString(), OTypes.to_chars(type_def->otype)));
          } else {
            LOG_WRITE(WARN, this,
                      L("!b{} !y{} type!! !b!-{}!! overwritten\n", type_id.toString(), OTypes.to_chars(type_def->otype),
                        current->toString()));
          }
        }
      } catch(const fError &e) {
        LOG_WRITE(ERROR, this, L("{}\n", e.what()));
      }
    } else if(type_progress_bar_) {
      type_progress_bar_->incr_dropped_count(type_id.toString());
    }
    if(type_progress_bar_ && type_progress_bar_->done())
      ROUTER_WRITE(*this->vid, const_pointer_cast<Obj>(shared_from_this()), true);
  }
} // namespace fhatos
