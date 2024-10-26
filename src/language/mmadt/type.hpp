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
#include <process/obj_process.hpp>
#include <structure/router.hpp>
#include FOS_MQTT(mqtt.hpp)

#define TOTAL_INSTRUCTIONS 75

using namespace fhatos;
namespace mmadt {
  class Type final : public Heap {
    /// mmadt:/rec/thread
    //  mmadt:/str/
  public:
    ptr<ProgressBar> progress_bar_ = nullptr;

  protected:
    explicit Type(const Pattern &pattern = FOS_TYPE_PREFIX) : Heap(pattern) {
    }

    static ID_p inst_id(const string &opcode) { return id_p(INST_FURI->resolve(opcode)); }

  public:
    static ptr<Type> singleton(const Pattern &pattern = FOS_TYPE_PREFIX) {
      static auto types_p = ptr<Type>(new Type(pattern));
      return types_p;
    }

    void setup() override {
    Heap::setup();
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_CHECKER = [](const Obj &obj, const fURI_p &type_id) -> void {
        //const OType ztype = OTypes.to_enum(string(type_id->path(FOS_BASE_TYPE_INDEX)));
        const fURI_p resolved_type_id = resolve_sugar_type(obj.type(), type_id);
        singleton()->check_type(obj, resolved_type_id, true);
      };
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_MAKER = [this](const Obj_p &obj, const ID_p &type_id) -> Obj_p {
        const ID_p resolved_type_id = resolve_sugar_type(obj->type(), type_id);
        if (OTypes.to_enum(resolved_type_id->path(FOS_BASE_TYPE_INDEX)) != obj->o_type())
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!", this->pattern()->toString().c_str(), obj->toString().c_str(),
                       resolved_type_id->toString().c_str());
        const Obj_p type_def = router()->read(resolved_type_id);
        // TODO: require all type_defs be bytecode to avoid issue with type constant mapping
        const Obj_p proto_obj = is_base_type(resolved_type_id) || (!type_def->is_bcode() && !type_def->is_inst()) ? obj : type_def->apply(obj);
        if ((proto_obj->is_noobj() && !resolved_type_id->equals(*NOOBJ_FURI)))
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!", this->pattern()->toString().c_str(), obj->toString().c_str(),
                       resolved_type_id->toString().c_str());
        return make_shared<Obj>(proto_obj->_value, resolved_type_id, obj->id());
      };
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      const Str_p ARG_ERROR = str("wrong number of arguments");
      // this->saveType(id_p(fURI(FOS_TYPE_PREFIX).extend("uri/url")), bcode());
      this->progress_bar_ = ProgressBar::start(Options::singleton()->printer<Ansi<>>().get(), TOTAL_INSTRUCTIONS);
      this->save_type(inst_id("a"), Insts::a(x(0)));
      this->save_type(inst_id("optional"), Insts::optional(x(0)), false);
      //this->save_type(inst_id("??"), Insts::from(vri(inst_id("optional"))));
      this->save_type(inst_id("inspect"), Insts::inspect());
      this->save_type(inst_id("plus"), Insts::plus(x(0)));
      this->save_type(inst_id("mult"), Insts::mult(x(0)));
      this->save_type(inst_id("div"), Insts::div(x(0)));
      this->save_type(inst_id("mod"), Insts::mod(x(0)));
      this->save_type(inst_id("eq"), Insts::eq(x(0)));
      this->save_type(inst_id("neq"), Insts::neq(x(0)));
      this->save_type(inst_id("gte"), Insts::gte(x(0)));
      this->save_type(inst_id("lte"), Insts::lte(x(0)));
      this->save_type(inst_id("lt"), Insts::lt(x(0)));
      this->save_type(inst_id("gt"), Insts::gt(x(0)));
      this->save_type(inst_id("to"), Insts::to(x(0), x(1, dool(true))));
      this->save_type(inst_id("to_inv"), Insts::to_inv(x(0), x(1, dool(true))));
      //this->save_type(inst_id("->"), Insts::from(vri(inst_id("to_inv"))));
      this->save_type(inst_id("via_inv"), Insts::to_inv(x(0), dool(false)));
      //this->save_type(inst_id("-->"), Insts::from(vri(inst_id("via_inv"))));
      this->save_type(inst_id("start"), Insts::start(x(0)));
      this->save_type(inst_id("merge"), Insts::merge(x(0)));
      //this->save_type(inst_id(">-"), Insts::from(vri(inst_id("merge"))));
      this->save_type(inst_id("map"), Insts::map(x(0)));
      this->save_type(inst_id("filter"), Insts::filter(x(0)));
      this->save_type(inst_id("count"), Insts::count());
      this->save_type(inst_id("subset"), Insts::subset(x(0), x(1)));
      this->save_type(inst_id("sum"), Insts::sum());
      this->save_type(inst_id("prod"), Insts::prod());
      this->save_type(inst_id("group"), Insts::group(x(0, bcode()), x(1, bcode()), x(2, bcode())));
      this->save_type(inst_id("get"), Insts::get(x(0)));
      this->save_type(inst_id("set"), Insts::set(x(0), x(1)));
      this->save_type(inst_id("noop"), Insts::noop());
      this->save_type(inst_id("as"), Insts::as(x(0)));
      this->save_type(inst_id("by"), Insts::by(x(0)));
      this->save_type(inst_id("type"), Insts::type());
      this->save_type(inst_id("is"), Insts::is(x(0)));
      this->save_type(inst_id("from"), Insts::from(x(0, Insts::error(ARG_ERROR)), x(1)));
      //this->save_type(inst_id("*"), Insts::from(x(0, Insts::error(ARG_ERROR)), x(1)));
      this->save_type(inst_id("at"), Insts::at(x(0, Insts::error(ARG_ERROR)), x(1)));
      //this->save_type(inst_id("@"), Insts::at(x(0, Insts::error(ARG_ERROR)), x(1)));
      this->save_type(inst_id("within"), Insts::within(x(0)));
      this->save_type(inst_id("print"), Insts::print(x(0, bcode())));
      // this->save_type(inst_id("switch"), Insts::bswitch(x(0)));
      this->save_type(inst_id("explain"), Insts::explain());
      this->save_type(inst_id("drop"), Insts::drop(x(0)));
      //this->save_type(inst_id("V"), Insts::from(vri(inst_id("drop"))));
      this->save_type(inst_id("lift"), Insts::lift(x(0)));
      //this->save_type(inst_id("^"), Insts::from(vri(inst_id("lift"))));
      this->save_type(inst_id("size"), Insts::size());
      this->save_type(inst_id("foldr"), Insts::foldr(x(0)));
      this->save_type(inst_id("barrier"), Insts::barrier(x(0)));
      this->save_type(inst_id("block"), Insts::block(x(0)));
      //this->save_type(inst_id("|"), Insts::from(vri(inst_id("block"))));
      this->save_type(inst_id("cleave"), Insts::cleave(x(0)));
      this->save_type(inst_id("split"), Insts::split(x(0)));
      //this->save_type(inst_id("-<"), Insts::from(vri(inst_id("split"))));
      this->save_type(inst_id("each"), Insts::each(x(0)));
      //this->save_type(inst_id("="), Insts::from(vri(inst_id("each"))));
      this->save_type(inst_id("window"), Insts::window(x(0)));
      this->save_type(inst_id("match"), Insts::match(x(0)));
      //this->save_type(inst_id("~"), Insts::from(vri(inst_id("match"))));
      this->save_type(inst_id("end"), Insts::end());
      this->save_type(inst_id("until"), Insts::until(x(0)));
      this->save_type(inst_id("dedup"), Insts::dedup(x(0, bcode())));
      this->save_type(inst_id("insert"), Insts::insert(x(0)));
      this->save_type(inst_id("and"), Insts::x_and(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3)));
      this->save_type(inst_id("or"), Insts::x_or(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3)));
      this->save_type(inst_id("rand"), Insts::rand(x(0, vri(BOOL_FURI))));
      this->save_type(inst_id("error"), Insts::error(x(0, str("an error occurred"))));
      this->save_type(inst_id("repeat"), Insts::repeat(x(0), x(1, bcode()), x(2)));
      this->progress_bar_->end("!bmm-adt !yinstruction set!! loaded\n");
      this->progress_bar_ = nullptr;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    void save_type(const ID_p &type_id, const Obj_p &type_def, const bool via_pub = false) const {
      try {
        if (!via_pub) {
          const Obj_p current = router()->read(type_id);
          if (current != type_def) {
            if (!current->is_noobj() && !progress_bar_)
              LOG_STRUCTURE(WARN, this, "!b%s!g[!!%s!g] !ytype!! overwritten\n", type_id->toString().c_str(),
                        current->toString().c_str());
            router()->write(type_id, type_def, RETAIN_MESSAGE);
          }
        }
        if (!this->progress_bar_)
          LOG_STRUCTURE(INFO, this, "!b%s!g[!!%s!g] !ytype!! defined\n", type_id->toString().c_str(),
                    type_def->toString().c_str());
        else {
          this->progress_bar_->incr_count(type_id->toString());
        }
      } catch (const fError &e) {
        LOG_STRUCTURE(ERROR, this, "unable to save type !b%s!!: %s\n", type_id->toString().c_str(), e.what());
      }
    }

    bool type_exists(const ID_p &type_id, const Obj_p &type_def) const {
      const Obj_p existing_type_def = router()->read(type_id);
      return !existing_type_def->is_noobj() && (*existing_type_def == *type_def);
    }

    static ID_p resolve_sugar_type(const fURI_p &type, const fURI_p &furi) {
      return OTypes.has_enum(furi->toString())
               ? id_p(ID(string(FOS_TYPE_PREFIX) + furi->name()))
               : id_p(type->resolve(*furi));
    }

    static bool is_base_type(const ID_p &type_id) { return type_id->path_length() == FOS_BASE_TYPE_INDEX + 1; }

    bool check_type(const Obj &obj, const fURI_p &type_id, const bool do_throw = true) const
      noexcept(false) {
      const OType type_otype = OTypes.to_enum(string(type_id->path(FOS_BASE_TYPE_INDEX)));
      if (obj.o_type() == OType::INST || obj.o_type() == OType::BCODE || type_otype == OType::INST || type_otype ==
          OType::BCODE)
        return true;
      if (obj.o_type() != type_otype) {
        if (do_throw)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!", this->id()->toString().c_str(), obj.toString(false).c_str(),
                       type_id->toString().c_str());
        return false;
      }
      if (type_id->path_length() == (FOS_BASE_TYPE_INDEX + 1)) {
        // base type (otype)
        return true;
      }
      const Obj_p type = router()->read(type_id);
      if (!type->is_noobj()) {
        if (obj.match(type, false)) {
          return true;
        }
        if (do_throw)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!g[!!%s!g]!!", this->id()->toString().c_str(),
                       obj.toString(false).c_str(), type_id->toString().c_str(), type->toString().c_str());
        return false;
      }
      if (do_throw)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", this->id()->toString().c_str(),
                     type_id->toString().c_str());
      return false;
    }
    ////////////////////////////////////////////
    ////////////////////////////////////////////
    ////////////////////////////////////////////
    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
       this->check_type(*obj,id,true);
       Heap::write_raw_pairs(id,obj,retain);
     }
  };
} // namespace fhatos
#endif
