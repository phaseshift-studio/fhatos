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

#include <fhatos.hpp>
#include <language/obj.hpp>

#include "model/log.hpp"

#include FOS_MQTT(mqtt.hpp)

#define TOTAL_INSTRUCTIONS 75

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
                                                                                             }, {x(0, ___)}, INST_FURI,
                                                                                             id_p(type_id.extend("inst/").extend(StringHelper::cxx_f_metadata(__FILE__,__LINE__))))},
                                                                               {vri(":start_progress_bar"),
                                                                                Obj::to_inst([this](const Int_p &, const InstArgs &args) {
                                                                                  this->start_progress_bar(args.at(0)->int_value());
                                                                                  return _noobj_;
                                                                                }, {x(0, ___)}, INST_FURI, make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__,__LINE__)))},
                                                                               {vri(":end_progress_bar"),
                                                                                Obj::to_inst([this](const Str_p &, const InstArgs &args) {
                                                                                  this->end_progress_bar(args.at(0)->str_value());
                                                                                  return _noobj_;
                                                                                }, {x(0, ___)}, INST_FURI, make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__,__LINE__)))},*/
                                                                  })),
                                                                OType::REC,
                                                                id_p(type_id),
                                                                id_p(value_id)) {
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_SAVER = [this](const ID_p &type_id, const Obj_p &type_def) {
        try {
          const Obj_p current = ROUTER_READ(type_id);
          if(type_progress_bar_) {
            ROUTER_WRITE(type_id, type_def,RETAIN);
            type_progress_bar_->incr_count(type_id->toString());
            if(type_progress_bar_->done())
              ROUTER_WRITE(this->vid(), const_pointer_cast<Obj>(shared_from_this()),RETAIN);
          } else {
            ROUTER_WRITE(type_id, type_def,RETAIN);
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
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      IS_TYPE_OF = [this](const ID_p &is_type_id, const ID_p &type_of_id, List<ID_p> *derivations) {
        return this->is_type_of(is_type_id, type_of_id, derivations);
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_CHECKER = [this](const Obj *obj, const ID_p &type_id, const bool throw_on_fail) -> bool {
        if(type_id->equals(*OBJ_FURI) || type_id->equals(*NOOBJ_FURI)) // TODO: hack on noobj
          return true;
        // if the type has already been associated with the object, then it's already been type checked TODO: is this true?
        if(obj->tid()->equals(*type_id))
          return true;
        // don't type check code yet -- this needs to be thought through more carefully as to the definition of code equivalence
        if(obj->o_type() == OType::TYPE || obj->o_type() == OType::INST || obj->o_type() == OType::BCODE)
          return true;
        if(type_id->equals(*NOOBJ_FURI) && (obj->o_type() == OType::NOOBJ || obj->tid()->equals(*OBJ_FURI)))
          return true;
        // if the type is a base type and the base types match, then type check passes
        if(type_id->equals(*OTYPE_FURI.at(obj->o_type())))
          return true;
        // get the type definition and match it to the obj
        if(const Obj_p type = ROUTER_READ(type_id); !type->is_noobj()) {
         // if(type->is_type() && !obj->apply(type)->is_noobj())
         //   return true;
          if(obj->match(type, false))
            return true;
          if(throw_on_fail) {
            static const auto p = GLOBAL_PRINTERS.at(obj->o_type())->clone();
            p->show_type = false;
            throw fError("!g[!b%s!g]!! %s is !rnot!! a !b%s!! as defined by %s", this->vid()->toString().c_str(),
                         obj->toString(p.get()).c_str(), type_id->toString().c_str(), type->toString().c_str());
          }
          return false;
        }
        if(throw_on_fail)
          throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", this->vid()->toString().c_str(),
                       type_id->toString().c_str());
        return false;
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_MAKER = [this](const Obj_p &obj, const ID_p &type_id) -> Obj_p {
        if(obj->otype_ == OType::TYPE)
          return obj;
        const ID_p resolved_type_id = id_p(*ROUTER_RESOLVE(fURI(*type_id)));
        const Obj_p type_def = ROUTER_READ(type_id);
        if(type_def->is_noobj()) {
          throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", this->vid()->toString().c_str(),
                       type_id->toString().c_str());
        }
        // TODO: require all type_defs be bytecode to avoid issue with type constant mapping ??
        const Obj_p proto_obj = type_id->equals(*OTYPE_FURI.at(obj->o_type())) || (
                                  !type_def->is_bcode() && !type_def->is_inst())
                                  ? obj
                                  : type_def->apply(obj);
        if(proto_obj->is_noobj() && !resolved_type_id->equals(*NOOBJ_FURI) && !resolved_type_id->equals(*OBJ_FURI))
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!",
                       Typer::singleton()->vid()->toString().c_str(),
                       obj->toString().c_str(),
                       resolved_type_id->toString().c_str());
        return Obj::create(proto_obj->value_, obj->o_type(), resolved_type_id, obj->vid());
      };
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_INST_RESOLVER = [](const Obj_p &lhs, const Inst_p &inst) -> Inst_p {
        Log::LOGGER(DEBUG, Typer::singleton().get(), " !yresolving!! !yinst!! %s [!gSTART!!]\n",
                    inst->toString().c_str());
        if(inst->is_noobj())
          return inst;
        const static auto TEMP = [](const Obj_p &lhs, const Inst_p &inst, List<ID> *derivation_tree) {
          Obj_p current_obj = lhs;
          const ID_p inst_type_id = id_p(*ROUTER_RESOLVE(fURI(*inst->tid())));
          while(true) {
            // check for inst on obj value
            Inst_p maybe;
            /////////////////////////////// INST VIA VALUE ///////////////////////////////
            if(current_obj->vid()) {
              Log::LOGGER(DEBUG, Typer::singleton().get(), "!m==>!!searching for !yinst!! !b%s!!\n",
                          inst_type_id->toString().c_str());
              if(derivation_tree)
                derivation_tree->emplace_back(current_obj->vid()->extend(C_INST_C).extend(*inst_type_id));
              maybe = ROUTER_READ(id_p(current_obj->vid()->extend(C_INST_C).extend(*inst_type_id)));
              if(!maybe->is_noobj())
                return maybe;
            }
            /////////////////////////////// INST VIA TYPE ///////////////////////////////
            // check for inst on obj type (if not, walk up the obj type tree till root)
            Log::LOGGER(DEBUG, Typer::singleton().get(), "!m==>!!searching for !yinst!! !b%s!!\n",
                        inst_type_id->toString().c_str());
            if(derivation_tree)
              derivation_tree->emplace_back(current_obj->tid()->equals(*OBJ_FURI)
                                              ? fURI(*inst_type_id)
                                              : current_obj->tid()->extend(C_INST_C).extend(*inst_type_id));
            maybe = ROUTER_READ(id_p(current_obj->tid()->equals(*OBJ_FURI)
                                       ? fURI(*inst_type_id) // drop back to flat namespace
                                       : current_obj->tid()->extend(C_INST_C).extend(*inst_type_id)));
            if(!maybe->is_noobj())
              return maybe;
            /////////////////////////////////////////////////////////////////////////////
            if(current_obj->tid()->equals(*(current_obj = ROUTER_READ(current_obj->tid()))->tid()))
              // infinite loop (i.e. base type)
              return noobj();
          }
        };
        Inst_p final_inst;
        if(inst->inst_f() == nullptr) {
          // inst is a token placeholder from a parse or dynamic generation (dynamic dispatch required)
          List<ID> derivation_tree;
          final_inst = TEMP(lhs, inst, &derivation_tree);
          if(final_inst->is_noobj()) {
            const Obj_p type_obj = ROUTER_READ(lhs->tid());
            derivation_tree.push_back(*final_inst->tid());
            final_inst = TEMP(lhs->as(lhs->type()->domain()), inst, &derivation_tree);
            if(final_inst->is_noobj()) {
              //////////////////// print derivation tree in the error message ////////////////////
              string error_message;
              int counter = 0;
              for(const auto &id: derivation_tree) {
                counter = inst->tid()->equals(id) ? 1 : counter + 1;
                error_message.append(StringHelper::format("\n\t!m%s>!!" FURI_WRAP,
                                                          StringHelper::repeat(counter, "--").c_str(),
                                                          id.toString().c_str()));
              }
              throw fError(FURI_WRAP_C(m) " " FURI_WRAP " !yno inst!! resolution %s", lhs->tid()->toString().c_str(),
                           inst->tid()->toString().c_str(), error_message.c_str());
              ////////////////////////////////////////////////////////////////////////////////
            }
          }
          if(final_inst->is_inst()) {
            LOG(TRACE, "merging resolved inst into provide inst\n\t\t%s => %s [!m&s!!]\n",
                final_inst->toString().c_str(),
                inst->toString().c_str(),
                ITypeDescriptions.to_chars(final_inst->itype()).c_str());
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
              final_inst->itype(),
              final_inst->inst_seed_supplier(),
              final_inst->tid(),
              final_inst->vid());
            /// TODO ^--- inst->vid());
          } else {
            final_inst = Obj::to_inst(
              inst->inst_op(),
              inst->inst_args(),
              make_shared<InstF>(make_shared<Cpp>(
                [x = final_inst->clone()](const Obj_p &lhs, const InstArgs &args) -> Obj_p {
                  return x->apply(lhs, args);
                })),
              inst->itype(),
              inst->inst_seed_supplier(),
              inst->tid(), inst->vid());
          }
        } else {
          final_inst = inst;
        }
        LOG_OBJ(DEBUG, lhs, " !gresolved!! !yinst!! %s [!gEND!!]\n", final_inst->toString().c_str());
        return final_inst;
      };
    }

  public:
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
      TYPE_SAVER(type_id, type_def);
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
