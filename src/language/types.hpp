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
#include <unistd.h>
#include <util/mutex_rw.hpp>

namespace fhatos {
  class Types : public Actor<Coroutine, KeyValue> {

    explicit Types(const ID &id = FOS_TYPE_PREFIX) : Actor(id) {}

    static ID_p inst_id(const string &opcode) {
      return id_p(INST_FURI->resolve(opcode));
    }

    void load_insts() {
      this->saveType(inst_id("plus"), Insts::plus(x("_0")));
      this->saveType(inst_id("mult"), Insts::mult(x("_0")));
      this->saveType(inst_id("mod"), Insts::mod(x("_0")));
      this->saveType(inst_id("eq"), Insts::eq(x("_0")));
      this->saveType(inst_id("neq"), Insts::neq(x("_0")));
      this->saveType(inst_id("gte"), Insts::gte(x("_0")));
      this->saveType(inst_id("lte"), Insts::lte(x("_0")));
      this->saveType(inst_id("lt"), Insts::lt(x("_0")));
      this->saveType(inst_id("gt"), Insts::gt(x("_0")));
      this->saveType(inst_id("to"), Insts::to(x("_0")));
      this->saveType(inst_id("to_inv"), Insts::to_inv(x("_0")));
      this->saveType(inst_id("->"), Insts::to_inv(x("_0")));
      this->saveType(inst_id("start"), Insts::start(x("_0")));
      this->saveType(inst_id("__"), Insts::start(x("_0")));
      this->saveType(inst_id("merge"), Insts::merge());
      this->saveType(inst_id(">-"), Insts::merge());
      this->saveType(inst_id("map"), Insts::map(x("_0")));
      this->saveType(inst_id("filter"), Insts::filter(x("_0")));
      this->saveType(inst_id("count"), Insts::count());
      this->saveType(inst_id("subset"), Insts::subset(x("_0"), x("_1")));
      this->saveType(inst_id("sum"), Insts::sum());
      this->saveType(inst_id("prod"), Insts::prod());
      this->saveType(inst_id("group"), Insts::group(x("_0"), x("_1"), x("_2")));
      this->saveType(inst_id("get"), Insts::get(x("_0")));
      this->saveType(inst_id("set"), Insts::set(x("_0"), x("_1")));
      this->saveType(inst_id("noop"), Insts::noop());
      this->saveType(inst_id("as"), Insts::as(x("_0")));
      this->saveType(inst_id("by"), Insts::by(x("_0")));
      this->saveType(inst_id("type"), Insts::type());
      this->saveType(inst_id("is"), Insts::is(x("_0")));
      this->saveType(inst_id("from"), Insts::from(x("_0")));
      this->saveType(inst_id("*"), Insts::from(x("_0")));
      this->saveType(inst_id("pub"), Insts::pub(x("_0"), x("_1")));
      this->saveType(inst_id("sub"), Insts::sub(x("_0"), x("_1")));
      this->saveType(inst_id("within"), Insts::within(x("_0")));
      this->saveType(inst_id("print"), Insts::print(x("_0")));
      this->saveType(inst_id("switch"), Insts::bswitch(x("_0")));
      this->saveType(inst_id("explain"), Insts::explain());
      this->saveType(inst_id("drop"), Insts::drop(x("_0")));
      this->saveType(inst_id("V"), Insts::drop(x("_0")));
      this->saveType(inst_id("lift"), Insts::lift(x("_0")));
      this->saveType(inst_id("^"), Insts::lift(x("_0")));
      this->saveType(inst_id("size"), Insts::size());
      this->saveType(inst_id("foldr"), Insts::foldr(x("_0")));
      this->saveType(inst_id("barrier"), Insts::barrier(x("_0")));
      this->saveType(inst_id("block"), Insts::block(x("_0")));
      this->saveType(inst_id("|"), Insts::block(x("_0")));
      //bcode({Insts::drop(bcode({Insts::from(uri(inst_id("block")))}))}));
      this->saveType(inst_id("cleave"), Insts::cleave(x("_0")));
      this->saveType(inst_id("split"), Insts::split(x("_0")));
      this->saveType(inst_id("each"), Insts::each(x("_0")));
      this->saveType(inst_id("="), Insts::each(x("_0")));
      this->saveType(inst_id("window"), Insts::window(x("_0")));
      this->saveType(inst_id("match"), Insts::match(x("_0")));
      this->saveType(inst_id("~"), Insts::match(x("_0")));
      this->saveType(inst_id("end"), Insts::end());
    }

  public:
    static ptr<Types> singleton(const ID &id = FOS_TYPE_PREFIX) {
      static ptr<Types> types_p = ptr<Types>(new Types(id));
      return types_p;
    }

    void setup() override {
      Actor::setup();
      TYPE_CHECKER = [](const Obj &obj, const OType otype, const ID_p &typeId) {
        singleton()->checkType(obj, otype, typeId, true);
        return typeId;
      };
      router()->route_subscription(share(
              Subscription{.source = *this->id(), .pattern = *this->pattern(), .onRecv = [](const Message_p &message) {
                if (message->retain && (message->source != *Types::singleton()->id()) &&
                    (message->source != ID("anon_src")))
                  Types::singleton()->saveType(id_p(message->target), message->payload, true);
                // else { // transient provides type checking?
                // TYPE_CHECKER(*message->payload, OTypes.toEnum(message->target.toString().c_str()),
                // id_p(message->target));
                //}
              }}));
      this->load_insts();

    }

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void saveType(const ID_p &typeId, const Obj_p &typeDef, const bool viaPub = false) {
      try {
        if (!viaPub) {
          Obj_p current = this->read(typeId, this->id());
          if (!current->isNoObj() && current != typeDef) {
            LOG_PROCESS(WARN, this, "!b%s!g[!!%s!g] !ytype!! overwritten\n", typeId->toString().c_str(),
                        current->toString().c_str());
          }
          this->write(typeId, typeDef, this->id());
        }
        if (OType::INST == OTypes.toEnum(string(typeId->path(FOS_BASE_TYPE_INDEX)))) {
          const Inst_p inst = Insts::to_inst(*typeId,
                                             typeDef->isBytecode() ? *typeDef->bcode_value() : List<Obj_p>({typeDef}));
          LOG_PROCESS(INFO, this, "!b%s!g[!!%s!g]!m:!b%s !ytype!! defined\n", typeId->toString().c_str(),
                      typeDef->isBytecode() ? typeDef->bcode_value()->front()->toString().c_str()
                                            : typeDef->toString().c_str(),
                      ITypeSignatures.toChars(inst->itype()).c_str());
        } else {
          LOG_PROCESS(INFO, this, "!b%s!g[!!%s!g] !ytype!! defined\n", typeId->toString().c_str(),
                      typeDef->toString().c_str());
        }
      } catch (const fError &e) {
        LOG_PROCESS(ERROR, this, "Unable to save type !b%s!!: %s\n", typeId->toString().c_str(), e.what());
      }
    }


    bool checkType(const Obj &obj, const OType otype, const ID_p &typeId, const bool doThrow = true) noexcept(false) {
      const OType typeOType = OTypes.toEnum(typeId->path(FOS_BASE_TYPE_INDEX));
      if (otype == OType::INST || otype == OType::BCODE || typeOType == OType::INST || typeOType == OType::BCODE)
        return true;
      if (otype != typeOType) {
        if (doThrow)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!\n", Types::singleton()->id()->toString().c_str(),
                       obj.toString().c_str(), typeId->toString().c_str());
        return false;
      }
      if (typeId->path_length() == (FOS_BASE_TYPE_INDEX + 1)) { // base type (otype)
        return true;
      }
      const Obj_p type = this->read(typeId, this->id());
      if (!type->isNoObj()) {
        if (obj.match(type, false)) {
          return true;
        }
        if (doThrow)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!g[!!%s!g]!!\n", Types::singleton()->id()->toString().c_str(),
                       obj.toString().c_str(), typeId->toString().c_str(), type->toString().c_str());
        return false;
      }
      if (doThrow)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!\n", Types::singleton()->id()->toString().c_str(),
                     typeId->toString().c_str());
      return false;
    }
  };
} // namespace fhatos
#endif
