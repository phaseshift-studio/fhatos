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
#include "obj.hpp"
#include "../model/log.hpp"
#include "../process/process.hpp"
#include "mmadt/compiler.hpp"

namespace fhatos {
  using std::const_pointer_cast;
  //TODO: MAKE THIS THREAD_LOCAL
  thread_local ptr<ProgressBar> type_progress_bar_;

  class Typer final : public Obj {
  protected:
    explicit Typer(const ID &value_id, const ID &type_id) : Obj(share(RecMap<>(
                                                                  {
                                                                    /*{vri(":check"),
                                                                                Obj::to_inst([this](const Obj_p &lhs, const InstArgs &args) {
                                                                                               return dool(this->check_type(
                                                                                                   args.at(0).get(),
                                                                                                   furi_p(lhs->lst_value()->at(1)->uri_value())));
                                                                                             }, {x(0, Obj::to_bcode())}, INST_FURI,
                                                                                             id_p(type_id.extend("inst/").extend(StringHelper::cxx_f_metadata(__FILE__,__LINE__))))},
                                                                               {vri(":start_progress_bar"),
                                                                                Obj::to_inst([this](const Int_p &, const InstArgs &args) {
                                                                                  this->start_progress_bar(args.at(0)->int_value());
                                                                                  return _noobj_;
                                                                                }, {x(0, Obj::to_bcode())}, INST_FURI, make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__,__LINE__)))},
                                                                               {vri(":end_progress_bar"),
                                                                                Obj::to_inst([this](const Str_p &, const InstArgs &args) {
                                                                                  this->end_progress_bar(args.at(0)->str_value());
                                                                                  return _noobj_;
                                                                                }, {x(0, Obj::to_bcode())}, INST_FURI, make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__,__LINE__)))},*/
                                                                  })),
                                                                OType::REC,
                                                                id_p(type_id),
                                                                id_p(value_id)) {
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_SAVER = [this](const ID_p &type_id, const Obj_p &type_def) {
        Typer::singleton()->save_type(type_id, type_def);
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      IS_TYPE_OF = [this](const ID_p &is_type_id, const ID_p &type_of_id, List<ID_p> *derivations) {
        return this->is_type_of(is_type_id, type_of_id, derivations);
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_CHECKER = [this](const Obj *obj, const ID_p &typex_id, const bool throw_on_fail) -> bool {
        // TODO: need to get coefficient data in dom/rng ids
        if(obj->is_noobj()) {
          if(const vector<string> coef = typex_id->query_values(FOS_RNG_COEF);
            !coef.empty() && stoi(coef.front()) == 0) {
            //LOG(INFO,"HERE: %s\n", type_id->toString().c_str());
            return true;
          }
        }
        const ID_p type_id = id_p(typex_id->no_query());
        if(type_id->equals(*OBJ_FURI) || type_id->equals(*NOOBJ_FURI)) // TODO: hack on noobj
          return true;
        // if the type is a base type and the base types match, then type check passes
        if(type_id->equals(*OTYPE_FURI.at(obj->o_type())))
          return true;
        // if the type has already been associated with the object, then it's already been type checked TODO: is this true?
        //if(obj->tid_->equals(*type_id))
        //  return true;
        // don't type check code yet -- this needs to be thought through more carefully as to the definition of code equivalence
        if(obj->o_type() == OType::TYPE || obj->o_type() == OType::INST || obj->o_type() == OType::BCODE)
          return true;
        if(type_id->equals(*NOOBJ_FURI) && (obj->o_type() == OType::NOOBJ || obj->tid_->equals(*OBJ_FURI)))
          return true;
        // get the type definition and match it to the obj
        if(const Obj_p type = Router::singleton()->read(type_id); !type->is_noobj()) {
          ObjHelper::check_coefficients(obj->range_coefficient(), type->domain_coefficient());
          if(type->is_code())
             return !obj->apply(type)->is_noobj();
          if(obj->match(type, false))
            return true;
          if(throw_on_fail) {
            static const auto p = GLOBAL_PRINTERS.at(obj->o_type())->clone();
            p->show_type = false;
            throw fError("!g[!b%s!g]!! %s is !rnot!! a !b%s!! as defined by %s", this->vid_->toString().c_str(),
                         obj->toString(p.get()).c_str(), type_id->toString().c_str(), type->toString().c_str());
          }
          return false;
        }
        if(throw_on_fail)
          throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", this->vid_->toString().c_str(),
                       type_id->toString().c_str());
        return false;
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_INST_RESOLVER = [](const Obj_p &lhs, const Inst_p &inst) -> Inst_p {
        using DerivationTree = List<Trip<ID_p, ID_p, Obj_p>>;
        Log::LOGGER(DEBUG, Typer::singleton().get(), " !yresolving!! !yinst!! %s [!gSTART!!]\n",
                    inst->toString().c_str());
        Compiler compiler = Compiler(true, false);
        if(inst->is_noobj())
          return inst;
        if(!lhs->is_noobj())
          compiler.coefficient_check(lhs->range_coefficient(), inst->domain_coefficient());
        const static auto TEMP = [](const Obj_p &lhs, const Inst_p &inst, DerivationTree *dt) {
          Obj_p current_obj = lhs;
          const ID_p inst_type_id = id_p(Router::singleton()->resolve(*inst->tid_));
          while(true) {
            Inst_p maybe;
            /////////////////////////////// INST VIA ID RESOLVE ///////////////////////////////
            /////////////////////////////// INST VIA VALUE ///////////////////////////////
            if(current_obj->vid_) {
              Log::LOGGER(DEBUG, Typer::singleton().get(), "!m==>!!searching for !yinst!! !b%s!!\n",
                          inst_type_id->toString().c_str());
              const ID_p next_inst_type_id =
                  id_p(current_obj->vid_->add_component(*inst_type_id));
              maybe = Router::singleton()->read(next_inst_type_id);
              if(dt)
                dt->emplace_back(current_obj->vid_, next_inst_type_id, maybe);
              if(!maybe->is_noobj() && maybe->is_inst() && maybe->inst_f())
                return maybe;
            }
            /////////////////////////////// INST VIA TYPE ///////////////////////////////
            // check for inst on obj type (if not, walk up the obj type tree till root)
            Log::LOGGER(DEBUG, Typer::singleton().get(), "!m==>!!searching for !yinst!! !b%s!!\n",
                        inst_type_id->toString().c_str());
            const ID_p next_inst_type_id = id_p(current_obj->tid_->no_query().equals(*OBJ_FURI)
                                                  ? fURI(*inst_type_id) // drop back to flat namespace
                                                  : current_obj->tid_->no_query().add_component(*inst_type_id));
            maybe = Router::singleton()->read(next_inst_type_id);
            if(dt)
              dt->emplace_back(id_p(current_obj->tid_->no_query()), next_inst_type_id, maybe);
            if(!maybe->is_noobj() && maybe->is_inst() && maybe->inst_f())
              return maybe;
            /////////////////////////////////////////////////////////////////////////////
            if(current_obj->tid_->no_query().equals(
              (current_obj = Router::singleton()->read(id_p(current_obj->tid_->no_query())))->tid_->no_query())) {
              // infinite loop (i.e. base type)
              return noobj();
            }
          }
        };
        //////////////////////////////////////
        //////////////////////////////////////
        /////////////////////////////////////
        DerivationTree *dt = nullptr; //make_unique<DerivationTree>();
        if(dt)
          dt->push_back({id_p(""), id_p(""), Obj::to_noobj()});
        ID_p inst_type_id = id_p(Router::singleton()->resolve(*inst->tid_));
        Inst_p final_inst = Router::singleton()->read(inst_type_id);
        if(dt)
          dt->emplace_back(id_p(""), inst_type_id, final_inst);
        if(final_inst->is_noobj() || !final_inst->is_inst() || !final_inst->inst_f()) {
          if(dt)
            dt->push_back({id_p(""), id_p(""), Obj::to_noobj()});
          final_inst = TEMP(lhs, inst, dt);
          if(final_inst->is_noobj() || !final_inst->is_inst() || !final_inst->inst_f()) {
            const Obj_p next_lhs = Router::singleton()->read(lhs->tid_);
            const ID_p next_id = id_p(next_lhs->range()->no_query());
            const Obj_p next_obj = Router::singleton()->read(next_id);
            if(dt)
              dt->emplace_back(id_p(""), id_p(""), Obj::to_noobj());
            final_inst = TEMP(next_obj, inst, dt);
            if(final_inst->is_noobj() || !final_inst->is_inst() || !final_inst->inst_f()) {
              if(inst->inst_f())
                final_inst = inst;
            }
            //////////////////// generated printable derivation tree ////////////////////
            string derivation_string;
            if(final_inst->is_noobj() || !final_inst->is_inst() || !final_inst->inst_f()) {
              if(dt) {
                int counter = 0;
                for(const auto &oir: *dt) {
                  counter = std::get<1>(oir)->empty() ? 0 : counter + 1;
                  if(counter != 0) {
                    string indent = StringHelper::repeat(counter, "-").append("!g>!!");
                    derivation_string.append(StringHelper::format(
                      "\n\t!m%-8s!g[!b%-15s!g] !b%-30s!! !m=>!m !b%-35s!!",
                      indent.c_str(),
                      std::get<0>(oir)->toString().c_str(),
                      std::get<1>(oir)->toString().c_str(),
                      std::get<2>(oir)->toString().c_str()));
                  }
                }
              }
              derivation_string = dt ? derivation_string : "<no derivation>";
              throw fError(FURI_WRAP_C(m) " " FURI_WRAP " !yno inst!! resolution %s", lhs->tid_->toString().c_str(),
                           inst->tid_->toString().c_str(), derivation_string.c_str());
            }
          }
          ////////////////////////////////////////////////////////////////////////////////
        }
        if(final_inst->is_inst()) {
          LOG(TRACE, "merging resolved inst into provide inst\n\t\t%s => %s [!m&s!!]\n",
              final_inst->toString().c_str(),
              inst->toString().c_str(),
              "SIGNATURE HERE");
          const auto merged_args = Obj::to_inst_args();
          int counter = 0;
          for(const auto &[k,v]: *final_inst->inst_args()->rec_value()) {
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
          final_inst = Obj::to_inst(
            final_inst->inst_op(),
            merged_args,
            final_inst->inst_f(),
            final_inst->inst_seed_supplier(),
            final_inst->tid_,
            final_inst->vid_);
          /// TODO ^--- inst->vid_);
        } else {
          final_inst = Obj::to_inst(
            inst->inst_op(),
            inst->inst_args(),
            make_shared<InstF>(make_shared<Cpp>(
              [x = final_inst->clone()](const Obj_p &lhs, const InstArgs &args) -> Obj_p {
                return x->apply(lhs, args);
              })),
            inst->inst_seed_supplier(),
            inst->tid_, inst->vid_);
        }
        LOG_OBJ(DEBUG, lhs, " !gresolved!! !yinst!! %s [!gEND!!]\n", final_inst->toString().c_str());
        return final_inst;
      };
    }

  public
  :
    static ptr<Typer> singleton(const ID &id = FOS_SCHEME "/type") {
      static auto types_p = ptr<Typer>(new Typer(id, *REC_FURI));
      return types_p;
    }

    void start_progress_bar(const uint16_t size) {
      type_progress_bar_ = ProgressBar::start(Options::singleton()->printer<Ansi<>>().get(), size);
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
    void save_type(const ID_p &type_id, const Obj_p &type_def) const {
      try {
        const Obj_p current = Router::singleton()->read(type_id);
        if(type_progress_bar_) {
          Router::singleton()->write(type_id, type_def,RETAIN);
          type_progress_bar_->incr_count(type_id->toString());
          if(type_progress_bar_->done())
            Router::singleton()->write(this->vid_, const_pointer_cast<Obj>(shared_from_this()),RETAIN);
        } else {
          Router::singleton()->write(type_id, type_def,RETAIN);
          if(current->is_noobj()) {
            LOG_OBJ(INFO, this, FURI_WRAP " !ytype!! defined\n",
                    type_id->toString().c_str(),
                    type_id->toString().c_str());
          } else {
            LOG_OBJ(INFO, this, "!b%s !ytype!! !b!-%s!! overwritten\n",
                    type_id->toString().c_str(), current->toString().c_str());
          }
        }
      } catch(const fError &e) {
        LOG_EXCEPTION(this, e);
      }
    }

    bool is_type_of(const ID_p &is_type_id, const ID_p &of_type_id, List<ID_p> *derivation_tree = nullptr) const {
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
