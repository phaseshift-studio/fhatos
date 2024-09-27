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
#include <structure/router.hpp>
#include <structure/stype/key_value.hpp>
#include FOS_MQTT(mqtt.hpp)

namespace fhatos {
  template<typename STRUCTURE = KeyValue>
  class Types : public STRUCTURE {
    explicit Types(const ID &id = FOS_TYPE_PREFIX) : STRUCTURE(id.extend("#")) {
    }

    static ID_p inst_id(const string &opcode) { return id_p(INST_FURI->resolve(opcode)); }

    void load_insts() {
      const Str_p ARG_ERROR = str("wrong number of arguments");
      // this->saveType(id_p(fURI(FOS_TYPE_PREFIX).extend("uri/url")), bcode());
      this->save_type(inst_id("optional"), Insts::optional(x(0)));
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
      this->save_type(inst_id("from"), Insts::from(x(0, Insts::error(ARG_ERROR)), x(1)));
      this->save_type(inst_id("*"), Insts::from(x(0, Insts::error(ARG_ERROR)), x(1)));
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
    // template <typename STRUCTURE = KeyValue>
    static ptr<Types<STRUCTURE>> singleton(const ID &id = FOS_TYPE_PREFIX) {
      static ptr<Types<STRUCTURE>> types_p = ptr<Types<STRUCTURE>>(new Types<STRUCTURE>(id));
      return types_p;
    }

    void setup() override {
      STRUCTURE::setup();
      TYPE_CHECKER = [](const Obj &obj, const OType otype, const ID_p &type_id) -> ID_p {
        singleton()->check_type(obj, otype, type_id, true);
        return type_id;
      };
      TYPE_MAKER = [](const Obj_p &obj, const ID_p &type_id) -> Obj_p {
        const Obj_p type_def = singleton()->read(type_id);
        // TODO: require all type_defs be bytecode to avoid issue with type constant mapping
        const Obj_p proto_obj = is_base_type(type_id) || !type_def->is_bcode() ? obj : type_def->apply(obj);
        if (proto_obj->is_noobj() && !type_id->equals(*NOOBJ_FURI))
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!\n", singleton()->pattern()->toString().c_str(),
                       obj->toString().c_str(), type_id->toString().c_str());
        return share(Obj(proto_obj->_value, OTypes.to_enum(type_id->path(FOS_BASE_TYPE_INDEX)), type_id));
      };
      this->load_insts();
      router()->route_subscription(subscription_p(
        ID(*this->pattern()), *this->pattern(), QoS::_1, Insts::to_bcode([this](const Message_p &message) {
          const ID_p type_id = id_p(message->target);
          if (message->retain && !this->type_exists(type_id, message->payload))
            this->save_type(type_id, message->payload, true);
        })));
    }

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void save_type(const ID_p &type_id, const Obj_p &type_def, const bool via_pub = false) {
      try {
        if (!via_pub) {
          const Obj_p current = this->read(type_id);
          if (current != type_def) {
            if (!current->is_noobj())
              LOG_STRUCTURE(WARN, this, "!b%s!g[!!%s!g] !ytype!! overwritten\n", type_id->toString().c_str(),
                          current->toString().c_str());
            this->write(type_id, type_def->clone(), RETAIN_MESSAGE);
          }
        }
        LOG_STRUCTURE(INFO, this, "!b%s!g[!!%s!g] !ytype!! defined\n", type_id->toString().c_str(),
                      type_def->toString().c_str());
      } catch (const fError &e) {
        LOG_STRUCTURE(ERROR, this, "Unable to save type !b%s!!: %s\n", type_id->toString().c_str(), e.what());
      }
    }

    bool type_exists(const ID_p &type_id, const Obj_p &type_def) {
      const Obj_p existing_type_def = this->read(type_id);
      return !existing_type_def->is_noobj() && existing_type_def->equals(*type_def);
    }

    void save_inst_type(const ID_p &inst_id, const Inst_p &inst) {
      this->write(inst_id, inst, RETAIN_MESSAGE);
      /*this->write(id_p(inst->id()->extend("_kind")), str(ITypeDescriptions.toChars(inst->itype())), source,
                  RETAIN_MESSAGE);
      this->write(id_p(inst->id()->extend("_doc")),
                  uri(MMADT_PREFIX.extend(inst_id->toString().c_str()) //id_p(URI_FURI->extend("url")), source,
          //        RETAIN_MESSAGE);
      //this->saveType(inst->id()->extend("_seed"),inst->inst_seed_supplier())) */
    }

    static bool is_base_type(const ID_p &type_id) { return type_id->path_length() == FOS_BASE_TYPE_INDEX + 1; }

    bool check_type(const Obj &obj, const OType otype, const ID_p &type_id,
                    const bool do_throw = true) noexcept(false) {
      const OType type_otype = OTypes.to_enum(string(type_id->path(FOS_BASE_TYPE_INDEX)));
      if (otype == OType::INST || otype == OType::BCODE || type_otype == OType::INST || type_otype == OType::BCODE)
        return true;
      if (otype != type_otype) {
        if (do_throw)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!\n", this->pattern()->toString().c_str(), obj.toString().c_str(),
                       type_id->toString().c_str());
        return false;
      }
      if (type_id->path_length() == (FOS_BASE_TYPE_INDEX + 1)) {
        // base type (otype)
        return true;
      }
      const Obj_p type = this->read(type_id);
      if (!type->is_noobj()) {
        if (obj.match(type, false)) {
          return true;
        }
        if (do_throw)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!g[!!%s!g]!!\n", this->pattern()->toString().c_str(),
                       obj.toString().c_str(), type_id->toString().c_str(), type->toString().c_str());
        return false;
      }
      if (do_throw)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!\n", this->pattern()->toString().c_str(),
                     type_id->toString().c_str());
      return false;
    }
  };
} // namespace fhatos
#endif
