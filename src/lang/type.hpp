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
#ifndef fhatos_types_hpp
#define fhatos_types_hpp

#include "../fhatos.hpp"
#include "../model/fos/util/log.hpp"
#include "mmadt/compiler.hpp"
#include "obj.hpp"

#define FOS_URI "/fos"

namespace fhatos {
  using std::const_pointer_cast;
  inline thread_local ptr<ProgressBar> type_progress_bar_;
  static const ID_p Q_PROC_FURI = id_p(FOS_URI "/q");
  static const ID_p MESSAGE_FURI = id_p(FOS_URI "/q/sub/msg");
  static const ID_p SUBSCRIPTION_FURI = id_p(FOS_URI "/q/sub/sub");


  class Typer final : public Obj {
  protected:
    std::vector<fURI>* filters = nullptr;


    explicit Typer(const ID &value_id, const ID &type_id) :
        Obj(std::make_shared<RecMap<>>(), OType::REC, id_p(type_id), id_p(value_id)) {
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_SAVER = [this](const ID &type_id, const Obj_p &type_def) {
        this->save_type(type_id, type_def);
      /*  try {
          const Obj_p current = ROUTER_READ(type_id);
          if(type_progress_bar_) {
            ROUTER_WRITE(type_id, type_def, true);
            type_progress_bar_->incr_count(type_id.toString());
            if(type_progress_bar_->done())
              ROUTER_WRITE(*this->vid, const_pointer_cast<Obj>(shared_from_this()), true);
          } else {
            ROUTER_WRITE(type_id, type_def, true);
            if(current->is_noobj()) {
              LOG_WRITE(INFO, this, L("!b{} !ytype!! defined\n", type_id.toString()));
            } else {
              LOG_WRITE(INFO, this, L("!b{} !ytype!! !b!-{}!! overwritten\n", type_id.toString(), current->toString()));
            }
          }
        } catch(const fError &e) {
          LOG_WRITE(ERROR, this, L("{}", e.what()));
        }*/
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      IS_TYPE_OF = [this](const ID_p &is_type_id, const ID_p &type_of_id, List<ID_p> *derivations) {
        return this->is_type_of(is_type_id, type_of_id, derivations);
      };
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
              InstF(make_shared<Cpp>([x = final_inst->clone()](const Obj_p &lhs, const InstArgs &args) -> Obj_p {
                return x->apply(lhs);
              })),
              inst->inst_seed_supplier(), inst->tid, inst->vid);
        }
        LOG_WRITE(DEBUG, lhs.get(), L(" !gresolved!! !yinst!! {} [!gEND!!]\n", final_inst->toString()));
        return final_inst;
      };
    }

  public:
    static ptr<Typer> &singleton(const ID &id = "/sys/type") {
      static auto types_p = ptr<Typer>(new Typer(id, *REC_FURI));
      Compiler::boot_loading = false;
      return types_p;
    }

    void set_filters(std::vector<fURI>*filters) {
      this->filters = filters;
    }

    void clear_filters() {
      this->filters = nullptr;
    }

    void start_progress_bar(const uint16_t size) {
      type_progress_bar_ = ProgressBar::start(Ansi<>::singleton().get(), size);
    }

    void end_progress_bar(const string &message) {
      if(type_progress_bar_) {
        type_progress_bar_->end(message);
        type_progress_bar_ = nullptr;
      }
    }

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void save_type(const ID &type_id, const Obj_p &type_def) const {
      bool allow = nullptr == this->filters || this->filters->empty();
      if(!allow) {
        for(const fURI &furi: *this->filters) {
          if(type_id.matches(furi)) {
            allow = true;
            break;
          }
        }
      }
      if(allow) {
        try {
          const Obj_p current = ROUTER_READ(type_id);
          if(type_progress_bar_) {
            ROUTER_WRITE(type_id, type_def, true);
            type_progress_bar_->incr_count(type_id.toString());
          } else {
            ROUTER_WRITE(type_id, type_def, true);
            if(current->is_noobj()) {
              LOG_WRITE(INFO, this, L("!b{} !ytype!! defined\n", type_id.toString()));
            } else {
              LOG_WRITE(INFO, this, L("!b{} !ytype!! !b!-{}!! overwritten\n", type_id.toString(), current->toString()));
            }
          }
        } catch(const fError &e) {
          LOG_WRITE(ERROR, this, L("{}", e.what()));
        }
      } else if(type_progress_bar_) {
        type_progress_bar_->incr_dropped_count(type_id.toString());
      }
      if(type_progress_bar_ && type_progress_bar_->done())
        ROUTER_WRITE(*this->vid, const_pointer_cast<Obj>(shared_from_this()), true);
    }

    bool is_type_of(const ID_p &is_type_id, const ID_p &of_type_id, List<ID_p> *derivation_tree = nullptr) {
      ID_p current_type_id = is_type_id;
      while(true) {
        if(derivation_tree)
          derivation_tree->emplace_back(current_type_id);
        if(current_type_id->equals(*of_type_id))
          return true;
        if(current_type_id->equals(*OBJ_FURI))
          return false;
        if(const Option<fURI> &domain = current_type_id->query_value(FOS_DOMAIN); domain.has_value()) {
          current_type_id = id_p(domain.value());
        } else {
          return false;
        }
      }
    }
  };
} // namespace fhatos
#endif
