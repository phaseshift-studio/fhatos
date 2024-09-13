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
#include <language/insts.hpp>
#include <language/obj.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <structure/router.hpp>
#include <structure/stype/key_value.hpp>

namespace fhatos {
  class Types : public Actor<Coroutine, KeyValue> {
    explicit Types(const ID &id = FOS_TYPE_PREFIX): Actor(id) {
    }

    static ID_p inst_id(const string &opcode) {
      return id_p(INST_FURI->resolve(opcode));
    }

    void load_insts() {
      const Str_p ARG_ERROR = str("wrong number of arguments");
      //this->saveType(id_p(fURI(FOS_TYPE_PREFIX).extend("uri/url")), bcode());
      this->save_type(inst_id("optional"), Insts::optional(x(0)));
      this->save_type(inst_id("inspect"), Insts::inspect());
      this->save_type(inst_id("plus"), Insts::plus(x(0)));
      this->save_type(inst_id("mult"), Insts::mult(x(0)));
      this->save_type(inst_id("mod"), Insts::mod(x(0)));
      this->save_type(inst_id("eq"), Insts::eq(x(0)));
      this->save_type(inst_id("neq"), Insts::neq(x(0)));
      this->save_type(inst_id("gte"), Insts::gte(x(0)));
      this->save_type(inst_id("lte"), Insts::lte(x(0)));
      this->save_type(inst_id("lt"), Insts::lt(x(0)));
      this->save_type(inst_id("gt"), Insts::gt(x(0)));
      this->save_type(inst_id("to"), Insts::to(x(0)));
      this->save_type(inst_id("to_inv"), Insts::to_inv(x(0)));
      this->save_type(inst_id("->"), Insts::from(uri(inst_id("to_inv"))));
      this->save_type(inst_id("start"), Insts::start(x(0)));
      this->save_type(inst_id("merge"), Insts::merge());
      this->save_type(inst_id(">-"), Insts::from(uri(inst_id("merge"))));
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
      this->save_type(inst_id("from"), Insts::from(x(0), x(1)));
      this->save_type(inst_id("*"), Insts::from(x(0), x(1)));
      this->save_type(inst_id("pub"), Insts::pub(x(0), x(1), x(2, dool(true))));
      this->save_type(inst_id("sub"), Insts::sub(x(0), x(1)));
      this->save_type(inst_id("within"), Insts::within(x(0)));
      this->save_type(inst_id("print"), Insts::print(x(0, bcode())));
      this->save_type(inst_id("switch"), Insts::bswitch(x(0)));
      this->save_type(inst_id("explain"), Insts::explain());
      this->save_type(inst_id("drop"), Insts::drop(x(0)));
      this->save_type(inst_id("V"), Insts::from(uri(inst_id("drop"))));
      this->save_type(inst_id("lift"), Insts::lift(x(0)));
      this->save_type(inst_id("^"), Insts::from(uri(inst_id("lift"))));
      this->save_type(inst_id("size"), Insts::size());
      this->save_type(inst_id("foldr"), Insts::foldr(x(0)));
      this->save_type(inst_id("barrier"), Insts::barrier(x(0)));
      this->save_type(inst_id("block"), Insts::block(x(0)));
      this->save_type(inst_id("|"), Insts::from(uri(inst_id("block"))));
      this->save_type(inst_id("cleave"), Insts::cleave(x(0)));
      this->save_type(inst_id("split"), Insts::split(x(0)));
      this->save_type(inst_id("-<"), Insts::from(uri(inst_id("split"))));
      this->save_type(inst_id("each"), Insts::each(x(0)));
      this->save_type(inst_id("="), Insts::from(uri(inst_id("each"))));
      this->save_type(inst_id("window"), Insts::window(x(0)));
      this->save_type(inst_id("match"), Insts::match(x(0)));
      this->save_type(inst_id("~"), Insts::from(uri(inst_id("match"))));
      this->save_type(inst_id("end"), Insts::end());
      this->save_type(inst_id("until"), Insts::until(x(0)));
      this->save_type(inst_id("dedup"), Insts::dedup(x(0, bcode())));
      this->save_type(inst_id("insert"), Insts::insert(x(0)));
      this->save_type(inst_id("and"), Insts::x_and(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3)));
      this->save_type(inst_id("or"), Insts::x_or(x(0, Insts::error(ARG_ERROR)), x(1), x(2), x(3)));
      this->save_type(inst_id("error"), Insts::error(x(0, str("an error occurred"))));
    }

  public:
    static ptr<Types> singleton(const ID &id = FOS_TYPE_PREFIX) {
      static ptr<Types> types_p = ptr<Types>(new Types(id));
      return types_p;
    }

    void setup() override {
      Actor::setup();
      TYPE_CHECKER = [](const Obj &obj, const OType otype, const ID_p &typeId) {
        singleton()->check_type(obj, otype, typeId, true);
        return typeId;
      };
      this->load_insts();
      this->subscribe(*this->pattern(), [](const Message_p &message) {
        if (message->retain && (message->source != *Types::singleton()->id()) &&
            message->target != ID("anon_tgt"))
          Types::singleton()->save_type(id_p(message->target), message->payload, true);
        // else { // transient provides type checking?
        // TYPE_CHECKER(*message->payload, OTypes.toEnum(message->target.toString().c_str()),
        // id_p(message->target));
        //}
      });
    }

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void save_type(const ID_p &typeId, const Obj_p &typeDef, const bool viaPub = false) {
      try {
        if (!viaPub) {
          const Obj_p current = this->read(typeId, this->id());
          if (current != typeDef) {
            if (!current->is_noobj())
              LOG_PROCESS(WARN, this, "!b%s!g[!!%s!g] !ytype!! overwritten\n", typeId->toString().c_str(),
                        current->toString().c_str());
            this->write(typeId, typeDef, this->id(),RETAIN_MESSAGE);
          }
        }
        LOG_PROCESS(INFO, this, "!b%s!g[!!%s!g] !ytype!! defined\n", typeId->toString().c_str(),
                    typeDef->toString().c_str());
      } catch (const fError &e) {
        LOG_PROCESS(ERROR, this, "Unable to save type !b%s!!: %s\n", typeId->toString().c_str(), e.what());
      }
    }

    inline const static auto MMADT_PREFIX = fURI(FOS_MMADT_URL_PREFIX);

    void save_inst_type(const ID_p &inst_id, const Inst_p &inst, const ID_p &source) {
      this->write(inst_id, inst, source,RETAIN_MESSAGE);
      /*this->write(id_p(inst->id()->extend("_kind")), str(ITypeDescriptions.toChars(inst->itype())), source,
                  RETAIN_MESSAGE);
      this->write(id_p(inst->id()->extend("_doc")),
                  uri(MMADT_PREFIX.extend(inst_id->toString().c_str()) //id_p(URI_FURI->extend("url")), source,
          //        RETAIN_MESSAGE);
      //this->saveType(inst->id()->extend("_seed"),inst->inst_seed_supplier())) */
    }

    bool check_type(const Obj &obj, const OType otype, const ID_p &typeId,
                    const bool doThrow = true) noexcept(false) {
      const OType typeOType = OTypes.toEnum(typeId->path(FOS_BASE_TYPE_INDEX));
      if (otype == OType::INST || otype == OType::BCODE || typeOType == OType::INST || typeOType == OType::BCODE)
        return true;
      if (otype != typeOType) {
        if (doThrow)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!\n", Types::singleton()->id()->toString().c_str(),
                       obj.toString().c_str(), typeId->toString().c_str());
        return false;
      }
      if (typeId->path_length() == (FOS_BASE_TYPE_INDEX + 1)) {
        // base type (otype)
        return true;
      }
      const Obj_p type = this->read(typeId, this->id());
      if (!type->is_noobj()) {
        if (obj.match(type, false)) {
          return true;
        }
        if (doThrow)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!g[!!%s!g]!!\n",
                       Types::singleton()->id()->toString().c_str(),
                       obj.toString().c_str(), typeId->toString().c_str(), type->toString().c_str());
        return false;
      }
      if (doThrow)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!\n",
                     Types::singleton()->id()->toString().c_str(),
                     typeId->toString().c_str());
      return false;
    }
  };
} // namespace fhatos
#endif
